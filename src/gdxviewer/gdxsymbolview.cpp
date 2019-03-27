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
#include "gdxsymbolview.h"
#include "ui_gdxsymbolview.h"
#include "gdxsymbolheaderview.h"
#include "gdxsymbol.h"
#include "columnfilter.h"
#include "nestedheaderview.h"
#include "tableviewmodel.h"

#include <QClipboard>
#include <QWidgetAction>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolView::GdxSymbolView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GdxSymbolView)
{
    ui->setupUi(this);
    ui->tvTableView->hide();

    //create context menu
    QAction* cpComma = mContextMenu.addAction("Copy (comma-separated)\tCtrl+C", [this]() { copySelectionToClipboard(","); });
    cpComma->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cpComma->setShortcutVisibleInContextMenu(true);
    cpComma->setShortcutContext(Qt::WidgetShortcut);
    ui->tvListView->addAction(cpComma);

    QAction* cpTab = mContextMenu.addAction("Copy (tab-separated)", [this]() { copySelectionToClipboard("\t"); }, QKeySequence("Ctrl+Shift+C"));
    cpTab->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cpTab->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(cpTab);

    mContextMenu.addSeparator();

    QAction* aSelectAll = mContextMenu.addAction("Select All\tCtrl+A", [this]() { selectAll(); });
    aSelectAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    aSelectAll->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(aSelectAll);

    //create header for list view
    GdxSymbolHeaderView* headerView = new GdxSymbolHeaderView(Qt::Horizontal);
    headerView->setEnabled(false);

    ui->tvListView->setHorizontalHeader(headerView);
    ui->tvListView->setSortingEnabled(true);
    ui->tvListView->horizontalHeader()->setSortIndicatorShown(true);
    ui->tvListView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    ui->tvListView->horizontalHeader()->setSectionsClickable(true);
    ui->tvListView->horizontalHeader()->setSectionsMovable(true);
    ui->tvListView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvListView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvListView->verticalHeader()->setDefaultSectionSize(int(ui->tvListView->fontMetrics().height()*1.4));

    connect(ui->tvListView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &GdxSymbolView::showColumnFilter);
    connect(ui->cbSqueezeDefaults, &QCheckBox::toggled, this, &GdxSymbolView::toggleSqueezeDefaults);
    connect(ui->pbResetSortFilter, &QPushButton::clicked, this, &GdxSymbolView::resetSortFilter);
    connect(ui->pbToggleView, &QPushButton::clicked, this, &GdxSymbolView::toggleView);

    ui->tvListView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tvTableView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);

    ui->tvTableView->setVerticalHeader(new NestedHeaderView(Qt::Vertical));
    ui->tvTableView->setHorizontalHeader(new NestedHeaderView(Qt::Horizontal));

    //mSym->setTvTableView(ui->tvTableView);
}

GdxSymbolView::~GdxSymbolView()
{
    delete ui;
}

void GdxSymbolView::showColumnFilter(QPoint p)
{
    int column = ui->tvListView->horizontalHeader()->logicalIndexAt(p);
    if(mSym->isLoaded() && column>=0 && column<mSym->dim()) {
        QMenu m(this);
        m.addAction(new ColumnFilter(mSym, column, this));
        m.exec(ui->tvListView->mapToGlobal(p));
    }
}

void GdxSymbolView::toggleSqueezeDefaults(bool checked)
{
    if (mSym->type() != GMS_DT_VAR && mSym->type() != GMS_DT_EQU)
        return;
    if (mSym) {
        if (mTableView) {
            ui->tvTableView->setUpdatesEnabled(false);
            if (checked) {
                for (int col=0; col<mTvModel->columnCount(); col++) {
                    if (mTvModel->isAllDefault(col) || !mShowValColActions[col%GMS_DT_MAX]->isChecked())
                        ui->tvTableView->setColumnHidden(col, true);
                    else
                        ui->tvTableView->setColumnHidden(col, false);
                }
            }
            else {
                for (int col=0; col<mTvModel->columnCount(); col++)
                    ui->tvTableView->setColumnHidden(col, !mShowValColActions[col%GMS_DT_MAX]->isChecked());
            }
            ui->tvTableView->setUpdatesEnabled(true);
        } else {
            ui->tvListView->setUpdatesEnabled(false);
            if (checked) {
                for (int i=0; i<GMS_VAL_MAX; i++) {
                    if (mSym->isAllDefault(i) || !mShowValColActions[i]->isChecked())
                        ui->tvListView->setColumnHidden(mSym->dim()+i, true);
                    else
                        ui->tvListView->setColumnHidden(mSym->dim()+i, false);
                }
            }
            else {
                for(int i=0; i<GMS_VAL_MAX; i++)
                    ui->tvListView->setColumnHidden(mSym->dim()+i, !mShowValColActions[i]->isChecked());
            }
            ui->tvListView->setUpdatesEnabled(true);
        }
    }
}

void GdxSymbolView::resetSortFilter()
{
    if(mSym) {
        if (mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU) {
            for (int i=0; i<GMS_VAL_MAX; i++)
                mShowValColActions[i]->setChecked(true);
        }
        mSym->resetSortFilter();
        ui->tvListView->horizontalHeader()->restoreState(mInitialHeaderState);
        static_cast<TableViewModel*>(ui->tvTableView->model())->reset();
    }
    ui->cbSqueezeDefaults->setChecked(false);
    showListView();
}

void GdxSymbolView::refreshView()
{
    if(!mSym)
        return;
    if(mSym->isLoaded())
        mSym->filterRows();
    toggleSqueezeDefaults(ui->cbSqueezeDefaults->isChecked());
}

GdxSymbol *GdxSymbolView::sym() const
{
    return mSym;
}

void GdxSymbolView::setSym(GdxSymbol *sym, GdxSymbolTable* symbolTable)
{
    mSym = sym;
    if (mSym->recordCount()>0) //enable controls only for symbols that have records, otherwise it does not make sense to filter, sort, etc
        connect(mSym, &GdxSymbol::loadFinished, this, &GdxSymbolView::enableControls);
    ui->tvListView->setModel(mSym);
    mTvModel = new TableViewModel(mSym, symbolTable);
    ui->tvTableView->setModel(mTvModel);

    if (mSym->type() == GMS_DT_EQU || mSym->type() == GMS_DT_VAR) {
        QVector<QString> valColNames;
        valColNames<< "Level" << "Marginal" << "Lower Bound" << "Upper Bound" << "Scale";
        QWidgetAction *checkableAction = new QWidgetAction(ui->tbVisibleValCols);
        QWidget *widget = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout();
        widget->setLayout(layout);
        checkableAction->setDefaultWidget(widget);
        QCheckBox *cb;
        for(int i=0; i<GMS_VAL_MAX; i++) {
            cb = new QCheckBox(valColNames[i]);
            cb->setChecked(true);
            layout->addWidget(cb);
            connect(cb, &QCheckBox::toggled, [this, i]() {toggleColumnHidden(i);});
            mShowValColActions.append(cb);
        }
        ui->tbVisibleValCols->addAction(checkableAction);
    }

    connect(ui->tvListView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);
    connect(ui->tvTableView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);

    refreshView();
}

void GdxSymbolView::copySelectionToClipboard(QString separator)
{
    if (!ui->tvListView->model())
        return;
    // row -> column -> QModelIndex
    QMap<int, QMap<int, QString>> sortedSelection;
    QTableView *tv;
    if (mTableView)
        tv = ui->tvTableView;
    else
        tv = ui->tvListView;

    QModelIndexList selection = tv->selectionModel()->selection().indexes();
    if (selection.isEmpty())
        return;

    int minRow = std::numeric_limits<int>::max();
    int maxRow = std::numeric_limits<int>::min();
    int minCol = std::numeric_limits<int>::max();
    int maxCol = std::numeric_limits<int>::min();

    for (QModelIndex idx : selection) {
        int currentRow = idx.row();
        int currentCol = idx.column();
        if (tv->isColumnHidden(currentCol))
            continue;

        currentCol = tv->horizontalHeader()->visualIndex(currentCol);
        QString currenText = idx.data().toString();
        if (currenText.contains(separator)) {
            if (currenText.contains("\'"))
                currenText = "\"" + currenText + "\"";
            else
                currenText = "\'" + currenText + "\'";
        }
        sortedSelection[currentRow][currentCol] = currenText;

        minRow = qMin(minRow, currentRow);
        maxRow = qMax(maxRow, currentRow);
        minCol = qMin(minCol, currentCol);
        maxCol = qMax(maxCol, currentCol);
    }

    QStringList sList;
    for(int r=minRow; r<maxRow+1; r++) {
        for(int c=minCol; c<maxCol+1; c++) {
            if (tv->isColumnHidden(tv->horizontalHeader()->logicalIndex(c)))
                continue;
            sList << sortedSelection[r][c] << separator;
        }
        sList.pop_back(); // remove last separator
        sList << "\n";
    }
    sList.pop_back();  // remove last newline

    QClipboard* clip = QApplication::clipboard();
    clip->setText(sList.join(""));
}

void GdxSymbolView::toggleColumnHidden(int valCol)
{
    toggleSqueezeDefaults(ui->cbSqueezeDefaults->isChecked());
}

void GdxSymbolView::showContextMenu(QPoint p)
{
    //mContextMenu.exec(ui->tvListView->mapToGlobal(p));
    if (mTableView)
        mContextMenu.exec(mapToGlobal(p)+ QPoint(ui->tvTableView->verticalHeader()->width(), ui->tvTableView->horizontalHeader()->height()));
    else
        mContextMenu.exec(mapToGlobal(p)+ QPoint(ui->tvListView->verticalHeader()->width(), ui->tvListView->horizontalHeader()->height()));
}

void GdxSymbolView::showListView()
{
    mTableView = false;
    ui->tvTableView->hide();

    // This is required in order to avoid a bug with the horizontal scrollbar (see 843)
    //ui->tvListView->setModel(nullptr);
    //ui->tvListView->setModel(mSym);

    ui->tvListView->show();
    ui->pbToggleView->setText("Table View");
}

void GdxSymbolView::showTableView()
{
    static_cast<TableViewModel*>(ui->tvTableView->model())->setTableView(true);
    ui->pbToggleView->setText("List View");

    ui->tvListView->hide();
    ui->tvTableView->show();

    QAbstractItemModel* model = ui->tvTableView->model();
    ui->tvTableView->setModel(nullptr);
    ui->tvTableView->setModel(model);


    mTableView = true;
}

void GdxSymbolView::toggleView()
{
    if (mTableView)
        showListView();
    else
        showTableView();
    refreshView();
}

void GdxSymbolView::selectAll()
{
    if (mTableView)
        ui->tvTableView->selectAll();
    else
        ui->tvListView->selectAll();

}

void GdxSymbolView::enableControls()
{
    ui->tvListView->horizontalHeader()->setEnabled(true);
    mInitialHeaderState = ui->tvListView->horizontalHeader()->saveState();
    if(mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU) {
        ui->cbSqueezeDefaults->setEnabled(true);
        ui->tbVisibleValCols->setEnabled(true);
    }
    else
        ui->cbSqueezeDefaults->setEnabled(false);
    ui->pbResetSortFilter->setEnabled(true);
    if (mSym->dim()>1)
        ui->pbToggleView->setEnabled(true);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
