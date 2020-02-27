/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "common.h"
#include "valuefilter.h"

#include <QClipboard>
#include <QWidgetAction>
#include <QLabel>
#include <QComboBox>

#include <numerics/doubleformatter.h>

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
    ui->tvTableView->addAction(cpComma);

    QAction* cpTab = mContextMenu.addAction("Copy (tab-separated)", [this]() { copySelectionToClipboard("\t"); }, QKeySequence("Ctrl+Shift+C"));
    cpTab->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cpTab->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(cpTab);
    ui->tvTableView->addAction(cpTab);

    mContextMenu.addSeparator();

    QAction* aResizeColumn = mContextMenu.addAction("Auto Resize Columns", [this]() { autoResizeColumns(); }, QKeySequence("Ctrl+R"));
    aResizeColumn->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    aResizeColumn->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(aResizeColumn);
    ui->tvTableView->addAction(aResizeColumn);

    mContextMenu.addSeparator();

    QAction* aSelectAll = mContextMenu.addAction("Select All\tCtrl+A", [this]() { selectAll(); });
    aSelectAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    aSelectAll->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(aSelectAll);
    ui->tvTableView->addAction(aSelectAll);

    // populate preferences
    QWidgetAction* preferences = new QWidgetAction(ui->tbPreferences);
    QVBoxLayout* vLayout = new QVBoxLayout();
    QWidget* widget = new QWidget();
    widget->setAutoFillBackground(true);
    mSqDefaults = new QCheckBox("Squeeze Defaults", this);
    vLayout->addWidget(mSqDefaults);
    mSqDefaults->setEnabled(false);
    mSqZeroes = new QCheckBox("Squeeze Trailing Zeroes", this);
    mSqZeroes->setChecked(true);
    vLayout->addWidget(mSqZeroes);

    QGridLayout* gridLayout = new QGridLayout(this);

    QLabel* lblValFormat = new QLabel("Format:", this);
    gridLayout->addWidget(lblValFormat,0,0);
    mValFormat = new QComboBox(this);
    mValFormat->addItem("g-format", numerics::DoubleFormatter::g);
    mValFormat->addItem("f-format", numerics::DoubleFormatter::f);
    mValFormat->addItem("e-format", numerics::DoubleFormatter::e);
    resetValFormat();
    gridLayout->addWidget(mValFormat,0,1);

    QLabel* lblPrecision = new QLabel("Precision:", this);
    gridLayout->addWidget(lblPrecision,1,0);
    mPrecision = new QSpinBox(this);
    mPrecision->setRange(1, 14);
    mPrecision->setValue(6);
    mPrecision->setWrapping(true);
    gridLayout->addWidget(mPrecision,1,1);

    vLayout->addItem(gridLayout);
    widget->setLayout(vLayout);
    preferences->setDefaultWidget(widget);
    ui->tbPreferences->addAction(preferences);

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
    ui->tvListView->verticalHeader()->setMinimumSectionSize(1);
    ui->tvListView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    connect(ui->tvListView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &GdxSymbolView::showColumnFilter);
    connect(mSqDefaults, &QCheckBox::toggled, this, &GdxSymbolView::toggleSqueezeDefaults);
    connect(ui->pbResetSortFilter, &QPushButton::clicked, this, &GdxSymbolView::resetSortFilter);
    connect(ui->pbToggleView, &QPushButton::clicked, this, &GdxSymbolView::toggleView);

    connect(mPrecision, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GdxSymbolView::updateNumericalPrecision);
    connect(mValFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GdxSymbolView::updateNumericalPrecision);

    ui->tvListView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tvTableView->setVerticalHeader(new NestedHeaderView(Qt::Vertical));
    ui->tvTableView->setHorizontalHeader(new NestedHeaderView(Qt::Horizontal));

    ui->tvTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->tvTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
}

GdxSymbolView::~GdxSymbolView()
{
    delete mSqDefaults;
    delete mSqZeroes;
    if (mTvModel)
        delete mTvModel;
    delete ui;
}

void GdxSymbolView::showColumnFilter(QPoint p)
{
    int column = ui->tvListView->horizontalHeader()->logicalIndexAt(p);
    if(mSym->isLoaded() && column>=0 && column<mSym->filterColumnCount()) {
        if (column<mSym->dim()) {
            QMenu m(this);
            ColumnFilter cf(mSym, column, this);
            m.addAction(&cf);
            m.exec(ui->tvListView->mapToGlobal(p));
        } else {
            QMenu m(this);
            ValueFilter* vf = mSym->valueFilter(column-mSym->dim());
            m.addAction(vf);
            m.exec(ui->tvListView->mapToGlobal(p));
        }
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
        mPrecision->setValue(mDefaultPrecision); // this is not to be confused with "MAX". The value will be 6
        resetValFormat();
        mSqZeroes->setChecked(true);
        if (mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU) {
            for (int i=0; i<GMS_VAL_MAX; i++)
                mShowValColActions[i]->setChecked(true);
        }
        mSym->resetSortFilter();
        ui->tvListView->horizontalHeader()->restoreState(mInitialHeaderState);
        mSqDefaults->setChecked(false);
        showListView();
        if (mTvModel) {
            ui->tvTableView->setModel(nullptr);
            delete mTvModel;
            mTvModel = nullptr;
        }
    }
}

void GdxSymbolView::refreshView()
{
    if(!mSym)
        return;
    if(mSym->isLoaded())
        mSym->filterRows();
    toggleSqueezeDefaults(mSqDefaults->isChecked());
}

GdxSymbol *GdxSymbolView::sym() const
{
    return mSym;
}

void GdxSymbolView::setSym(GdxSymbol *sym, GdxSymbolTable* symbolTable)
{
    mSym = sym;
    mGdxSymbolTable = symbolTable;
    if (mSym->recordCount()>0) { //enable controls only for symbols that have records, otherwise it does not make sense to filter, sort, etc
        connect(mSym, &GdxSymbol::loadFinished, this, &GdxSymbolView::enableControls);
        connect(mSym, &GdxSymbol::triggerListViewAutoResize, this, &GdxSymbolView::autoResizeColumns);
    }
    ui->tvListView->setModel(mSym);

    if (mSym->type() == GMS_DT_EQU || mSym->type() == GMS_DT_VAR) {
        QVector<QString> valColNames;
        valColNames<< "Level" << "Marginal" << "Lower Bound" << "Upper Bound" << "Scale";
        QWidgetAction *checkableAction = new QWidgetAction(ui->tbVisibleValCols);
        QWidget *widget = new QWidget();
        widget->setAutoFillBackground(true);
        QVBoxLayout *layout = new QVBoxLayout();
        widget->setLayout(layout);
        checkableAction->setDefaultWidget(widget);
        QCheckBox *cb;
        for(int i=0; i<GMS_VAL_MAX; i++) {
            cb = new QCheckBox(valColNames[i]);
            cb->setChecked(true);
            layout->addWidget(cb);
            connect(cb, &QCheckBox::toggled, [this]() {toggleColumnHidden();});
            mShowValColActions.append(cb);
        }
        ui->tbVisibleValCols->addAction(checkableAction);
    }

    connect(ui->tvListView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);
    connect(ui->tvTableView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);
    connect(mSqZeroes, &QCheckBox::stateChanged, this, &GdxSymbolView::updateNumericalPrecision);

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

void GdxSymbolView::toggleColumnHidden()
{
    toggleSqueezeDefaults(mSqDefaults->isChecked());
}

void GdxSymbolView::updateNumericalPrecision()
{
    if (!mSym)
        return;
    this->mSym->setNumericalPrecision(mPrecision->value(), mSqZeroes->isChecked());
    numerics::DoubleFormatter::Format format = static_cast<numerics::DoubleFormatter::Format>(mValFormat->currentData().toInt());
    this->mSym->setNumericalFormat(format);
    if (format == numerics::DoubleFormatter::g) {
        mPrecision->setRange(numerics::DoubleFormatter::gFormatFull, 17);
        mPrecision->setSpecialValueText("Full");
    }
    else if (format == numerics::DoubleFormatter::f) {
        mPrecision->setRange(0, 14);
        mPrecision->setSpecialValueText("");
    }
    else if (format == numerics::DoubleFormatter::e) {
        mPrecision->setRange(1, 17);
        mPrecision->setSpecialValueText("");
    }


    if (mTvModel)
        ui->tvTableView->reset();
}

void GdxSymbolView::showContextMenu(QPoint p)
{
    //mContextMenu.exec(ui->tvListView->mapToGlobal(p));
    if (mTableView)
        mContextMenu.exec(mapToGlobal(p)+ QPoint(ui->tvTableView->verticalHeader()->width(), ui->tvTableView->horizontalHeader()->height()));
    else
        mContextMenu.exec(mapToGlobal(p)+ QPoint(ui->tvListView->verticalHeader()->width(), ui->tvListView->horizontalHeader()->height()));
}

void GdxSymbolView::autoResizeColumns()
{
    if (mTableView) {
        ui->tvTableView->horizontalHeader()->setResizeContentsPrecision(mTVResizePrecision);
        for (int i=0; i<mTVResizeColNr; i++)
            ui->tvTableView->resizeColumnToContents(ui->tvTableView->columnAt(0)+i);
    }
    else
        ui->tvListView->resizeColumnsToContents();
}

void GdxSymbolView::showListView()
{
    mTableView = false;
    ui->tvTableView->hide();
    ui->tvListView->show();
    ui->pbToggleView->setText("Table View");
}

void GdxSymbolView::showTableView()
{
    if (!mTvModel) {
        mTvModel = new TableViewModel(mSym, mGdxSymbolTable);
        mTvModel->setTableView();
        ui->tvTableView->setModel(mTvModel);
    } else if (mSym->filterHasChanged())
        mTvModel->setTableView();
    mSym->setFilterHasChanged(false);

    ui->pbToggleView->setText("List View");

    ui->tvListView->hide();
    ui->tvTableView->show();
    mTableView = true;
    autoResizeColumns();
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

void GdxSymbolView::resetValFormat()
{
    int index = mValFormat->findData(mDefaultValFormat);
    if (index != -1)
       mValFormat->setCurrentIndex(index);
}

void GdxSymbolView::enableControls()
{
    ui->tvListView->horizontalHeader()->setEnabled(true);
    mInitialHeaderState = ui->tvListView->horizontalHeader()->saveState();
    if(mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU) {
        mSqDefaults->setEnabled(true);
        ui->tbVisibleValCols->setEnabled(true);
    }
    else
        mSqDefaults->setEnabled(false);
    ui->pbResetSortFilter->setEnabled(true);
    ui->tbPreferences->setEnabled(true);
    if (mSym->dim()>1)
        ui->pbToggleView->setEnabled(true);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
