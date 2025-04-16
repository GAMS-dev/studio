/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "gdxviewer.h"
#include "ui_gdxviewer.h"
#include "gdxsymbol.h"
#include "gdxsymboltablemodel.h"
#include "gdxsymbolview.h"
#include "common.h"
#include "logger.h"
#include "exception.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "headerviewproxy.h"
#include "settings.h"
#include "encoding.h"

#include <QMutex>
#include <QtConcurrent>
#include <QMessageBox>
#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QApplication>
#include <QWheelEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewer::GdxViewer(const QString &gdxFile, const QString &systemDirectory, const QString &encoding, QWidget *parent)
    : AbstractView(parent),
    ui(new Ui::GdxViewer),
    mGdxFile(gdxFile),
    mSystemDirectory(systemDirectory),
    mDecoder(Encoding::createDecoder(encoding))
{
    ui->setupUi(this);
    connect(qApp, &QApplication::focusChanged, this, &GdxViewer::applySelectedSymbolOnFocus);
    headerRegister(ui->tvSymbols->horizontalHeader());
    headerRegister(ui->tvSymbols->verticalHeader());

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->tvSymbols->horizontalHeader()->setStyle(HeaderViewProxy::instance());
    setFocusProxy(ui->tvSymbols);

    mGdxMutex = new QMutex();
    gdxSetExitIndicator(0); // switch of exit() call
    gdxSetScreenIndicator(0);
    gdxSetErrorCallback(GdxViewer::errorCallback);
    char msg[GMS_SSSIZE];
    if (!gdxCreateD(&mGdx, mSystemDirectory.toLatin1(), msg, sizeof(msg)))
        EXCEPT() << "Could not load GDX library: " << msg;
    int errNr = init();
    if (errNr < 0)
        EXCEPT() << "Could not open invalid GDX file: " << gdxFile;

    QAction* cpAction = new QAction("Copy", this);
//    cpAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
//    cpAction->setShortcut(QKeySequence(tr("Ctrl+C")));
    ui->tvSymbols->addAction(cpAction);
    connect(cpAction, &QAction::triggered, this, &GdxViewer::copySelectionToClipboard);
    headerRegister(ui->tvSymbols->horizontalHeader());
    headerRegister(ui->tvSymbols->verticalHeader());
}

GdxViewer::~GdxViewer()
{
    delete mExportDialog;
    freeSymbols();
    delete mState;
    delete mGdxMutex;
    delete ui;
}

void GdxViewer::updateSelectedSymbol(const QItemSelection &selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected)
    QModelIndexList rows = ui->tvSymbols->selectionModel()->selectedRows();
    if (rows.size() > 0) {
        int selectedIdx = mSymbolTableProxyModel->mapToSource(rows.at(0)).row();
        if (deselected.indexes().size() > 0) {
            GdxSymbol* deselectedSymbol = mGdxSymbolTable->gdxSymbols().at(mSymbolTableProxyModel->mapToSource(deselected.indexes().at(0)).row());

            std::ignore = QtConcurrent::run(&GdxSymbol::stopLoadingData, deselectedSymbol);
        }

        if (reload(mDecoder.name()) != 0)
            return;

        GdxSymbol* selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selectedIdx);
        if (!selectedSymbol) return;

        if (mState) {
            mState->setSelectedSymbol(selectedSymbol->name());
            mState->setSelectedSymbolIsAlias(selectedSymbol->type() == GMS_DT_ALIAS);
        }

        //aliases are also aliases in the sense of the view
        if (selectedSymbol->type() == GMS_DT_ALIAS) {
            selectedIdx = selectedSymbol->subType();
            selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selectedIdx);
        }

        // create new GdxSymbolView if the symbol is selected for the first time
        if (!mSymbolViews.at(selectedIdx))
            createSymbolView(selectedSymbol, selectedIdx);

        if (!selectedSymbol->isLoaded())
            std::ignore = QtConcurrent::run(&GdxViewer::loadSymbol, this, selectedSymbol);

        if (ui->splitter->widget(1) != mSymbolViews.at(selectedIdx))
        ui->splitter->replaceWidget(1, mSymbolViews.at(selectedIdx));
    }
    else if (ui->splitter->widget(1) != ui->widget)
        ui->splitter->replaceWidget(1, ui->widget);
}

GdxSymbol *GdxViewer::selectedSymbol()
{
    GdxSymbol* selected = nullptr;
    if (ui->tvSymbols->selectionModel()) {
        QModelIndexList selectedIdx = ui->tvSymbols->selectionModel()->selectedRows();
        if(!selectedIdx.isEmpty()) {
            int symNr =  selectedIdx.at(0).data().toInt();
            selected = mGdxSymbolTable->gdxSymbols().at(symNr);
        }
    }
    return selected;
}

void GdxViewer::createSymbolView(GdxSymbol *sym, int symbolIndex)
{
    GdxSymbolView* symView = new GdxSymbolView(this);
    const auto headers = symView->headers();
    for (QHeaderView *header : headers) {
        headerRegister(header);
    }

    mSymbolViews.replace(symbolIndex, symView);

    if (mState && mState->symbolViewStates().contains(sym->name()))
        symView->setSym(sym, mGdxSymbolTable, mState->symbolViewState(sym->name()));
    else
        symView->setSym(sym, mGdxSymbolTable);
}

int GdxViewer::reload(const QString &encoding, bool quiet, bool triggerReload)
{
    if (mHasChanged || encoding != mDecoder.name()) {
        // in case a drag-and-drop opertion or the invalidate() function is in progress, we have to wait for it to complete
        if (dragInProgress() || mPendingInvalidate) {
            if (triggerReload) // call reload() again every 50 ms
                QTimer::singleShot(50, this, [this, encoding, quiet, triggerReload](){ reload(encoding, quiet, triggerReload); });
            return -2;
        }
        mDecoder = Encoding::createDecoder(encoding);
        releaseFile();
        int initError = init(quiet);
        if (!initError) {
            mHasChanged = false;
            setEnabled(true);
            applyState();
            //QMessageBox msgBox;
            //msgBox.setWindowTitle("GDX File Reloaded");
            //msgBox.setText("GDX file has been modified and was reloaded.");
            //msgBox.setStandardButtons(QMessageBox::Ok);
            //msgBox.setIcon(QMessageBox::Information);
            //msgBox.exec();
        }
        if (mSymbolTableProxyModel) {
            mSymbolTableProxyModel->setFilterRegularExpression(ui->lineEdit->regExp());
        }
        return initError;
    }
    return 0;
}

void GdxViewer::setHasChanged(bool value)
{
    mHasChanged = value;
}

void GdxViewer::copyAction()
{
    QWidget *source = focusWidget();

    if (source == ui->tvSymbols)
        copySelectionToClipboard();
    else if (GdxSymbolView* gdxView = qobject_cast<GdxSymbolView*>(source->parent()))
        gdxView->copySelectionToClipboard(",");
}

void GdxViewer::selectAllAction()
{
    QWidget *source = focusWidget();

    QTableView* view = qobject_cast<QTableView*>(source);
    if (!view) return;
    view->selectAll();
}

void GdxViewer::selectSearchField()
{
    QWidget *fw = focusWidget();
    while (fw && fw !=ui->splitter->widget(1))
        fw = fw->parentWidget();
    if (fw && selectedSymbol())
        symbolViewByName(selectedSymbol()->name())->setFocusSearchEdit();
    else
        ui->lineEdit->setFocus();
}

void GdxViewer::releaseFile()
{
    if (ui->splitter->widget(1) != ui->widget)
        ui->splitter->replaceWidget(1, ui->widget);
    freeSymbols();
}

void GdxViewer::invalidate()
{
    // in case a drag-and-drop opertion is in progress, we have to wait for it to complete
    if (dragInProgress()) {
        mPendingInvalidate = true;
        // call invalidate() again every 50 ms
        QTimer::singleShot(50, this, &GdxViewer::invalidate);
        return;
    }
    if (isEnabled()) {
        if (mIsInitialized) {
            saveState();
            delete mExportDialog;
            mExportDialog = nullptr;
        }
        setEnabled(false);
        releaseFile();
    }
    mPendingInvalidate = false;
}

bool GdxViewer::dragInProgress()
{
    if (!mIsInitialized) return false;
    GdxSymbol *sym = selectedSymbol();
    if (sym) {
        if (GdxSymbolView *symView=symbolViewByName(sym->name()))
            return symView->dragInProgress();
    }
    return false;
}

void GdxViewer::loadSymbol(GdxSymbol* selectedSymbol)
{
    bool ok = selectedSymbol->loadData();
    if (ok) {
        QTimer::singleShot(0,this, [this, selectedSymbol](){
            if (selectedSymbol->isDataTruncated()) {
                auto logger = SysLogLocator::systemLog();
                QString msg = "GDX symbol '" + selectedSymbol->name() + "' in file '" + mGdxFile + "' exceeds the maximum number of records (~107 million) that can be displayed and might be truncated depending on applied filters reducing the actual number of records to be displayed.";
                logger->append(msg, LogMsgType::Warning);
            }
            applySymbolState(selectedSymbol);
        });
    }
}

void GdxViewer::copySelectionToClipboard()
{
    if (!ui->tvSymbols->model())
        return;

    QModelIndexList selection = ui->tvSymbols->selectionModel()->selectedIndexes();
    if (selection.isEmpty())
        return;
    std::sort(selection.begin(), selection.end());
    QString text;
    for (QModelIndex idx : std::as_const(selection))
        text += idx.data().toString() + ",";
    text = text.chopped(1);

    QClipboard* clip = QApplication::clipboard();
    clip->setText(text);
}

int GdxViewer::init(bool quiet)
{
    int errNr = 0;
    gdxOpenRead(mGdx, mGdxFile.toLocal8Bit(), &errNr);

    if (errNr) {
        gdxClose(mGdx);
        char msg[GMS_SSSIZE];
        gdxErrorStr(mGdx,errNr, msg);

        if (!quiet) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Unable to Open GDX File");
            msgBox.setText("Unable to open GDX file: " + mGdxFile + "\nError: " + msg);
            msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            if (QMessageBox::Retry == msgBox.exec()) {
                mHasChanged = true;
                invalidate();
                errNr = reload(mDecoder.name());
            } else {
                errNr = -1;
            }
        }
        return errNr;
    }
    setEnabled(true);

    ui->splitter->widget(0)->hide();
    ui->splitter->widget(1)->hide();

    mGdxSymbolTable = new GdxSymbolTableModel(mGdx, mGdxMutex, mDecoder.name());
    mSymbolViews.resize(mGdxSymbolTable->symbolCount() + 1); // +1 because of the hidden universe symbol

    mSymbolTableProxyModel = new QSortFilterProxyModel(this);
    mSymbolTableProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mSymbolTableProxyModel->setSortRole(Qt::UserRole);
    mSymbolTableProxyModel->setSourceModel(mGdxSymbolTable);
    mSymbolTableProxyModel->setFilterKeyColumn(1);
    mSymbolTableProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tvSymbols->setModel(mSymbolTableProxyModel);
    ui->tvSymbols->resizeColumnsToContents();
    ui->tvSymbols->sortByColumn(1,Qt::AscendingOrder);
    ui->tvSymbols->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvSymbols->verticalHeader()->setMinimumSectionSize(1);
    ui->tvSymbols->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    connect(ui->tvSymbols->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GdxViewer::updateSelectedSymbol);
    connect(ui->lineEdit, &FilterLineEdit::regExpChanged, this, [this](const QRegularExpression &regExp) {
        mSymbolTableProxyModel->setFilterRegularExpression(regExp);
    });
    connect(mSymbolTableProxyModel, &QSortFilterProxyModel::rowsInserted, this, &GdxViewer::hideUniverseSymbol);
    connect(mSymbolTableProxyModel, &QSortFilterProxyModel::rowsRemoved, this, &GdxViewer::hideUniverseSymbol);
    connect(ui->lineEdit, &FilterLineEdit::columnScopeChanged, this, &GdxViewer::columnScopeChanged);
    ui->lineEdit->setKeyColumn(1);

    ui->splitter->widget(0)->show();
    ui->splitter->widget(1)->show();

    this->hideUniverseSymbol(); //first entry is the universe which we do not want to show
    ui->tvSymbols->setColumnHidden(5,true); //hide the "Loaded" column
    mIsInitialized = true;
    return errNr;
}

void GdxViewer::freeSymbols()
{
    if (!mIsInitialized)
        return;
    GdxSymbol* selected = selectedSymbol();
    if(selected)
        selected->stopLoadingData();

    disconnect(ui->tvSymbols->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GdxViewer::updateSelectedSymbol);
    ui->tvSymbols->setModel(nullptr);

    if(mGdxSymbolTable) {
        delete mGdxSymbolTable;
        mGdxSymbolTable = nullptr;
    }
    QMutexLocker locker(mGdxMutex);
    gdxClose(mGdx);
    locker.unlock();

    for (GdxSymbolView* view : std::as_const(mSymbolViews)) {
        if(view) {
            view->freeFilterMenu();
            delete view;
        }
    }
    mSymbolViews.clear();
    mIsInitialized = false;
}

void GdxViewer::hideUniverseSymbol()
{
    int row = mSymbolTableProxyModel->rowCount();
    for(int r=0; r<row; r++) {
        QVariant symName = mSymbolTableProxyModel->data(mSymbolTableProxyModel->index(r, 0), Qt::DisplayRole);
        if (symName == QVariant(0)) {
            ui->tvSymbols->hideRow(r);
            return;
        }
    }
}

void GdxViewer::columnScopeChanged()
{
    mSymbolTableProxyModel->setFilterKeyColumn(ui->lineEdit->effectiveKeyColumn());
}

void GdxViewer::applySelectedSymbolOnFocus(QWidget *old, QWidget *now)
{
    Q_UNUSED(old)
    if (isFocusedWidget(now))
        applySelectedSymbol();

}

bool GdxViewer::isFocusedWidget(QWidget *wid)
{
    while (wid) {
        wid = wid->parentWidget();
        if (wid == this)
            return true;
    }
    return false;
}

int GdxViewer::errorCallback(int count, const char *message)
{
    Q_UNUSED(count)
    auto logger = SysLogLocator::systemLog();
    logger->append(InvalidGAMS, LogMsgType::Error);
    logger->append(message, LogMsgType::Error);
    return 0;
}

void GdxViewer::saveState()
{
    if (!mState)
        mState = new GdxViewerState();

    QModelIndexList indexList = ui->tvSymbols->selectionModel()->selectedIndexes();
    if (!indexList.isEmpty()) {
        QModelIndex index = ui->tvSymbols->selectionModel()->selectedIndexes().at(1);
        QString name =ui->tvSymbols->model()->data(index).toString();
        mState->setSelectedSymbol(name);
        mState->setSelectedSymbolIsAlias(mGdxSymbolTable->getSymbolByName(name)->type() == GMS_DT_ALIAS);

    }
    mState->setSymbolTableHeaderState(ui->tvSymbols->horizontalHeader()->saveState());
    mState->setSymbolFilter(ui->lineEdit->text());
    for (GdxSymbolView* symView : std::as_const(mSymbolViews)) {
        if (symView && symView->sym()->isLoaded()) {
            GdxSymbolViewState* symViewState = mState->addSymbolViewState(symView->sym()->name());
            symView->saveState(symViewState);

            // merge pending unchecked labels into uncheck labels
            if (!symView->pendingUncheckedLabels().empty()) {
                QVector<QStringList> uncheckedLabels = symViewState->uncheckedLabels();
                for(int i=0; i<symView->sym()->dim(); i++) {
                    for (const QString &l : symView->pendingUncheckedLabels().at(i)) {
                        if (!uncheckedLabels[i].contains(l))
                            uncheckedLabels[i].append(l);
                    }
                }
                symViewState->setUncheckedLabels(uncheckedLabels);
            }
        }
    }
}

void GdxViewer::applyState()
{
    if (!mState) return;
    ui->tvSymbols->horizontalHeader()->restoreState(mState->symbolTableHeaderState());
    ui->tvSymbols->horizontalHeader()->setStretchLastSection(false);
    ui->tvSymbols->resizeColumnsToContents();
    ui->tvSymbols->horizontalHeader()->setStretchLastSection(true);
    ui->lineEdit->setText(mState->symbolFilter());
    // delete symbols that do not exist anymore or differ in dimension or type
    auto svStates = mState->symbolViewStates();
    for (auto it = svStates.constBegin() ; it != svStates.constEnd() ; ++it) {
        GdxSymbol* sym = mGdxSymbolTable->getSymbolByName(it.key());
        if (!sym || sym->dim() != it.value()->dim() || sym->type() != it.value()->type())
            mState->deleteSymbolViewState(it.key());
    }
    if (this->isVisible())
        applySelectedSymbol();
}

void GdxViewer::applySymbolState(GdxSymbol *sym)
{
    QString name = sym->name();
    GdxSymbolView* symView = symbolViewByName(name);
    if (mState && mState->symbolViewState(name)) {
        GdxSymbolViewState* symViewState = mState->symbolViewState(name);
        symView->applyState(symViewState);
        mState->deleteSymbolViewState(name);
    } else
        symView->applyDefaults();
}

void GdxViewer::applySelectedSymbol()
{
    if (!mState)
        return;
    QString name = mState->selectedSymbol();
    if (!name.isEmpty()) {
        mState->setSelectedSymbol("");
        for (int r=0; r<ui->tvSymbols->model()->rowCount(); r++) {
            QModelIndex index = ui->tvSymbols->model()->index(r, 1);
            if (index.data().toString().toLower() == name.toLower()) {
                if (mState->symbolViewState(name) || mState->selectedSymbolIsAlias())
                    ui->tvSymbols->selectRow(r);
                break;
            }
        }
    }
}

void GdxViewer::showExportDialog()
{
    if (!mExportDialog) {
        mExportDialog = new ExportDialog(this, this);
        mExportDialog->setWindowModality(Qt::ApplicationModal);
    }
    mExportDialog->show();
}

GdxSymbolTableModel *GdxViewer::gdxSymbolTable() const
{
    return mGdxSymbolTable;
}

void GdxViewer::saveDelete()
{
    GdxSymbol *sym = selectedSymbol();
    if (sym && !sym->isLoaded()) {  // we are currently loading a symbol
        connect(sym, &GdxSymbol::loadFinished, this, [this](){ deleteLater(); });
        connect(sym, &GdxSymbol::loadPaused, this, [this](){ deleteLater(); });
        sym->stopLoadingData();
        if (sym->isLoaded()) // check if loading has finished in the meantime
            deleteLater();
    } else {
        deleteLater();
    }
}

GdxViewerState *GdxViewer::state() const
{
    return mState;
}

void GdxViewer::readState(const QVariantMap& map)
{
    if (!map.isEmpty()) {
        if (!mState)
            mState = new GdxViewerState();
        mState->read(map);
        applyState();
    }
}

void GdxViewer::writeState(const QString &location)
{
    QVariantMap map;
    saveState();
    if (mState)
        mState->write(map);
    QVariantMap states = Settings::settings()->toMap(skGdxStates);
    states.insert(location, map);
    Settings::settings()->setMap(skGdxStates, states);

}

QStringList GdxViewer::getEnabledContextActions()
{
    QStringList res;
    if (focusWidget() == ui->tvSymbols) {
        if (!ui->tvSymbols->selectionModel()->selectedIndexes().isEmpty()) return {"edit-copy"};
    } else {
        QWidget *wid = focusWidget();
        for (QAction *act : wid->actions()) {
            if (act->objectName() == "edit-copy") {
                if (act->isEnabled()) res << act->objectName();
            } else if (act->objectName() == "select-all") {
                if (act->isEnabled()) res << act->objectName();
            }
        }
    }
    return res;
}



QString GdxViewer::gdxFile() const
{
    return mGdxFile;
}

GdxSymbolView *GdxViewer::symbolViewByName(QString name)
{
    GdxSymbol *sym = mGdxSymbolTable->getSymbolByName(name);
    if (sym->type() == GMS_DT_ALIAS) {
        sym = mGdxSymbolTable->gdxSymbols().at(sym->subType());
        name = sym->name();
    }
    for (GdxSymbolView* symView : std::as_const(mSymbolViews)) {
        if (symView && symView->sym()->name().toLower() == name.toLower())
            return symView;
    }
    return nullptr;
}

void GdxViewer::on_pbExport_clicked()
{
    showExportDialog();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
