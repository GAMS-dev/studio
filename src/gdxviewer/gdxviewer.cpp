/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "exception.h"

#include <QMutex>
#include <QtConcurrent>
#include <QMessageBox>
#include <QClipboard>
#include <QSortFilterProxyModel>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewer::GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::GdxViewer),
      mGdxFile(gdxFile),
      mSystemDirectory(systemDirectory)
{
    ui->setupUi(this);

    QPalette palette;
    palette.setColor(QPalette::Highlight, ui->tvSymbols->palette().highlight().color());
    palette.setColor(QPalette::HighlightedText, ui->tvSymbols->palette().highlightedText().color());
    ui->tvSymbols->setPalette(palette);

    mGdxMutex = new QMutex();
    char msg[GMS_SSSIZE];
    if (!gdxCreateD(&mGdx, mSystemDirectory.toLatin1(), msg, sizeof(msg)))
        EXCEPT() << "Could not load GDX library: " << msg;
    init();

    QAction* cpAction = new QAction("Copy");
//    cpAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
//    cpAction->setShortcut(QKeySequence(tr("Ctrl+C")));
    ui->tvSymbols->addAction(cpAction);
    connect(cpAction, &QAction::triggered, this, &GdxViewer::copySelectionToClipboard);
}

GdxViewer::~GdxViewer()
{
    free();
    delete mGdxMutex;
    delete ui;
}

void GdxViewer::updateSelectedSymbol(QItemSelection selected, QItemSelection deselected)
{
    if (selected.indexes().size()>0) {
        int selectedIdx = mSymbolTableProxyModel->mapToSource(selected.indexes().at(0)).row();
        if (deselected.indexes().size()>0) {
            GdxSymbol* deselectedSymbol = mGdxSymbolTable->gdxSymbols().at(mSymbolTableProxyModel->mapToSource(deselected.indexes().at(0)).row());
            QtConcurrent::run(deselectedSymbol, &GdxSymbol::stopLoadingData);
        }

        if (!reload())
            return;

        GdxSymbol* selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selectedIdx);

        //aliases are also aliases in the sense of the view
        if (selectedSymbol->type() == GMS_DT_ALIAS) {
            selectedIdx = selectedSymbol->subType();
            selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selectedIdx);
        }

        // create new GdxSymbolView if the symbol is selected for the first time
        if (!mSymbolViews.at(selectedIdx)) {
            GdxSymbolView* symView = new GdxSymbolView();
            mSymbolViews.replace(selectedIdx, symView);
            symView->setSym(selectedSymbol);
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

bool GdxViewer::reload()
{
    if (mHasChanged) {
        if (ui->splitter->widget(1) != ui->widget)
            ui->splitter->replaceWidget(1, ui->widget);
        free();
        bool initSuccess = init();
        if (initSuccess) {
            mHasChanged = false;
            //QMessageBox msgBox;
            //msgBox.setWindowTitle("GDX File Reloaded");
            //msgBox.setText("GDX file has been modified and was reloaded.");
            //msgBox.setStandardButtons(QMessageBox::Ok);
            //msgBox.setIcon(QMessageBox::Information);
            //msgBox.exec();
        }
        return initSuccess;
    }
    return true;
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

void GdxViewer::loadSymbol(GdxSymbol* selectedSymbol)
{
    selectedSymbol->loadData();
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

bool GdxViewer::init()
{
    int errNr = 0;

    gdxOpenRead(mGdx, mGdxFile.toLocal8Bit(), &errNr);
    if (errNr) {
        gdxClose(mGdx);
        char msg[GMS_SSSIZE];
        gdxErrorStr(mGdx,errNr, msg);

        QMessageBox msgBox;
        msgBox.setWindowTitle("Unable to Open GDX File");
        msgBox.setText("Unable to open GDX file: " + mGdxFile + "\nError: " + msg);
        msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        if (QMessageBox::Retry == msgBox.exec()) {
            mHasChanged = true;
            reload();
        }
        return false;
    }

    ui->splitter->widget(0)->hide();
    ui->splitter->widget(1)->hide();

    mGdxSymbolTable = new GdxSymbolTable(mGdx, mGdxMutex);
    mSymbolViews.resize(mGdxSymbolTable->symbolCount() + 1); // +1 because of the hidden universe symbol

    mSymbolTableProxyModel = new QSortFilterProxyModel(this);
    mSymbolTableProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mSymbolTableProxyModel->setSourceModel(mGdxSymbolTable);
    mSymbolTableProxyModel->setFilterKeyColumn(1);
    mSymbolTableProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tvSymbols->setModel(mSymbolTableProxyModel);
    ui->tvSymbols->resizeColumnsToContents();
    ui->tvSymbols->sortByColumn(1,Qt::AscendingOrder);

    connect(ui->tvSymbols->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GdxViewer::updateSelectedSymbol);
    connect(ui->lineEdit, &QLineEdit::textChanged, mSymbolTableProxyModel, &QSortFilterProxyModel::setFilterWildcard);
    connect(mSymbolTableProxyModel, &QSortFilterProxyModel::rowsInserted, this, &GdxViewer::hideUniverseSymbol);
    connect(mSymbolTableProxyModel, &QSortFilterProxyModel::rowsRemoved, this, &GdxViewer::hideUniverseSymbol);
    connect(ui->cbToggleSearch, &QCheckBox::toggled, this, &GdxViewer::toggleSearchColumns);

    ui->splitter->widget(0)->show();
    ui->splitter->widget(1)->show();

    this->hideUniverseSymbol(); //first entry is the universe which we do not want to show
    ui->tvSymbols->setColumnHidden(5,true); //hide the "Loaded" column
    return true;
}

void GdxViewer::free()
{
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
        if(view)
            delete view;
    }
    mSymbolViews.clear();
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

void GdxViewer::reportIoError(int errNr, QString message)
{
    // TODO(JM) An exception contains information about it's source line -> it should be thrown where it occurs
    EXCEPT() << "Fatal I/O Error = " << errNr << " when calling " << message;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
