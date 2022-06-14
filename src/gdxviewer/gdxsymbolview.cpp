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
#include "gdxsymbolview.h"
#include "ui_gdxsymbolview.h"
#include "gdxsymbolheaderview.h"
#include "gdxsymbol.h"
#include "columnfilter.h"
#include "headerviewproxy.h"
#include "nestedheaderview.h"
#include "tableviewmodel.h"
#include "theme.h"
#include "common.h"
#include "valuefilter.h"
#include "tableviewdomainmodel.h"
#include "settings.h"

#include <QClipboard>
#include <QWidgetAction>
#include <QLabel>
#include <QTimer>


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
    ui->tvTableViewFilter->hide();
    ui->tbDomLeft->hide();
    ui->tbDomRight->hide();
    ui->laError->hide();
    ui->laError->setStyleSheet("color:"+toColor(Theme::Normal_Red).name()+";");

    mDefaultSymbolView = DefaultSymbolView(Settings::settings()->toInt(SettingsKey::skGdxDefaultSymbolView));

    //create context menu
    QAction* cpComma = mContextMenuLV.addAction("Copy (comma-separated)\tCtrl+C", [this]() { copySelectionToClipboard(","); });
    mContextMenuTV.addAction(cpComma);
    cpComma->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cpComma->setShortcutVisibleInContextMenu(true);
    cpComma->setShortcutContext(Qt::WidgetShortcut);
    ui->tvListView->addAction(cpComma);
    ui->tvTableView->addAction(cpComma);

    QAction* cpTab = mContextMenuLV.addAction("Copy (tab-separated)", [this]() { copySelectionToClipboard("\t"); }, QKeySequence("Ctrl+Shift+C"));
    mContextMenuTV.addAction(cpTab);
    cpTab->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cpTab->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(cpTab);
    ui->tvTableView->addAction(cpTab);

    mContextMenuTV.addAction("Copy Without Labels (comma-separated)", [this]() { copySelectionToClipboard(",", false); });
    mContextMenuTV.addAction("Copy Without Labels (tab-separated)", [this]() { copySelectionToClipboard("\t", false); });

    mContextMenuLV.addSeparator();
    mContextMenuTV.addSeparator();

    QAction* aResizeColumn = mContextMenuLV.addAction("Auto Resize Columns", [this]() { autoResizeColumns(); }, QKeySequence("Ctrl+R"));
    mContextMenuTV.addAction(aResizeColumn);
    aResizeColumn->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    aResizeColumn->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(aResizeColumn);
    ui->tvTableView->addAction(aResizeColumn);

    mContextMenuLV.addSeparator();
    mContextMenuTV.addSeparator();

    QAction* aSelectAll = mContextMenuLV.addAction("Select All\tCtrl+A", [this]() { selectAll(); });
    mContextMenuTV.addAction(aSelectAll);
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

    QGridLayout* gridLayout = new QGridLayout();

    QLabel* lblValFormat = new QLabel("Format:", this);
    gridLayout->addWidget(lblValFormat,0,0);
    mValFormat = new QComboBox(this);
    mValFormat->addItem("g-format", numerics::DoubleFormatter::g);
    mValFormat->addItem("f-format", numerics::DoubleFormatter::f);
    mValFormat->addItem("e-format", numerics::DoubleFormatter::e);
    mValFormat->setToolTip("<html><head/><body><p>Display format for numerical values:</p>"
                          "<p><span style=' font-weight:600;'>g-format:</span> The display format is chosen automatically:  <span style=' font-style:italic;'>f-format</span> for numbers closer to one and  <span style=' font-style:italic;'>e-format</span> otherwise. The value in the <span style=' font-style:italic;'>Precision</span> spin box specifies the number of significant digits. When precision is set to  <span style=' font-style:italic;'>Full</span>, the number of digits used is the least possible such that the displayed value would convert back to the value stored in GDX. Trailing zeros do not exist when <span style=' font-style:italic;'>precision=Full</span>.</p>"
                          "<p><span style=' font-weight:600;'>f-format:</span> Values are displayed in fixed format as long as appropriate. Large numbers are still displayed in scientific format. The value in the <span style=' font-style:italic;'>Precision</span> spin box specifies the number of decimals.</p>"
                          "<p><span style=' font-weight:600;'>e-format:</span> Values are displayed in scientific format. The value in the <span style=' font-style:italic;'>Precision</span> spin box specifies the number of significant digits. When precision is set to  <span style=' font-style:italic;'>Full</span>, the number of digits used is the least possible such that the displayed value would convert back to the value stored in GDX. Trailing zeros do not exist when <span style=' font-style:italic;'>precision=Full</span>.</p></body></html>");
    resetValFormat();
    gridLayout->addWidget(mValFormat,0,1);

    QLabel* lblPrecision = new QLabel("Precision:", this);
    gridLayout->addWidget(lblPrecision,1,0);
    mPrecision = new QSpinBox(this);
    mPrecision->setRange(1, 14);
    mPrecision->setValue(6);
    mPrecision->setWrapping(true);
    mPrecision->setToolTip("<html><head/><body><p>Specifies the number of decimals or the number of significant digits depending on the chosen format:</p><p><span style=' font-weight:600;'>"
                           "g-format:</span> Significant digits [1..17, Full]</p><p><span style=' font-weight:600;'>"
                           "f-format:</span> Decimals [0..14]</p><p><span style=' font-weight:600;'>"
                           "e-format:</span> Significat digits [1..17, Full]</p></body></html>");
    gridLayout->addWidget(mPrecision,1,1);

    vLayout->addItem(gridLayout);
    widget->setLayout(vLayout);
    preferences->setDefaultWidget(widget);
    ui->tbPreferences->addAction(preferences);

    //create header for list view
    GdxSymbolHeaderView* headerView = new GdxSymbolHeaderView(Qt::Horizontal, GdxSymbolHeaderView::ListView);
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

    connect(ui->tvListView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &GdxSymbolView::showFilter);
    connect(mSqDefaults, &QCheckBox::toggled, this, &GdxSymbolView::toggleSqueezeDefaults);
    connect(ui->pbResetSortFilter, &QPushButton::clicked, this, &GdxSymbolView::resetSortFilter);
    connect(ui->pbToggleView, &QPushButton::clicked, this, &GdxSymbolView::toggleView);

    connect(mPrecision, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GdxSymbolView::updateNumericalPrecision);
    connect(mValFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GdxSymbolView::updateNumericalPrecision);

    ui->tvListView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tvTableView->setVerticalHeader(new NestedHeaderView(Qt::Vertical));
    ui->tvTableView->setHorizontalHeader(new NestedHeaderView(Qt::Horizontal));
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->tvTableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());

    ui->tvTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->tvTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    ui->tvTableViewFilter->setHorizontalHeader(new GdxSymbolHeaderView(Qt::Horizontal, GdxSymbolHeaderView::TableViewFilter));
    ui->tvTableViewFilter->horizontalHeader()->setVisible(true);
    ui->tvTableViewFilter->horizontalHeader()->setSectionsClickable(true);
    ui->tvTableViewFilter->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tvTableViewFilter->horizontalHeader()->installEventFilter(this);
    connect(ui->tvTableViewFilter->horizontalHeader(), &QHeaderView::sectionResized, this, &GdxSymbolView::adjustDomainScrollbar);

    connect(ui->tvTableViewFilter->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &GdxSymbolView::showFilter);

    connect(ui->tbDomLeft, &QToolButton::clicked, this, &GdxSymbolView::tvFilterScrollLeft);
    connect(ui->tbDomRight, &QToolButton::clicked, this, &GdxSymbolView::tvFilterScrollRight);
}

GdxSymbolView::~GdxSymbolView()
{
    delete mSqDefaults;
    delete mSqZeroes;
    if (mTvModel)
        delete mTvModel;
    delete ui;
}

void GdxSymbolView::showFilter(QPoint p)
{
    if (mSym->hasInvalidUel()) return;
    QTableView* tableView = mTableView ? ui->tvTableViewFilter : ui->tvListView;
    int column = tableView->horizontalHeader()->logicalIndexAt(p);

    if(mSym->isLoaded() && column>=0 && column<mSym->filterColumnCount()) {
        mColumnFilterMenu = new QMenu(this);
        connect(mColumnFilterMenu, &QMenu::aboutToHide, this, &GdxSymbolView::freeFilterMenu);
        QWidgetAction *filter = nullptr;
        if (column<mSym->dim())
            filter = mSym->columnFilter(column);
        else
            filter = mSym->valueFilter(column-mSym->dim());
        mColumnFilterMenu->addAction(filter);
        mColumnFilterMenu->popup(tableView->mapToGlobal(p));
    }
}

void GdxSymbolView::freeFilterMenu()
{
    if (mColumnFilterMenu) {
        mColumnFilterMenu->deleteLater();
        mColumnFilterMenu = nullptr;
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
            for (int col=0; col<GMS_VAL_MAX; col++)
                ui->tvTableViewFilter->setColumnHidden(mSym->dim()+col, !mShowValColActions[col]->isChecked());
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
        showListView();
        if (mTvModel) {
            ui->tvTableViewFilter->setModel(nullptr);
            delete mTvDomainModel;
            mTvDomainModel = nullptr;
            delete mTvModel;
            mTvModel = nullptr;
            ui->tvTableView->setModel(nullptr);
        }
        ui->tvListView->horizontalHeader()->restoreState(mInitialHeaderState);
        mSym->resetSortFilter();
        mSqDefaults->setChecked(false);
        mLVFirstInit = true;
        mTVFirstInit = true;
        mTVResizeOnInit = true;
        mPendingUncheckedLabels.clear();
        showDefaultView();
    }
}

GdxSymbol *GdxSymbolView::sym() const
{
    return mSym;
}

void GdxSymbolView::setSym(GdxSymbol *sym, GdxSymbolTableModel* symbolTable, GdxSymbolViewState* symViewState)
{
    mSym = sym;
    mGdxSymbolTable = symbolTable;
    if (mSym->recordCount()>0) { //enable controls only for symbols that have records, otherwise it does not make sense to filter, sort, etc
        connect(mSym, &GdxSymbol::loadFinished, this, &GdxSymbolView::enableControls);
        connect(mSym, &GdxSymbol::triggerListViewAutoResize, this, &GdxSymbolView::autoResizeColumns);
        showDefaultView(symViewState);
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
}

void GdxSymbolView::copySelectionToClipboard(QString separator, bool copyLabels)
{
    if (!ui->tvListView->model())
        return;
    // row -> column -> QModelIndex
    QMap<int, QMap<int, QString>> sortedSelection;
    QTableView *tv;
    if (mTableView)
        tv = ui->tvTableView;
    else {
        tv = ui->tvListView;
        copyLabels = false; // copy labels only available in table view mode
    }

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
    if (copyLabels) { // copy labels as well in table view mode
        int colHeaderDim = (static_cast<NestedHeaderView*>(tv->horizontalHeader()))->dim();
        int rowHeaderDim = (static_cast<NestedHeaderView*>(tv->verticalHeader()))->dim();
        for (int i=0; i<colHeaderDim; i++) {
            for (int j=0; j<rowHeaderDim; j++)
                sList << separator;
            for(int c=minCol; c<maxCol+1; c++) {
                if (tv->isColumnHidden(tv->horizontalHeader()->logicalIndex(c)))
                    continue;
                sList << tv->model()->headerData(c, Qt::Horizontal).toStringList()[i] << separator;
            }
            sList.pop_back(); // remove last separator
            sList << "\n";
        }
    }

    for(int r=minRow; r<maxRow+1; r++) {
        if (copyLabels) { // copy labels as well in table view mode
            for (QString label:tv->model()->headerData(r, Qt::Vertical).toStringList())
                sList << label << separator;
        }
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

void GdxSymbolView::moveTvFilterColumns(int from, int to)
{
    ui->tvTableViewFilter->horizontalHeader()->moveSection(from, to);
}

void GdxSymbolView::updateNumericalPrecision()
{
    QString svFull = "Full";
    if (!mSym)
        return;
    this->mSym->setNumericalPrecision(mPrecision->value(), mSqZeroes->isChecked());
    numerics::DoubleFormatter::Format format = static_cast<numerics::DoubleFormatter::Format>(mValFormat->currentData().toInt());
    this->mSym->setNumericalFormat(format);
    if (format == numerics::DoubleFormatter::g || format == numerics::DoubleFormatter::e) {
        mPrecision->setRange(numerics::DoubleFormatter::gFormatFull, 17);
        mPrecision->setSpecialValueText(svFull);
    }
    else if (format == numerics::DoubleFormatter::f) {
        mPrecision->setRange(0, 14);
        mPrecision->setSpecialValueText("");
    }
    if (mPrecision->text() == svFull && mSqZeroes->isEnabled()) {
        if (mSqZeroes->isChecked())
            mRestoreSqZeroes = true;
        mSqZeroes->setChecked(false);
        mSqZeroes->setEnabled(false);
    }
    else if (mPrecision->text() != svFull && !mSqZeroes->isEnabled()) {
        mSqZeroes->setEnabled(true);
        if (mRestoreSqZeroes) {
            mSqZeroes->setChecked(true);
            mRestoreSqZeroes = false;
        }
    }
    if (mTvModel)
        ui->tvTableView->reset();
}

void GdxSymbolView::tvFilterScrollLeft()
{
    mTvFilterSection--;
    ui->tbDomRight->setEnabled(true);
    ui->tbDomLeft->setEnabled(true);
    if (mTvFilterSection<=0) {
        mTvFilterSection=0;
        ui->tbDomLeft->setEnabled(false);
    }
    ui->tvTableViewFilter->horizontalHeader()->setOffsetToSectionPosition(mTvFilterSection);
}

void GdxSymbolView::tvFilterScrollRight()
{
    mTvFilterSection++;
    ui->tbDomRight->setEnabled(true);
    ui->tbDomLeft->setEnabled(true);
    if (mTvFilterSection >= mTvFilterSectionMax) {
        mTvFilterSection=mTvFilterSectionMax;
        ui->tbDomRight->setEnabled(false);
    }
    ui->tvTableViewFilter->horizontalHeader()->setOffsetToSectionPosition(mTvFilterSection);
}

void GdxSymbolView::showContextMenu(QPoint p)
{
    //mContextMenu.exec(ui->tvListView->mapToGlobal(p));
    if (mTableView)
        mContextMenuTV.exec(mapToGlobal(p)+ QPoint(ui->tvTableView->verticalHeader()->width(), ui->tvTableView->horizontalHeader()->height()));
    else
        mContextMenuLV.exec(mapToGlobal(p)+ QPoint(ui->tvListView->verticalHeader()->width(), ui->tvListView->horizontalHeader()->height()));
}

void GdxSymbolView::autoResizeColumns()
{
    if (mTableView) {
        ui->tvTableView->horizontalHeader()->setResizeContentsPrecision(mTVResizePrecision);
        for (int i=0; i<mTVResizeColNr; i++) {
            ui->tvTableView->resizeColumnToContents(ui->tvTableView->columnAt(0)+i);
            ui->tvTableViewFilter->resizeColumnToContents(ui->tvTableViewFilter->columnAt(0)+i);
        }
    }
    else
        ui->tvListView->resizeColumnsToContents();
}

void GdxSymbolView::autoResizeTableViewColumns(bool force)
{
    if (mTableView || force) {
        ui->tvTableView->horizontalHeader()->setResizeContentsPrecision(mTVResizePrecision);
        for (int i=0; i<mTVResizeColNr; i++)
            ui->tvTableView->resizeColumnToContents(ui->tvTableView->columnAt(0)+i);
    }
}

void GdxSymbolView::adjustDomainScrollbar()
{
    int colCount = ui->tvTableViewFilter->model()->columnCount();
    QVector<int> accSecWidth(colCount);
    int tableWidth = ui->tvTableViewFilter->horizontalHeader()->width();
    int last = 0;
    for (int i=0; i<colCount; ++i) {
        last += ui->tvTableViewFilter->horizontalHeader()->sectionSize(i);
        accSecWidth[i] = last;
    }
    if (accSecWidth.last() > tableWidth) {
        mTvFilterSectionMax = 1;

        int diff = accSecWidth.last() - tableWidth;
        for (int i=0; i<colCount; ++i) {
            if (accSecWidth[i]>=diff) {
                mTvFilterSectionMax = i+1;
                break;
            }
        }
        ui->tbDomLeft->setEnabled(mTvFilterSection != 0);
        ui->tbDomRight->setEnabled(mTvFilterSection != mTvFilterSectionMax);
    } else {
        mTvFilterSectionMax = 0;
        mTvFilterSection = 0;
        ui->tbDomLeft->setEnabled(false);
        ui->tbDomRight->setEnabled(false);
    }
}

void GdxSymbolView::showListView()
{
    mTableView = false;
    ui->tvTableView->hide();
    ui->tvTableViewFilter->hide();
    ui->tbDomLeft->hide();
    ui->tbDomRight->hide();
    ui->tvListView->show();
    ui->pbToggleView->setText("Table View");
    if (mLVFirstInit) {
        autoResizeColumns();
        mLVFirstInit = false;
    }
}

void GdxSymbolView::showTableView(int colDim, QVector<int> tvDimOrder)
{
    if (!mTvModel)
        initTableViewModel(colDim, tvDimOrder);
    else {
        if (colDim != -1)
            mTvModel->setTableView(colDim, tvDimOrder);
    }
    ui->pbToggleView->setText("List View");
    ui->tvListView->hide();
    mTableView = true;
    ui->tvTableView->show();
    QTimer::singleShot(0,this, [this](){ ui->tvTableViewFilter->show(); });
    ui->tbDomLeft->show();
    ui->tbDomRight->show();
    if (mTVFirstInit) {
        autoResizeColumns();
        mTVFirstInit = false;
    }
}

void GdxSymbolView::initTableViewModel(int colDim, QVector<int> tvDimOrder)
{
    mTvModel = new TableViewModel(mSym, mGdxSymbolTable);
    mTvModel->setTableView(colDim, tvDimOrder);
    if (mDefaultSymbolView == DefaultSymbolView::tableView)
        connect(mTvModel, &TableViewModel::initFinished, this, [this](){ if(mTVResizeOnInit) { autoResizeColumns(); mTVResizeOnInit=false;}} );
    ui->tvTableView->setModel(mTvModel);

    mTvDomainModel = new TableViewDomainModel(mTvModel);
    ui->tvTableViewFilter->setModel(mTvDomainModel);

    ui->tbDomLeft->setIcon(Theme::icon(":/%1/triangle-left"));
    ui->tbDomRight->setIcon(Theme::icon(":/%1/triangle-right"));
    QTimer::singleShot(0,this, [this](){
        int height = ui->tvTableViewFilter->horizontalHeader()->height()+2;
        ui->tvTableViewFilter->setMaximumHeight(height);
        ui->tbDomLeft->setMaximumHeight(height);
        ui->tbDomRight->setMaximumHeight(height);
        ui->tbDomLeft->setIconSize(QSize(height/2, height/2));
        ui->tbDomRight->setIconSize(QSize(height/2, height/2));
    });
}

void GdxSymbolView::showDefaultView(GdxSymbolViewState* symViewState)
{
    if (symViewState) {
        if (symViewState->tableViewActive())
            showTableView(symViewState->tvColDim(), symViewState->tvDimOrder());
        else
            showListView();
    } else {
        if (mSym->dim() > 1 && DefaultSymbolView::tableView == Settings::settings()->toInt(SettingsKey::skGdxDefaultSymbolView)) {
            showTableView();
        }
        else
            showListView();
    }
}

void GdxSymbolView::toggleView()
{
    if (mTableView)
        showListView();
    else
        showTableView();
    toggleSqueezeDefaults(mSqDefaults->isChecked());
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

void GdxSymbolView::saveTableViewHeaderState(GdxSymbolViewState* symViewState)
{
    QVector<int> widths;
    for(int col=0; col<ui->tvTableView->horizontalHeader()->count(); col++) {
        if (ui->tvTableView->horizontalHeader()->isSectionHidden(col))
            widths.append(-1);
        else
            widths.append(ui->tvTableView->horizontalHeader()->sectionSize(col));
    }
    symViewState->setTableViewColumnWidths(widths);
}

void GdxSymbolView::restoreTableViewHeaderState(GdxSymbolViewState* symViewState)
{
    QVector<int> widths = symViewState->getTableViewColumnWidths();
    for(int col=0; col<ui->tvTableView->horizontalHeader()->count(); col++) {
        if (col < widths.size()) {
            if (widths.at(col) != -1)
                ui->tvTableView->horizontalHeader()->resizeSection(col, widths.at(col));
        }
    }
}

QVector<QStringList> GdxSymbolView::pendingUncheckedLabels() const
{
    return mPendingUncheckedLabels;
}

bool GdxSymbolView::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::Resize) {
        this->adjustDomainScrollbar();
    }
    return false;
}

void GdxSymbolView::applyState(GdxSymbolViewState* symViewState)
{
    applyFilters(symViewState);

    ui->tvListView->horizontalHeader()->restoreState(symViewState->listViewHeaderState());
    mSqDefaults->setChecked(symViewState->sqDefaults());
    mSqZeroes->setChecked(symViewState->sqTrailingZeroes());
    mRestoreSqZeroes = symViewState->restoreSqZeroes();
    mPrecision->setValue(symViewState->numericalPrecision());
    mValFormat->setCurrentIndex(symViewState->valFormatIndex());

    if (symViewState->tableViewLoaded()) {
        if (!symViewState->tableViewActive()) {
            initTableViewModel(symViewState->tvColDim(), symViewState->tvDimOrder());
            autoResizeTableViewColumns(true);
        }
        restoreTableViewHeaderState(symViewState);
        ui->tvTableViewFilter->horizontalHeader()->restoreState(symViewState->tableViewFilterHeaderState());
        mTVFirstInit = false;
    }
    mLVFirstInit = false;
    if (mSym->type() == GMS_DT_EQU || mSym->type() == GMS_DT_VAR) {
        for (int i=0; i< GMS_VAL_MAX; i++)
            mShowValColActions.at(i)->setChecked(symViewState->getShowAttributes().at(i));
    }
}

void GdxSymbolView::applyFilters(GdxSymbolViewState *symViewState)
{
    // apply uel filters
    mPendingUncheckedLabels.resize(mSym->dim());
    for (int i=0; i<mSym->dim(); i++) {
        bool filterActive = false;
        if (!symViewState->uncheckedLabels().at(i).empty()) {
            for (const QString &l : symViewState->uncheckedLabels().at(i)) {
                int uel = mGdxSymbolTable->label2Uel(l);
                if (uel != -1) {
                    bool labelExistsInColumn = mSym->showUelInColumn().at(i)[uel];
                    if (!labelExistsInColumn)
                        mPendingUncheckedLabels[i].append(l);
                    filterActive = filterActive || labelExistsInColumn;
                    mSym->showUelInColumn().at(i)[uel] = false;
                } else
                    mPendingUncheckedLabels[i].append(l);
            }
        }
        mSym->setFilterActive(i, filterActive);
    }

    // apply value filters
    for (int i=0; i<mSym->numericalColumnCount(); i++) {
        ValueFilterState vfState = symViewState->valueFilterState().at(i);
        if (vfState.active) {
            mSym->setFilterActive(mSym->dim()+i);
            mSym->valueFilter(i)->setCurrentMin(vfState.min);
            mSym->valueFilter(i)->setCurrentMax(vfState.max);
            mSym->valueFilter(i)->setExclude(vfState.exclude);
            mSym->valueFilter(i)->setShowEps(vfState.showEps);
            mSym->valueFilter(i)->setShowPInf(vfState.showPInf);
            mSym->valueFilter(i)->setShowMInf(vfState.showMInf);
            mSym->valueFilter(i)->setShowNA(vfState.showNA);
            mSym->valueFilter(i)->setShowUndef(vfState.showUndef);
            mSym->valueFilter(i)->setShowAcronym(vfState.showAcronym);
        }
    }

    mSym->filterRows();
}

void GdxSymbolView::saveState(GdxSymbolViewState* symViewState)
{
    saveFilters(symViewState);
    symViewState->setSqDefaults(mSqDefaults->isChecked());
    symViewState->setSqTrailingZeroes(mSqZeroes->isChecked());
    symViewState->setRestoreSqZeroes(mRestoreSqZeroes);
    symViewState->setNumericalPrecision(mPrecision->value());
    symViewState->setValFormatIndex(mValFormat->currentIndex());

    if (mSym->type() == GMS_DT_ALIAS) {
        int idx = mSym->subType();
        GdxSymbol* aliasedSymbol = mGdxSymbolTable->gdxSymbols().at(idx);
        symViewState->setDim(aliasedSymbol->dim());
        symViewState->setType(aliasedSymbol->type());
    } else {
        symViewState->setDim(mSym->dim());
        symViewState->setType(mSym->type());
    }


    symViewState->setTableViewActive(mTableView);
    symViewState->setListViewHeaderState(ui->tvListView->horizontalHeader()->saveState());

    QVector<bool> showAttributes;
    for (QCheckBox* cb : mShowValColActions)
        showAttributes.append(cb->isChecked());
    symViewState->setShowAttributes(showAttributes);

    symViewState->setTableViewLoaded(mTvModel != nullptr);
    if (symViewState->tableViewLoaded()) {
        symViewState->setTvColDim(mTvModel->tvColDim());
        symViewState->setTvDimOrder(mTvModel->tvDimOrder());
        symViewState->setTableViewFilterHeaderState(ui->tvTableViewFilter->horizontalHeader()->saveState());
        saveTableViewHeaderState(symViewState);
    }
}

void GdxSymbolView::saveFilters(GdxSymbolViewState *symViewState)
{
    // save uel filters
    QVector<QStringList> uncheckedLabels;
    for (int i=0; i<mSym->dim(); i++) {
        QStringList labels;
        if (mSym->filterActive(i)) {
            for (int u : *mSym->uelsInColumn().at(i)) {
                if (!mSym->showUelInColumn().at(i)[u])
                    labels.append(mGdxSymbolTable->uel2Label(u));
            }
        }
        uncheckedLabels.append(labels);
    }
    symViewState->setUncheckedLabels(uncheckedLabels);

    // save value filters
    int colCount = mSym->numericalColumnCount();

    QVector<ValueFilterState> valueFilterState;
    ValueFilterState vfState;
    for (int i=0; i<colCount; i++) {
        ValueFilter* vf = mSym->valueFilter(i);
        if (mSym->filterActive(mSym->dim()+i)) {
            vfState.active = true;
            vfState.min = vf->currentMin();
            vfState.max = vf->currentMax();
            vfState.showEps = vf->showEps();
            vfState.showPInf = vf->showPInf();
            vfState.showMInf = vf->showMInf();
            vfState.showNA = vf->showNA();
            vfState.showUndef = vf->showUndef();
            vfState.showAcronym = vf->showAcronym();
            vfState.exclude = vf->exclude();
        } else
            vfState.active = false;
        valueFilterState.append(vfState);
    }
    symViewState->setValueFilterState(valueFilterState);
}

void GdxSymbolView::enableControls()
{
    if (mSym->hasInvalidUel()) {
        ui->tvListView->setSortingEnabled(false);
        ui->tvListView->horizontalHeader()->setSortIndicatorShown(false);
        ui->laError->setVisible(true);
    }

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
