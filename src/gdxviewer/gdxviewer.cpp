/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include "gdxsymboltable.h"
#include "gdxsymbolview.h"
#include "common.h"
#include "logger.h"
#include "exception.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "headerviewproxy.h"

#include <QMutex>
#include <QtConcurrent>
#include <QMessageBox>
#include <QClipboard>
#include <QSortFilterProxyModel>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewer::GdxViewer(QString gdxFile, QString systemDirectory, QTextCodec* codec, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::GdxViewer),
      mGdxFile(gdxFile),
      mSystemDirectory(systemDirectory),
      mCodec(codec)
{
    ui->setupUi(this);

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->tvSymbols->horizontalHeader()->setStyle(HeaderViewProxy::instance());
    QPalette palette;
    palette.setColor(QPalette::Highlight, ui->tvSymbols->palette().highlight().color());
    palette.setColor(QPalette::HighlightedText, ui->tvSymbols->palette().highlightedText().color());
    ui->tvSymbols->setPalette(palette);
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

    QAction* cpAction = new QAction("Copy");
//    cpAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
//    cpAction->setShortcut(QKeySequence(tr("Ctrl+C")));
    ui->tvSymbols->addAction(cpAction);
    connect(cpAction, &QAction::triggered, this, &GdxViewer::copySelectionToClipboard);
}

GdxViewer::~GdxViewer()
{
    freeSymbols();
    delete mState;
    delete mGdxMutex;
    delete ui;
}

void GdxViewer::updateSelectedSymbol(QItemSelection selected, QItemSelection deselected)
{
    if (selected.indexes().size() > 0) {
        int selectedIdx = mSymbolTableProxyModel->mapToSource(selected.indexes().at(0)).row();
        if (deselected.indexes().size()>0) {
            GdxSymbol* deselectedSymbol = mGdxSymbolTable->gdxSymbols().at(mSymbolTableProxyModel->mapToSource(deselected.indexes().at(0)).row());
            QtConcurrent::run(deselectedSymbol, &GdxSymbol::stopLoadingData);
        }

        if (reload(mCodec) != 0)
            return;

        GdxSymbol* selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selectedIdx);
        if (!selectedSymbol) return;

        //aliases are also aliases in the sense of the view
        if (selectedSymbol->type() == GMS_DT_ALIAS) {
            selectedIdx = selectedSymbol->subType();
            selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selectedIdx);
        }

        // create new GdxSymbolView if the symbol is selected for the first time
        if (!mSymbolViews.at(selectedIdx)) {
            GdxSymbolView* symView = new GdxSymbolView();
            mSymbolViews.replace(selectedIdx, symView);
            symView->setSym(selectedSymbol, mGdxSymbolTable);
        }

        if (!selectedSymbol->isLoaded())
            QtConcurrent::run(this, &GdxViewer::loadSymbol, selectedSymbol);

        ui->splitter->replaceWidget(1, mSymbolViews.at(selectedIdx));
    }
    else
        ui->splitter->replaceWidget(1, ui->widget);
}

GdxSymbol *GdxViewer::selectedSymbol()
{
    GdxSymbol* selected = nullptr;
    if (ui->tvSymbols->selectionModel()) {
        QModelIndexList selectedIdx = ui->tvSymbols->selectionModel()->selectedRows();
        if(!selectedIdx.isEmpty())
            selected = mGdxSymbolTable->gdxSymbols().at(selectedIdx.at(0).row());
    }
    return selected;
}

int GdxViewer::reload(QTextCodec* codec, bool quiet)
{
    if (mHasChanged || codec != mCodec) {
        mCodec = codec;
        releaseFile();
        int initError = init(quiet);
        if (!initError) {
            mHasChanged = false;
            setEnabled(true);
            //QMessageBox msgBox;
            //msgBox.setWindowTitle("GDX File Reloaded");
            //msgBox.setText("GDX file has been modified and was reloaded.");
            //msgBox.setStandardButtons(QMessageBox::Ok);
            //msgBox.setIcon(QMessageBox::Information);
            //msgBox.exec();
        }
        if (mSymbolTableProxyModel) {
            mSymbolTableProxyModel->setFilterWildcard(ui->lineEdit->text());
            mSymbolTableProxyModel->setFilterKeyColumn(ui->cbToggleSearch->isChecked() ? -1 : 1);
        }
        return initError;
        applyState();
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

    if (static_cast<QTableView*>(source) == ui->tvSymbols)
        copySelectionToClipboard();
    else if (static_cast<GdxSymbolView*>(source->parent())) {
        GdxSymbolView* gdxView = static_cast<GdxSymbolView*>(source->parent());
        gdxView->copySelectionToClipboard(",");
    }
}

void GdxViewer::selectAllAction()
{
    QWidget *source = focusWidget();

    QTableView* view = dynamic_cast<QTableView*>(source);
    if (!view) return;
    view->selectAll();
}

void GdxViewer::selectSearchField()
{
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
    if (isEnabled()) {
        saveState();
        setEnabled(false);
        releaseFile();
    }
}

void GdxViewer::loadSymbol(GdxSymbol* selectedSymbol)
{
    selectedSymbol->loadData();
    QTimer::singleShot(0,this, [this, selectedSymbol](){applySymbolState(selectedSymbol);});
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
    for (QModelIndex idx : selection)
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
                errNr = reload(mCodec);
            } else {
                errNr = -1;
            }
        }
        return errNr;
    }
    setEnabled(true);

    ui->splitter->widget(0)->hide();
    ui->splitter->widget(1)->hide();

    mGdxSymbolTable = new GdxSymbolTable(mGdx, mGdxMutex, mCodec);
    mSymbolViews.resize(mGdxSymbolTable->symbolCount() + 1); // +1 because of the hidden universe symbol

    mSymbolTableProxyModel = new QSortFilterProxyModel(this);
    mSymbolTableProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
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
    connect(ui->lineEdit, &QLineEdit::textChanged, mSymbolTableProxyModel, &QSortFilterProxyModel::setFilterWildcard);
    connect(mSymbolTableProxyModel, &QSortFilterProxyModel::rowsInserted, this, &GdxViewer::hideUniverseSymbol);
    connect(mSymbolTableProxyModel, &QSortFilterProxyModel::rowsRemoved, this, &GdxViewer::hideUniverseSymbol);
    connect(ui->cbToggleSearch, &QCheckBox::toggled, this, &GdxViewer::toggleSearchColumns);

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

    for (GdxSymbolView* view : mSymbolViews) {
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

void GdxViewer::toggleSearchColumns(bool checked)
{
    if (checked)
        mSymbolTableProxyModel->setFilterKeyColumn(-1);
    else
        mSymbolTableProxyModel->setFilterKeyColumn(1);
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
    if (mState != NULL)
        delete mState;
    mState = new GdxViewerState();
    mState->setSymbolTableFilter(ui->lineEdit->text());
    mState->setAllColumnsChecked(ui->cbToggleSearch->isChecked());
    for (GdxSymbolView* symView : mSymbolViews) {
        if (symView != NULL && symView->sym()->isLoaded()) {
            GdxSymbolViewState* symViewState = mState->addSymbolViewState(symView->sym()->name());
            symView->saveState(symViewState);
        }
    }
}

void GdxViewer::applyState()
{
    ui->lineEdit->setText(mState->symbolTableFilter());
    ui->cbToggleSearch->setChecked(mState->allColumnsChecked());
}

void GdxViewer::applySymbolState(GdxSymbol *sym)
{
    QString name = sym->name();
    if (mState) {
        GdxSymbolViewState* symViewState = mState->symbolViewState(name);
        if (symViewState) {
            GdxSymbolView* symView = symbolViewByName(name);
            symView->applyState(symViewState);
            mState->deleteSymbolViewState(name);
        }
    }
}

GdxSymbolView *GdxViewer::symbolViewByName(QString name)
{
    for (GdxSymbolView* symView : mSymbolViews) {
        if (symView != NULL) {
            if (symView->sym()->name() == name)
                return symView;
        }
    }
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
