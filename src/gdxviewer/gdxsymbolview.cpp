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
#include "exportdialog.h"
#include "numericalformatcontroller.h"
#include "tabenabledmenu.h"

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
    ui->laTruncatedData->hide();
    ui->laError->setStyleSheet("color:"+toColor(Theme::Normal_Red).name()+";");
    ui->laTruncatedData->setStyleSheet("color:"+toColor(Theme::Normal_Red).name()+";");

    mDefaultSymbolView = DefaultSymbolView(Settings::settings()->toInt(SettingsKey::skGdxDefaultSymbolView));

    //create context menu
    QAction* cpComma = mContextMenuLV.addAction("Copy (comma-separated)\tCtrl+C", this, [this]() { copySelectionToClipboard(","); });
    cpComma->setObjectName("edit-copy");
    mContextMenuTV.addAction(cpComma);
    cpComma->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cpComma->setShortcutVisibleInContextMenu(true);
    cpComma->setShortcutContext(Qt::WidgetShortcut);
    ui->tvListView->addAction(cpComma);
    ui->tvTableView->addAction(cpComma);

    QAction* cpTab = mContextMenuLV.addAction("Copy (tab-separated)", QKeySequence("Ctrl+Shift+C"), this, [this]() { copySelectionToClipboard("\t"); });
    mContextMenuTV.addAction(cpTab);
    cpTab->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cpTab->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(cpTab);
    ui->tvTableView->addAction(cpTab);

    mContextMenuTV.addAction("Copy Without Labels (comma-separated)", this, [this]() { copySelectionToClipboard(",", false); });
    mContextMenuTV.addAction("Copy Without Labels (tab-separated)", this, [this]() { copySelectionToClipboard("\t", false); });

    mContextMenuLV.addSeparator();
    mContextMenuTV.addSeparator();

    QAction* aResizeColumn = mContextMenuLV.addAction("Auto Resize Columns", QKeySequence("Ctrl+R"), this, [this]() { autoResizeColumns(); });
    mContextMenuTV.addAction(aResizeColumn);
    aResizeColumn->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    aResizeColumn->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(aResizeColumn);
    ui->tvTableView->addAction(aResizeColumn);

    mContextMenuLV.addSeparator();
    mContextMenuTV.addSeparator();

    QAction* aSelectAll = mContextMenuLV.addAction("Select All\tCtrl+A", this, [this]() { selectAll(); });
    aSelectAll->setObjectName("select-all");
    mContextMenuTV.addAction(aSelectAll);
    aSelectAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    aSelectAll->setShortcutVisibleInContextMenu(true);
    ui->tvListView->addAction(aSelectAll);
    ui->tvTableView->addAction(aSelectAll);

    // populate preferences
    QWidgetAction* preferences = new QWidgetAction(ui->tbPreferences);
    QVBoxLayout* vLayout = new QVBoxLayout();
    mPreferencesWidget = new QWidget(this);
    mPreferencesWidget->setAutoFillBackground(true);
    mSqDefaults = new QCheckBox("Squeeze Defaults", this);
    vLayout->addWidget(mSqDefaults);
    vLayout->setContentsMargins(6,6,6,6);
    mSqDefaults->setEnabled(false);
    mSqZeroes = new QCheckBox("Squeeze Trailing Zeroes", this);
    mSqZeroes->setChecked(true);
    vLayout->addWidget(mSqZeroes);

    QGridLayout* gridLayout = new QGridLayout();

    QLabel* lblValFormat = new QLabel("Format:", this);
    gridLayout->addWidget(lblValFormat,0,0);
    mValFormat = new QComboBox(this);
    NumericalFormatController::initFormatComboBox(mValFormat);
    resetValFormat();
    gridLayout->addWidget(mValFormat,0,1);

    QLabel* lblPrecision = new QLabel("Precision:", this);
    gridLayout->addWidget(lblPrecision,1,0);
    mPrecision = new QSpinBox(this);
    NumericalFormatController::initPrecisionSpinBox(mPrecision);

    gridLayout->addWidget(mPrecision,1,1);

    vLayout->addItem(gridLayout);
    mPreferencesWidget->setLayout(vLayout);
    preferences->setDefaultWidget(mPreferencesWidget);
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

    connect(ui->pbSearchPrev, &QPushButton::clicked, this, [this](){ onSearch(true); });
    connect(ui->pbSearchForw, &QPushButton::clicked, this, [this](){ onSearch(); });

    connect(ui->lineEdit, &FilterLineEdit::regExpChanged, this, [this](const QRegularExpression &regExp) {
        mSearchRegEx = regExp;
        ui->pbSearchPrev->setEnabled(!mSearchRegEx.pattern().isEmpty());
        ui->pbSearchForw->setEnabled(!mSearchRegEx.pattern().isEmpty());
        markSearchResults();
    });

    connect(ui->lineEdit, &QLineEdit::returnPressed, ui->pbSearchForw, &QPushButton::click);

    connect(mPrecision, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GdxSymbolView::updateNumericalPrecision);
    connect(mValFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GdxSymbolView::updateNumericalPrecision);

    ui->tvListView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tvTableView->setVerticalHeader(new NestedHeaderView(Qt::Vertical, this, ui->tvTableView));
    ui->tvTableView->setHorizontalHeader(new NestedHeaderView(Qt::Horizontal, this, ui->tvTableView));
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
    if (mTvModel)
        delete mTvModel;
    delete ui;
}

bool GdxSymbolView::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        if (mPreferencesWidget) mPreferencesWidget->setFont(font());
        if (mVisibleValColWidget) mVisibleValColWidget->setFont(font());
        if (mTvModel) {
            int height = ui->tvTableViewFilter->horizontalHeader()->height()+2;
            ui->tvTableViewFilter->setMaximumHeight(height);
            ui->tbDomLeft->setMaximumHeight(height);
            ui->tbDomRight->setMaximumHeight(height);
            ui->tbDomLeft->setIconSize(QSize(height/2, height/2));
            ui->tbDomRight->setIconSize(QSize(height/2, height/2));
        }
    }
    return QWidget::event(event);
}

void GdxSymbolView::showFilter(QPoint p)
{
    if (mSym->hasInvalidUel()) return;
    QTableView* tableView = mTableView ? ui->tvTableViewFilter : ui->tvListView;
    int column = tableView->horizontalHeader()->logicalIndexAt(p);

    if(mSym->isLoaded() && column>=0 && column<mSym->filterColumnCount()) {
        mColumnFilterMenu = new TabEnabledMenu(this);
        connect(mColumnFilterMenu, &QMenu::aboutToHide, this, &GdxSymbolView::freeFilterMenu);
        QWidgetAction *filter = nullptr;
        if (column<mSym->dim())
            filter = mSym->columnFilter(column);
        else
            filter = mSym->valueFilter(column-mSym->dim());
        mColumnFilterMenu->addAction(filter);
        mColumnFilterMenu->setFont(font());
        mColumnFilterMenu->popup(tableView->mapToGlobal(p));
        if (column<mSym->dim())
            mSym->columnFilter(column)->setFocus();
        else
            mSym->valueFilter(column-mSym->dim())->setFocus();
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
        NumericalFormatController::initFormatComboBox(mValFormat);
        NumericalFormatController::initPrecisionSpinBox(mPrecision);
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
        ui->lineEdit->setText("");
        ui->lineEdit->setOptionState(FilterLineEdit::foExact, 0);
        ui->lineEdit->setOptionState(FilterLineEdit::foRegEx, 0);
        showDefaultView();
        applyDefaults();
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
    connect(mSym, &GdxSymbol::truncatedData, this, &GdxSymbolView::setTruncatedDataVisible);
    connect(mSym, &GdxSymbol::loadFinished, this, &GdxSymbolView::enableControls);
    connect(mSym, &GdxSymbol::triggerListViewAutoResize, this, &GdxSymbolView::autoResizeColumns);
    connect(mSym, &GdxSymbol::filterChanged, this, &GdxSymbolView::toggleColumnHidden);
    showDefaultView(symViewState);
    ui->tvListView->setModel(mSym);

    if (mSym->type() == GMS_DT_EQU || mSym->type() == GMS_DT_VAR) {
        QVector<QString> valColNames;
        valColNames<< "Level" << "Marginal" << "Lower Bound" << "Upper Bound" << "Scale";
        QWidgetAction *checkableAction = new QWidgetAction(ui->tbVisibleValCols);
        mVisibleValColWidget = new QWidget();
        mVisibleValColWidget->setFont(font());
        mVisibleValColWidget->setAutoFillBackground(true);
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setContentsMargins(6,6,6,6);
        mVisibleValColWidget->setLayout(layout);
        checkableAction->setDefaultWidget(mVisibleValColWidget);
        QCheckBox *cb;
        for(int i=0; i<GMS_VAL_MAX; i++) {
            cb = new QCheckBox(valColNames[i]);
            cb->setChecked(true);
            layout->addWidget(cb);
            connect(cb, &QCheckBox::toggled, this, [this]() {toggleColumnHidden();});
            mShowValColActions.append(cb);
        }
        QPushButton *invert = new QPushButton("  Invert Selection  ", mVisibleValColWidget);
        connect(invert, &QPushButton::clicked, this, [this]() {
            for (QCheckBox *cb : std::as_const(mShowValColActions))
                cb->setChecked(!cb->isChecked());
        });
        layout->addWidget(invert);
        ui->tbVisibleValCols->addAction(checkableAction);
    }

    connect(ui->tvListView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);
    connect(ui->tvTableView, &QTableView::customContextMenuRequested, this, &GdxSymbolView::showContextMenu);
    connect(mSqZeroes, &QCheckBox::checkStateChanged, this, &GdxSymbolView::updateNumericalPrecision);
}

void GdxSymbolView::copySelectionToClipboard(const QString &separator, bool copyLabels)
{
    QString data = copySelectionToString(separator, copyLabels);
    QClipboard* clip = QApplication::clipboard();
    clip->setText(data);
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
    if (!mSym)
        return;
    mRestoreSqZeroes = NumericalFormatController::update(mValFormat, mPrecision, mSqZeroes, mRestoreSqZeroes);

    numerics::DoubleFormatter::Format format = static_cast<numerics::DoubleFormatter::Format>(mValFormat->currentData().toInt());
    this->mSym->setNumericalFormat(format);
    this->mSym->setNumericalPrecision(mPrecision->value(), mSqZeroes->isChecked());
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

void GdxSymbolView::onResizeColumnsLV()
{
    mAutoResizeLV = false;
}

void GdxSymbolView::onResizeColumnsTV()
{
    mAutoResizeTV = false;
}

TableViewModel *GdxSymbolView::getTvModel() const
{
    return mTvModel;
}

QVector<int> GdxSymbolView::listViewDimOrder()
{
    QVector<int> dimOrder;
    for (int i=0; i<mSym->columnCount(); i++) {
        int idx = ui->tvListView->horizontalHeader()->logicalIndex(i);
        if (idx<mSym->dim())
            dimOrder << idx;
    }
    return dimOrder;
}

void GdxSymbolView::setFocusSearchEdit()
{
    ui->lineEdit->setFocus();
}

void GdxSymbolView::applyDefaults()
{
    if (mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU) {
        for(int i=0; i<GMS_VAL_MAX; i++) {
            if (i == GMS_VAL_LEVEL)
                mShowValColActions.at(i)->setChecked(Settings::settings()->toBool(SettingsKey::skGdxDefaultShowLevel));
            else if (i == GMS_VAL_MARGINAL)
                mShowValColActions.at(i)->setChecked(Settings::settings()->toBool(SettingsKey::skGdxDefaultShowMarginal));
            else if (i == GMS_VAL_LOWER)
                mShowValColActions.at(i)->setChecked(Settings::settings()->toBool(SettingsKey::skGdxDefaultShowLower));
            else if (i == GMS_VAL_UPPER)
                mShowValColActions.at(i)->setChecked(Settings::settings()->toBool(SettingsKey::skGdxDefaultShowUpper));
            else if (i == GMS_VAL_SCALE)
                mShowValColActions.at(i)->setChecked(Settings::settings()->toBool(SettingsKey::skGdxDefaultShowScale));
        }
        mSqDefaults->setChecked(Settings::settings()->toBool(SettingsKey::skGdxDefaultSqueezeDefaults));
    }

    mSqZeroes->setChecked(Settings::settings()->toBool(SettingsKey::skGdxDefaultSqueezeZeroes));

    mValFormat->setCurrentIndex(Settings::settings()->toInt(SettingsKey::skGdxDefaultFormat));
    mPrecision->setValue(Settings::settings()->toInt(SettingsKey::skGdxDefaultPrecision));
    mRestoreSqZeroes = Settings::settings()->toBool(SettingsKey::skGdxDefaultRestoreSqueezeZeroes);
}

QVector<bool> GdxSymbolView::showAttributes()
{
    QVector<bool> showAttributes;
    for (QCheckBox* cb : std::as_const(mShowValColActions))
        showAttributes.append(cb->isChecked());
    return showAttributes;
}

void GdxSymbolView::showContextMenu(QPoint p)
{
    //mContextMenu.exec(ui->tvListView->mapToGlobal(p));
    if (mTableView) {
        mContextMenuTV.setFont(font());
        mContextMenuTV.exec(mapToGlobal(p)+ QPoint(ui->tvTableView->verticalHeader()->width(), ui->tvTableView->horizontalHeader()->height()+ui->tvTableView->y()));
    } else {
        mContextMenuLV.setFont(font());
        mContextMenuLV.exec(mapToGlobal(p)+ QPoint(ui->tvListView->verticalHeader()->width(), ui->tvListView->horizontalHeader()->height()+ui->tvListView->y()));
    }
}

void GdxSymbolView::autoResizeColumns()
{
    disconnect(ui->tvListView->horizontalHeader(), &QHeaderView::sectionResized, this, &GdxSymbolView::onResizeColumnsLV);
    if (mTableView) {
        autoResizeTableViewColumns();
        ui->tvTableViewFilter->resizeColumnsToContents();
    }
    else
        autoResizeListViewColumns();
    connect(ui->tvListView->horizontalHeader(), &QHeaderView::sectionResized, this, &GdxSymbolView::onResizeColumnsLV);
}

void GdxSymbolView::autoResizeTableViewColumns(bool force)
{
    disconnect(ui->tvTableView->horizontalHeader(), &QHeaderView::sectionResized, this, &GdxSymbolView::onResizeColumnsTV);
    if (mTableView || force) {
        ui->tvTableView->horizontalHeader()->setResizeContentsPrecision(mTVResizePrecision);
        for (int i=0; i<mTVResizeColNr; i++)
            ui->tvTableView->resizeColumnToContents(ui->tvTableView->columnAt(0)+i);
        mAutoResizeTV = true;
    }
    connect(ui->tvTableView->horizontalHeader(), &QHeaderView::sectionResized, this, &GdxSymbolView::onResizeColumnsTV);
}

void GdxSymbolView::autoResizeListViewColumns()
{
    ui->tvListView->resizeColumnsToContents();
    mAutoResizeLV = true;
}

void GdxSymbolView::adjustDomainScrollbar()
{
    if (!ui->tvTableViewFilter->model()) return;
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

void GdxSymbolView::showTableView(int colDim, const QVector<int> &tvDimOrder)
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

void GdxSymbolView::initTableViewModel(int colDim, const QVector<int> &tvDimOrder)
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

bool GdxSymbolView::dragInProgress() const
{
    return mDragInProgress;
}

void GdxSymbolView::setDragInProgress(bool dragInProgress)
{
    mDragInProgress = dragInProgress;
}

QString GdxSymbolView::copySelectionToString(const QString &separator, bool copyLabels)
{
    mSym->updateDecSepCopy();
    if (!ui->tvListView->model())
        return "" ;
    // row -> column -> QModelIndex
    QMap<int, QMap<int, QString>> sortedSelection;
    QTableView *tv = nullptr;
    if (mTableView)
        tv = ui->tvTableView;
    else {
        tv = ui->tvListView;
        copyLabels = false; // copy labels only available in table view mode
    }

    QModelIndexList selection = tv->selectionModel()->selection().indexes();
    if (selection.isEmpty())
        return "";

    int minRow = std::numeric_limits<int>::max();
    int maxRow = std::numeric_limits<int>::min();
    int minCol = std::numeric_limits<int>::max();
    int maxCol = std::numeric_limits<int>::min();

    for (const QModelIndex &idx : std::as_const(selection)) {
        int currentRow = idx.row();
        int currentCol = idx.column();
        if (tv->isColumnHidden(currentCol))
            continue;

        currentCol = tv->horizontalHeader()->visualIndex(currentCol);
        QString currenText = idx.data(Qt::EditRole).toString();
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
            const auto labels = tv->model()->headerData(r, Qt::Vertical).toStringList();
            for (const QString &label : labels)
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
    return sList.join("");
}

bool GdxSymbolView::isTableViewActive() const
{
    return mTableView;
}

QVector<QStringList> GdxSymbolView::pendingUncheckedLabels() const
{
    return mPendingUncheckedLabels;
}

bool GdxSymbolView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        this->adjustDomainScrollbar();
    }
    return QWidget::eventFilter(watched, event);
}

void GdxSymbolView::applyState(GdxSymbolViewState* symViewState)
{
    //applyFilters(symViewState);

    ui->tvListView->horizontalHeader()->restoreState(symViewState->listViewHeaderState());

    mAutoResizeLV = symViewState->autoResizeLV();
    mAutoResizeTV = symViewState->autoResizeTV();

    mSqDefaults->setChecked(symViewState->sqDefaults());
    mSqZeroes->setChecked(symViewState->sqTrailingZeroes());
    mRestoreSqZeroes = symViewState->restoreSqZeroes();
    mPrecision->setValue(symViewState->numericalPrecision());
    mValFormat->setCurrentIndex(symViewState->valFormatIndex());

    if (mAutoResizeLV)
        autoResizeListViewColumns();

    if (symViewState->tableViewLoaded()) {
        if (!symViewState->tableViewActive()) {
            initTableViewModel(symViewState->tvColDim(), symViewState->tvDimOrder());
            if (mAutoResizeTV)
                autoResizeTableViewColumns(true);
        }
        if (!mAutoResizeTV)
            restoreTableViewHeaderState(symViewState);
        else
            autoResizeTableViewColumns();
        ui->tvTableViewFilter->horizontalHeader()->restoreState(symViewState->tableViewFilterHeaderState());
        ui->tvTableViewFilter->resizeColumnsToContents();
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
    if (!symViewState) return;
    saveFilters(symViewState);
    symViewState->setSqDefaults(mSqDefaults->isChecked());
    symViewState->setSqTrailingZeroes(mSqZeroes->isChecked());
    symViewState->setRestoreSqZeroes(mRestoreSqZeroes);
    symViewState->setNumericalPrecision(mPrecision->value());
    symViewState->setValFormatIndex(mValFormat->currentIndex());

    symViewState->setDim(mSym->dim());
    symViewState->setType(mSym->type());

    symViewState->setTableViewActive(mTableView);
    symViewState->setListViewHeaderState(ui->tvListView->horizontalHeader()->saveState());

    symViewState->setAutoResizeLV(mAutoResizeLV);
    symViewState->setAutoResizeTV(mAutoResizeTV);

    QVector<bool> showAttributes;
    for (QCheckBox* cb : std::as_const(mShowValColActions))
        showAttributes.append(cb->isChecked());
    symViewState->setShowAttributes(showAttributes);

    symViewState->setTableViewLoaded(mTvModel != nullptr);
    if (mTvModel && symViewState->tableViewLoaded()) {
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

QList<QHeaderView *> GdxSymbolView::headers()
{
    return QList<QHeaderView*>() << ui->tvListView->horizontalHeader()
                                 << ui->tvTableView->horizontalHeader()
                                 << ui->tvTableViewFilter->horizontalHeader()
                                 << ui->tvListView->verticalHeader()
                                 << ui->tvTableView->verticalHeader();
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
    if (mSym->type() != GMS_DT_SET && mSym->type() != GMS_DT_ALIAS) {
        ui->tbPreferences->setEnabled(true);
        if(mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU) {
            mSqDefaults->setEnabled(true);
            ui->tbVisibleValCols->setEnabled(true);
        }
    }
    ui->pbResetSortFilter->setEnabled(true);
    ui->lineEdit->setEnabled(true);
    if (mSym->dim()>1)
        ui->pbToggleView->setEnabled(true);
}

bool GdxSymbolView::matchAndSelect(int row, int col, QTableView *tv) {
    bool match = false;

    // match in data for list view and table view
    if (mSearchRegEx.match(tv->model()->index(row, col).data().toString()).hasMatch())
        match = true;

    // match in header for table view only
    if (mTableView) {
        QStringList rowLabels = mTvModel->headerData(row, Qt::Vertical).toStringList();
        QStringList colLabels = mTvModel->headerData(col, Qt::Horizontal).toStringList();

        // exclude dummy headers and level/equation attributes from search
        if (mTvModel->needDummyRow())
            rowLabels.pop_back();
        if (mTvModel->needDummyColumn())
            colLabels.pop_back();
        if (mSym->type() == GMS_DT_EQU || mSym->type() == GMS_DT_VAR)
            colLabels.pop_back();

        QStringList labels = rowLabels + colLabels;
        for (const QString &s : labels) {
            if (mSearchRegEx.match(s).hasMatch())
                match = true;
        }
    }

    if (match)
        tv->selectionModel()->setCurrentIndex(tv->model()->index(row, col), QItemSelectionModel::SelectCurrent);
    return match;
}

void GdxSymbolView::onSearch(bool backward)
{
    if (mSearchRegEx.pattern().isEmpty())
        return;

    QTableView *tv = mTableView ? ui->tvTableView : ui->tvListView;
    if (tv->model()->rowCount() == 0)
        return;
    QModelIndex idx = tv->currentIndex();

    QMap<int, int> vToL;  // visual index -> logical index
    for (int i=0; i<tv->model()->columnCount(); i++) {
        if (!tv->horizontalHeader()->isSectionHidden(i))
            vToL.insert(tv->horizontalHeader()->visualIndex(i), i);
    }

    int row = idx.row();
    int visualCol = tv->horizontalHeader()->visualIndex(idx.column());
    if (!idx.isValid()) {
        if (backward) {
            row = tv->model()->rowCount();
            visualCol = vToL.firstKey();
        } else {
            row = -1;
            visualCol = vToL.lastKey();
        }
    }
    int startRow = -1;
    int startCol = -1;
    bool first = true;
    while (true) {
        auto iter = vToL.find(visualCol);
        if (backward) {
             if (iter == vToL.begin()) {
                 iter = vToL.end();
                 row--;
             }
             --iter;
        } else {
            ++iter;
            if (iter == vToL.end()) {
                iter = vToL.begin();
                row++;
            }
        }
        visualCol = iter.key();
        if (backward) {
            if (row < 0)
                row = tv->model()->rowCount()-1;
        } else {
            if (row >= tv->model()->rowCount())
                row = 0;
        }
        if (startRow == row && startCol == visualCol) {
            if (idx.isValid())  // restore previous selection if any
                tv->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent);
            return;
        }
        if (first) {
            startRow = row;
            startCol = visualCol;
            first = false;
        }
        if (matchAndSelect(row, vToL[visualCol], tv))
            return;
    }
}

void GdxSymbolView::setTruncatedDataVisible(bool visible)
{
    ui->laTruncatedData->setVisible(visible);
}

void GdxSymbolView::markSearchResults()
{
    mSym->setSearchRegEx(mSearchRegEx);
    if (mTableView) {
        ui->tvTableView->setUpdatesEnabled(false);
        ui->tvTableView->setUpdatesEnabled(true);
    } else
        ui->tvListView->viewport()->update();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
