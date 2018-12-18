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

#include <QClipboard>

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

    //ui->tvTableView->setVerticalHeader(new NestedHeaderView(Qt::Vertical));
    //ui->tvTableView->setHorizontalHeader(new NestedHeaderView(Qt::Horizontal));

    connect(ui->tvListView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &GdxSymbolView::showColumnFilter);
    connect(ui->cbSqueezeDefaults, &QCheckBox::toggled, this, &GdxSymbolView::toggleSqueezeDefaults);
    connect(ui->pbResetSortFilter, &QPushButton::clicked, this, &GdxSymbolView::resetSortFilter);
    connect(ui->pbToggleView, &QPushButton::clicked, this, &GdxSymbolView::toggleView);

    ui->tvListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tvListView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);
    ui->tvTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tvTableView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);
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
        if (mSym->tableView()) {
            ui->tvTableView->setUpdatesEnabled(false);
            if (checked) {
                for (int col=0; col<mSym->columnCount(); col++) {
                    if (mSym->isAllDefault(col) || !mShowValColActions[col%GMS_DT_MAX]->isChecked())
                        ui->tvTableView->setColumnHidden(col, true);
                    else
                        ui->tvTableView->setColumnHidden(col, false);
                }
            }
            else {
                for (int col=0; col<mSym->columnCount(); col++)
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
    }
    ui->cbSqueezeDefaults->setChecked(false);


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

void GdxSymbolView::setSym(GdxSymbol *sym)
{
    mSym = sym;
    if (mSym->recordCount()>0) //enable controls only for symbols that have records, otherwise it does not make sense to filter, sort, etc
        connect(mSym, &GdxSymbol::loadFinished, this, &GdxSymbolView::enableControls);
    ui->tvListView->setModel(mSym);
    ui->tvTableView->setModel(mSym);

    if (mSym->type() == GMS_DT_EQU || mSym->type() == GMS_DT_VAR) {
        QVector<QString> valColNames;
        valColNames<< "Level" << "Marginal" << "Lower Bound" << "Upper Bound" << "Scale";
        QMenu* valColMenu = mContextMenu.addMenu("Visible Value Columns");
        QAction *action;
        for(int i=0; i<GMS_VAL_MAX; i++) {
            action = valColMenu->addAction(valColNames[i], [this, i]() {toggleColumnHidden(i);});
            action->setCheckable(true);
            action->setChecked(true);
            mShowValColActions.append(action);
        }
    }
    refreshView();
}

void GdxSymbolView::copySelectionToClipboard(QString separator)
{
    if (!ui->tvListView->model())
        return;
    // row -> column -> QModelIndex
    QMap<int, QMap<int, QString>> sortedSelection;
    QTableView *tv;
    if (mSym->tableView())
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
    if (mSym->tableView()) {
        for (int i=0; i<mSym->rowCount(); i++) {
            ui->tvTableView->setColumnHidden(i, !mShowValColActions[i%GMS_DT_MAX]);
        }
    } else
        ui->tvListView->setColumnHidden(valCol+mSym->dim(), !mShowValColActions[valCol]);
    toggleSqueezeDefaults(ui->cbSqueezeDefaults->isChecked());
}

void GdxSymbolView::showContextMenu(QPoint p)
{
    //mContextMenu.exec(ui->tvListView->mapToGlobal(p));
    if (mSym->tableView())
        mContextMenu.exec(mapToGlobal(p)+ QPoint(ui->tvTableView->verticalHeader()->width(), ui->tvTableView->horizontalHeader()->height()));
    else
        mContextMenu.exec(mapToGlobal(p)+ QPoint(ui->tvListView->verticalHeader()->width(), ui->tvListView->horizontalHeader()->height()));
}

void GdxSymbolView::showListView()
{
    ui->pbResetSortFilter->setEnabled(true);
    mSym->setTableView(false);
    ui->tvTableView->hide();
    ui->tvListView->show();
    ui->pbToggleView->setText("Table View");
}

void GdxSymbolView::showTableView()
{
    ui->pbResetSortFilter->setEnabled(false);
    mSym->setTableView(true);

    ui->pbToggleView->setText("List View");

    NestedHeaderView *hvV = new NestedHeaderView(Qt::Vertical);
    NestedHeaderView *hvH = new NestedHeaderView(Qt::Horizontal);
    ui->tvTableView->setVerticalHeader(hvV);
    ui->tvTableView->setHorizontalHeader(hvH);
    hvV->init();
    hvH->init();
    ui->tvListView->hide();
    ui->tvTableView->show();
}

void GdxSymbolView::toggleView()
{
    if (mSym->tableView())
        showListView();
    else
        showTableView();
    refreshView();
}

void GdxSymbolView::selectAll()
{
    if (mSym->tableView())
        ui->tvTableView->selectAll();
    else
        ui->tvListView->selectAll();

}

void GdxSymbolView::enableControls()
{
    ui->tvListView->horizontalHeader()->setEnabled(true);
    mInitialHeaderState = ui->tvListView->horizontalHeader()->saveState();
    if(mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU)
        ui->cbSqueezeDefaults->setEnabled(true);
    else
        ui->cbSqueezeDefaults->setEnabled(false);
    ui->pbResetSortFilter->setEnabled(true);
    if (mSym->dim()>1)
        ui->pbToggleView->setEnabled(true);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
