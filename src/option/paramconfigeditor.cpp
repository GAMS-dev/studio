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
#include "common.h"
#include "definitionitemdelegate.h"
#include "configoptiondefinitionmodel.h"
#include "optionsortfilterproxymodel.h"
#include "paramconfigeditor.h"
#include "theme.h"
#include "ui_paramconfigeditor.h"
#include "headerviewproxy.h"
#include "msgbox.h"

#include <QScrollBar>
#include <QMessageBox>
#include <QMenu>
#include <QClipboard>
#include <QTimer>
#include <QStandardItemModel>

namespace gams {
namespace studio {
namespace option {

ParamConfigEditor::ParamConfigEditor(const QList<ConfigItem *> &initParams,  QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParamConfigEditor),
    mModified(false)
{
    ui->setupUi(this);
    init(initParams);
}

ParamConfigEditor::~ParamConfigEditor()
{
    delete ui;
    if (mOptionTokenizer)
        delete mOptionTokenizer;
    if (mParameterTableModel)
        delete mParameterTableModel;
    if (mOptionCompleter)
        delete mOptionCompleter;
}

void ParamConfigEditor::init(const QList<ConfigItem *> &initParamItems)
{
    initActions();

    mToolBar = new QToolBar();
    mToolBar ->setIconSize(QSize(16,16));
    mToolBar->addAction(ui->actionInsert);
    mToolBar->addAction(ui->actionDelete);
    mToolBar->addSeparator();
    mToolBar->addAction(ui->actionMoveUp);
    mToolBar->addAction(ui->actionMoveDown);
    ui->paramCfgCtrl->layout()->setMenuBar(mToolBar);

    setFocusPolicy(Qt::StrongFocus);
    mOptionTokenizer = new OptionTokenizer(GamsOptDefFile);

    QList<ParamConfigItem *> optionItem;
    for(ConfigItem* item: initParamItems) {
        optionItem.append( new ParamConfigItem(-1, item->key, item->value, item->minVersion, item->maxVersion) );
    }
    mParameterTableModel = new ConfigParamTableModel(optionItem, mOptionTokenizer, this);
    ui->paramCfgTableView->setModel( mParameterTableModel );

    mOptionCompleter = new OptionCompleterDelegate(mOptionTokenizer, ui->paramCfgTableView);
    ui->paramCfgTableView->setItemDelegate( mOptionCompleter );
    connect(mOptionCompleter, &QStyledItemDelegate::commitData, this, &ParamConfigEditor::parameterItemCommitted);

    ui->paramCfgTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::SelectedClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->paramCfgTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->paramCfgTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->paramCfgTableView->setAutoScroll(true);
    ui->paramCfgTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->paramCfgTableView->setSortingEnabled(false);

    ui->paramCfgTableView->setDragEnabled(true);
    ui->paramCfgTableView->viewport()->setAcceptDrops(true);
    ui->paramCfgTableView->setDropIndicatorShown(true);
    ui->paramCfgTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->paramCfgTableView->setDragDropOverwriteMode(true);
    ui->paramCfgTableView->setDefaultDropAction(Qt::CopyAction);

    ui->paramCfgTableView->setColumnHidden(ConfigParamTableModel::COLUMN_ENTRY_NUMBER, true);
    ui->paramCfgTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->paramCfgTableView->verticalHeader()->setDefaultSectionSize(static_cast<int>(fontMetrics().height()*TABLE_ROW_HEIGHT));
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->paramCfgTableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());

    ui->paramCfgTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_PARAM_KEY, QHeaderView::Stretch);
    ui->paramCfgTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_PARAM_VALUE, QHeaderView::Stretch);
    ui->paramCfgTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_MAX_VERSION, QHeaderView::Stretch);

    ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_KEY);
    ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_VALUE);
    ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_MIN_VERSION);
    ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_MAX_VERSION);

    QList<OptionGroup> optionGroupList = mOptionTokenizer->getOption()->getOptionGroupList();
    int groupsize = 0;
    for(const OptionGroup &group : std::as_const(optionGroupList)) {
        if (group.hidden || group.name.compare("deprecated", Qt::CaseInsensitive)==0)
            continue;
        else
            ++groupsize;
    }

    QStandardItemModel* groupModel = new QStandardItemModel(groupsize+1, 3);
    int i = 0;
    groupModel->setItem(0, 0, new QStandardItem("--- All Options ---"));
    groupModel->setItem(0, 1, new QStandardItem("0"));
    groupModel->setItem(0, 2, new QStandardItem("All Options"));
    for(const OptionGroup &group : std::as_const(optionGroupList)) {
        if (group.hidden || group.name.compare("deprecated", Qt::CaseInsensitive)==0)
            continue;
        ++i;
        groupModel->setItem(i, 0, new QStandardItem(group.description));
        groupModel->setItem(i, 1, new QStandardItem(QString::number(group.number)));
        groupModel->setItem(i, 2, new QStandardItem(group.name));
    }
    ui->paramCfgDefGroup->setModel(groupModel);
    ui->paramCfgDefGroup->setModelColumn(0);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    ConfigOptionDefinitionModel* optdefmodel =  new ConfigOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->paramCfgDefSearch, &FilterLineEdit::regExpChanged, proxymodel, [this, proxymodel]() {
        proxymodel->setFilterRegularExpression(ui->paramCfgDefSearch->regExp());
    });

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->paramCfgDefTreeView->header()->setStyle(HeaderViewProxy::instance());
    ui->paramCfgDefTreeView->setModel( proxymodel );
    ui->paramCfgDefTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->paramCfgDefTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->paramCfgDefTreeView->setDragEnabled(true);
    ui->paramCfgDefTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->paramCfgDefTreeView->setItemDelegate( new DefinitionItemDelegate(ui->paramCfgDefTreeView) );
    ui->paramCfgDefTreeView->setItemsExpandable(true);
    ui->paramCfgDefTreeView->setSortingEnabled(true);
    ui->paramCfgDefTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->paramCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
    ui->paramCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
    ui->paramCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
    ui->paramCfgDefTreeView->setExpandsOnDoubleClick(false);
    ui->paramCfgDefTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true);
    ui->paramCfgDefTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->paramCfgTableView->setTabKeyNavigation(true);

    connect(ui->paramCfgTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &ParamConfigEditor::on_selectRow, Qt::UniqueConnection);
    connect(ui->paramCfgTableView, &QTableView::customContextMenuRequested,this, &ParamConfigEditor::showParameterContextMenu, Qt::UniqueConnection);
    connect(ui->paramCfgTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParamConfigEditor::selectionChanged);
    connect(mParameterTableModel, &ConfigParamTableModel::newTableRowDropped, this, &ParamConfigEditor::on_newTableRowDropped, Qt::UniqueConnection);

    connect(ui->paramCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParamConfigEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
    connect(ui->paramCfgDefTreeView, &QTreeView::customContextMenuRequested, this, &ParamConfigEditor::showDefinitionContextMenu, Qt::UniqueConnection);
    connect(ui->paramCfgDefTreeView, &QAbstractItemView::doubleClicked, this, &ParamConfigEditor::addParameterFromDefinition, Qt::UniqueConnection);

    connect(ui->paramCfgDefGroup, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
        optdefmodel->loadOptionFromGroup( groupModel->data(groupModel->index(index, 1)).toInt() );
        mParameterTableModel->on_groupDefinitionReloaded();
    });
    connect(mParameterTableModel, &QAbstractTableModel::dataChanged, this, &ParamConfigEditor::on_dataItemChanged, Qt::UniqueConnection);
    connect(mParameterTableModel, &QAbstractTableModel::dataChanged, mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigParamTableModel::configParamModelChanged, optdefmodel, &ConfigOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigParamTableModel::configParamItemRemoved, mParameterTableModel, &ConfigParamTableModel::on_removeConfigParamItem, Qt::UniqueConnection);

    connect(this, &ParamConfigEditor::modificationChanged, this, &ParamConfigEditor::setModified, Qt::UniqueConnection);
    emit mParameterTableModel->configParamModelChanged(optionItem);

    QTimer::singleShot(0, this, [this]() {
        ui->paramCfgTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_PARAM_KEY, QHeaderView::Interactive);
        ui->paramCfgTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_PARAM_VALUE, QHeaderView::Interactive);
    });
}

void ParamConfigEditor::initActions()
{
    ui->actionInsert->setEnabled(true);
    ui->actionDelete->setEnabled(false);
    ui->actionMoveUp->setEnabled(false);
    ui->actionMoveDown->setEnabled(false);
    ui->actionSelect_Current_Row->setEnabled(true);
    ui->actionSelectAll->setEnabled(true);
    ui->actionShow_Option_Definition->setEnabled(false);
    ui->actionShowRecurrence->setEnabled(false);

    ui->actionAdd_This_Parameter->setEnabled(false);
    ui->actionRemove_This_Parameter->setEnabled(false);
    ui->actionResize_Columns_To_Contents->setEnabled(false);

    ui->actionResize_Columns_To_Contents->setEnabled(true);

    ui->actionInsert->setIcon(Theme::icon(":/%1/plus", true));
    ui->actionDelete->setIcon(Theme::icon(":/%1/delete-all", true));
    ui->actionMoveUp->setIcon(Theme::icon(":/%1/move-up", true));
    ui->actionMoveDown->setIcon(Theme::icon(":/%1/move-down", true));

    ui->actionAdd_This_Parameter->setIcon(Theme::icon(":/%1/plus", true));
    ui->actionRemove_This_Parameter->setIcon(Theme::icon(":/%1/delete-all", true));
}

bool ParamConfigEditor::isInFocus(QWidget *focusWidget) const
{
    return (focusWidget==ui->paramCfgTableView || focusWidget==ui->paramCfgDefTreeView);
}

QList<QHeaderView *> ParamConfigEditor::headers()
{
    return QList<QHeaderView *>() << ui->paramCfgTableView->horizontalHeader()
                                  << ui->paramCfgDefTreeView->header()
                                  << ui->paramCfgTableView->verticalHeader();
}

QString ParamConfigEditor::getSelectedParameterName(QWidget *widget) const
{
    if (widget == ui->paramCfgTableView) {
        QModelIndexList selection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
        if (selection.count() > 0) {
            const QModelIndex index = selection.at(0);
            const QVariant headerData = ui->paramCfgTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::PartiallyChecked) {
                return "";
            }
            const QVariant data = ui->paramCfgTableView->model()->data( index.sibling(index.row(),0) );
            if (mOptionTokenizer->getOption()->isValid(data.toString()))
               return data.toString();
            else if (mOptionTokenizer->getOption()->isASynonym(data.toString()))
                    return mOptionTokenizer->getOption()->getNameFromSynonym(data.toString());
            else
               return "";
        }
    } else if (widget == ui->paramCfgDefTreeView) {
        const QModelIndexList selection = ui->paramCfgDefTreeView->selectionModel()->selectedRows();
        if (selection.count() > 0) {
            const QModelIndex index = selection.at(0);
            const QModelIndex  parentIndex =  ui->paramCfgDefTreeView->model()->parent(index);
            if (parentIndex.row() >= 0) {
                return ui->paramCfgDefTreeView->model()->data( parentIndex.sibling(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            } else {
                return ui->paramCfgDefTreeView->model()->data( index.sibling(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            }
        }
    }
    return "";

}

void ParamConfigEditor::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    if (selected.isEmpty()) {
        updateActionsState();
        return;
    }

    updateActionsState(selected.indexes().first());
}

void ParamConfigEditor::updateActionsState(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QModelIndexList idxSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    bool thereIsSelection = (isThereARow() && !idxSelection.isEmpty());

    bool singleSelection = (idxSelection.size() ==1);
    bool singleSelectionIsRow = (singleSelection && ui->paramCfgTableView->selectionModel()->isRowSelected(idxSelection.first().row()));
    bool multiSelectionIsRow = false;
    bool multiSelectionIsCell_sameRow = multiSelectionIsRow;
    if (!singleSelection) {
         QList<int> rowList;
         for (QModelIndex& idx :idxSelection) {
             if (ui->paramCfgTableView->selectionModel()->isRowSelected(idx.row())) {
                 multiSelectionIsRow = true;
                 break;
             }
             if (rowList.contains(idx.row())) {
                 continue;
             }
             rowList.append(idx.row());
         }
         multiSelectionIsCell_sameRow = (rowList.size() == 1);
    }

    ui->actionInsert->setEnabled( true );
    ui->actionDelete->setEnabled( thereIsSelection && idxSelection.first().row() < mParameterTableModel->rowCount() );
    ui->actionMoveUp->setEnabled(   (singleSelection || singleSelectionIsRow || multiSelectionIsRow || multiSelectionIsCell_sameRow) && idxSelection.first().row() > 0 );
    ui->actionMoveDown->setEnabled( (singleSelection || singleSelectionIsRow || multiSelectionIsRow || multiSelectionIsCell_sameRow) && idxSelection.last().row() < mParameterTableModel->rowCount()-1 );
    ui->actionSelect_Current_Row->setEnabled( thereIsSelection );
    ui->actionSelectAll->setEnabled( thereIsSelection );
    ui->actionShow_Option_Definition->setEnabled( thereIsSelection && index.row() < mParameterTableModel->rowCount() );
    ui->actionResize_Columns_To_Contents->setEnabled( thereIsSelection );
    ui->actionShowRecurrence->setEnabled( false );

    ui->actionInsert->icon().pixmap( QSize(16, 16), ui->actionInsert->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                    QIcon::Off);
    ui->actionDelete->icon().pixmap( QSize(16, 16), ui->actionDelete->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                    QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), ui->actionMoveUp->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                    QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), ui->actionMoveDown->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                    QIcon::Off);
    mToolBar->repaint();
}

void ParamConfigEditor::updateActionsState()
{
    QModelIndexList idxSelection = ( ui->paramCfgTableView->selectionModel()->selectedRows().isEmpty()
                                         ? ui->paramCfgTableView->selectionModel()->selectedIndexes()
                                         : ui->paramCfgTableView->selectionModel()->selectedRows() );

    bool thereIsSelection = (isThereARow() && !idxSelection.isEmpty());

    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });

    ui->actionInsert->setEnabled( true );
    ui->actionDelete->setEnabled(  thereIsSelection ? idxSelection.first().row() < mParameterTableModel->rowCount() : false );

    ui->actionMoveUp->setEnabled( thereIsSelection ? idxSelection.first().row() > 0 : false );
    ui->actionMoveDown->setEnabled( thereIsSelection ? idxSelection.last().row() < mParameterTableModel->rowCount()-1 : false );

    ui->actionSelect_Current_Row->setEnabled( thereIsSelection );
    ui->actionSelectAll->setEnabled( thereIsSelection );

    ui->actionShow_Option_Definition->setEnabled( thereIsSelection ? idxSelection.first().row() < mParameterTableModel->rowCount() : false );
    ui->actionResize_Columns_To_Contents->setEnabled( thereIsSelection ? idxSelection.first().row() < mParameterTableModel->rowCount() : false);
    ui->actionShowRecurrence->setEnabled( thereIsSelection ? idxSelection.first().row() < mParameterTableModel->rowCount()
                                          && getRecurrentOption(idxSelection.first()).size() >0 : false );
    ui->actionInsert->icon().pixmap( QSize(16, 16), ui->actionInsert->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                    QIcon::Off);
    ui->actionDelete->icon().pixmap( QSize(16, 16), ui->actionDelete->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                    QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), ui->actionMoveUp->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                    QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), ui->actionMoveDown->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                    QIcon::Off);
    mToolBar->repaint();
}

void ParamConfigEditor::updateDefinitionActionsState(const QModelIndex &index)
{
    QModelIndex parentIndex =  ui->paramCfgDefTreeView->model()->parent(index);
    QVariant data = (parentIndex.row() < 0) ? ui->paramCfgDefTreeView->model()->data(index, Qt::CheckStateRole)
                                            : ui->paramCfgDefTreeView->model()->data(parentIndex, Qt::CheckStateRole);
    ui->actionAdd_This_Parameter->setEnabled( Qt::CheckState(data.toInt()) == Qt::Unchecked );
    ui->actionRemove_This_Parameter->setEnabled( Qt::CheckState(data.toInt()) == Qt::Checked );
    ui->actionResize_Columns_To_Contents->setEnabled( true );

    ui->actionAdd_This_Parameter->icon().pixmap( QSize(16, 16), ui->actionAdd_This_Parameter->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                                QIcon::Off);
    ui->actionRemove_This_Parameter->icon().pixmap( QSize(16, 16), ui->actionRemove_This_Parameter ? QIcon::Selected : QIcon::Disabled,
                                                                   QIcon::Off);
}

void ParamConfigEditor::showParameterContextMenu(const QPoint &pos)
{
    QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : std::as_const(indexSelection)) {
        ui->paramCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    updateActionsState();

    QMenu menu(this);
    menu.addAction(ui->actionInsert);
    menu.addAction(ui->actionDelete);
    menu.addSeparator();
    menu.addAction(ui->actionMoveUp);
    menu.addAction(ui->actionMoveDown);
    menu.addSeparator();
    menu.addAction(ui->actionSelect_Current_Row);
    menu.addAction(ui->actionSelectAll);
    menu.addSeparator();
    menu.addAction(ui->actionShow_Option_Definition);
    menu.addAction(ui->actionShowRecurrence);
    menu.addSeparator();
    menu.addAction(ui->actionResize_Columns_To_Contents);

    menu.exec(ui->paramCfgTableView->viewport()->mapToGlobal(pos));
}

void ParamConfigEditor::showDefinitionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->paramCfgDefTreeView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    QMenu menu(this);
    menu.addAction(ui->actionAdd_This_Parameter);
    menu.addAction(ui->actionRemove_This_Parameter);
    menu.addSeparator();
    menu.addAction(ui->actionResize_Columns_To_Contents);
    menu.exec(ui->paramCfgDefTreeView->viewport()->mapToGlobal(pos));
}

void ParamConfigEditor::parameterItemCommitted(QWidget *editor)
{
    Q_UNUSED(editor)
    if (mOptionCompleter->currentEditedIndex().isValid()) {
        ui->paramCfgTableView->resizeColumnToContents( mOptionCompleter->currentEditedIndex().column() );
        ui->paramCfgTableView->selectionModel()->select( mOptionCompleter->currentEditedIndex(), QItemSelectionModel::ClearAndSelect );
        ui->paramCfgTableView->setCurrentIndex( mOptionCompleter->currentEditedIndex() );
        ui->paramCfgTableView->setFocus();
    }
}

void ParamConfigEditor::on_selectRow(int logicalIndex)
{
    if (ui->paramCfgTableView->model()->rowCount() <= 0)
        return;

    QItemSelectionModel *selectionModel = ui->paramCfgTableView->selectionModel();
    QModelIndex topLeft = ui->paramCfgTableView->model()->index(logicalIndex, ConfigParamTableModel::COLUMN_PARAM_KEY, QModelIndex());
    QModelIndex  bottomRight = ui->paramCfgTableView->model()->index(logicalIndex, ConfigParamTableModel::COLUMN_ENTRY_NUMBER, QModelIndex());
    QItemSelection selection( topLeft, bottomRight);
    selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    updateActionsState();
}

void ParamConfigEditor::on_reloadGamsUserConfigFile(const QList<ConfigItem *> &initParams)
{
    QList<ParamConfigItem *> optionItem;
    for(ConfigItem* item: initParams) {
        optionItem.append( new ParamConfigItem(-1, item->key, item->value, item->minVersion, item->maxVersion) );
    }
    mParameterTableModel->on_reloadConfigParamModel(optionItem);
    setModified(false);
}

void ParamConfigEditor::selectAll()
{
    ui->paramCfgTableView->setFocus();
    ui->paramCfgTableView->selectAll();

    updateActionsState( );
}

void ParamConfigEditor::findAndSelectionParameterFromDefinition()
{
    if (ui->paramCfgTableView->model()->rowCount() <= 0)
        return;

    const QModelIndex index = ui->paramCfgDefTreeView->selectionModel()->currentIndex();

    updateDefinitionActionsState(index);

    const QModelIndex parentIndex =  ui->paramCfgDefTreeView->model()->parent(index);

    const QModelIndex idx = (parentIndex.row()<0) ? ui->paramCfgDefTreeView->model()->index( index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER )
                                            : ui->paramCfgDefTreeView->model()->index( parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER );
    const QVariant data = ui->paramCfgDefTreeView->model()->data( idx, Qt::DisplayRole );
    QModelIndexList indices = ui->paramCfgTableView->model()->match(ui->paramCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                       Qt::DisplayRole,
                                                                       data, -1, Qt::MatchExactly|Qt::MatchRecursive);
    ui->paramCfgTableView->clearSelection();
    ui->paramCfgTableView->clearFocus();
    QItemSelection selection;
    for(const QModelIndex i :std::as_const(indices)) {
        const QModelIndex valueIndex = ui->paramCfgTableView->model()->index(i.row(), ConfigParamTableModel::COLUMN_PARAM_VALUE);
        const QString value =  ui->paramCfgTableView->model()->data( valueIndex, Qt::DisplayRole).toString();
        bool selected = false;
        if (parentIndex.row() < 0) {
            selected = true;
        } else {
            const QModelIndex enumIndex = ui->paramCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex);
            const QString enumValue = ui->paramCfgDefTreeView->model()->data( enumIndex, Qt::DisplayRole).toString();
            if (QString::compare(value, enumValue, Qt::CaseInsensitive)==0)
                selected = true;
        }
        if (selected) {
           const QModelIndex leftIndex  = ui->paramCfgTableView->model()->index(i.row(), 0);
           const QModelIndex rightIndex = ui->paramCfgTableView->model()->index(i.row(), ui->paramCfgTableView->model()->columnCount() -1);

           const QItemSelection rowSelection(leftIndex, rightIndex);
           selection.merge(rowSelection, QItemSelectionModel::Select);
        }
        ui->actionAdd_This_Parameter->setEnabled(!selected);
        ui->actionRemove_This_Parameter->setEnabled(selected);
    }

    ui->paramCfgTableView->selectionModel()->select(selection, QItemSelectionModel::Select);
    ui->paramCfgDefTreeView->setFocus();

}

void ParamConfigEditor::selectAnOption()
{
    QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    if (indexSelection.empty())
        indexSelection <<  ui->paramCfgTableView->selectionModel()->currentIndex();

    QList<int> rowIndex;
    for(int i=0; i<indexSelection.count(); ++i) {
        if (!rowIndex.contains(i)) {
            rowIndex << i;
            on_selectRow( indexSelection.at(i).row() );
        }
    }
}

void ParamConfigEditor::deleteOption()
{
    QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->paramCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        const QItemSelection selection( ui->paramCfgTableView->selectionModel()->selection() );

        QList<int> rows;
        const auto indexes = ui->paramCfgTableView->selectionModel()->selectedRows();
        for(const QModelIndex & index : indexes) {
            rows.append( index.row() );
        }

        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            const int current = rows[i];
            if (current != prev) {
                const QString text = mParameterTableModel->getParameterTableEntry(current);
                ui->paramCfgTableView->model()->removeRows( current, 1 );
                mOptionTokenizer->logger()->append(QString("Option entry '%1' has been deleted").arg(text), LogMsgType::Info);
                prev = current;
            }
        }
        emit modificationChanged(true);
    }
    initActions();
}

void ParamConfigEditor::deSelect()
{
    initActions();
    if (ui->paramCfgTableView->hasFocus() && ui->paramCfgTableView->selectionModel()->hasSelection())
        ui->paramCfgTableView->selectionModel()->clearSelection();
    else if (ui->paramCfgDefTreeView->hasFocus() && ui->paramCfgDefTreeView->selectionModel()->hasSelection())
             ui->paramCfgDefTreeView->selectionModel()->clearSelection();
    this->focusNextChild();
}

void ParamConfigEditor::setModified(bool modified)
{
    mModified = modified;
}

bool ParamConfigEditor::isModified() const
{
    return mModified;
}

void ParamConfigEditor::selectSearchField() const
{
    ui->paramCfgDefSearch->setFocus();
}

QList<ConfigItem *> ParamConfigEditor::parameterConfigItems()
{
    QList<ConfigItem *> itemList;
    for(ParamConfigItem* item : mParameterTableModel->parameterConfigItems()) {
        itemList.append( new ConfigItem(item->key, item->value, item->minVersion, item->maxVersion) );
    }
    return itemList;
}

void ParamConfigEditor::addParameterFromDefinition(const QModelIndex &index)
{
    emit modificationChanged(true);

    const QModelIndex parentIndex =  ui->paramCfgDefTreeView->model()->parent(index);
    const QModelIndex optionNameIndex = (parentIndex.row()<0) ? ui->paramCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                              : ui->paramCfgDefTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    const QModelIndex defValueIndex = (parentIndex.row()<0) ? ui->paramCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE)
                                                            : ui->paramCfgDefTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    const QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex
                                                                 : ui->paramCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex) ;

    disconnect( mParameterTableModel, &QAbstractTableModel::dataChanged,  mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem);

    bool replaceExistingEntry = false;
    const QString optionNameData = ui->paramCfgDefTreeView->model()->data(optionNameIndex).toString();
    const QModelIndex optionIdIndex = (parentIndex.row()<0) ? ui->paramCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER)
                                                            : ui->paramCfgDefTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) ;
    const QVariant optionIdData = ui->paramCfgDefTreeView->model()->data(optionIdIndex);

    int rowToBeAdded = ui->paramCfgTableView->model()->rowCount();
//    StudioSettings* settings = SettingsLocator::settings();
//    if (settings && settings->overridExistingOption()) {
        QModelIndexList indices = ui->paramCfgTableView->model()->match(ui->paramCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                            Qt::DisplayRole,
                                                                            optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
        ui->paramCfgTableView->clearSelection();
        QItemSelection selection;
        for(const QModelIndex &idx: indices) {
            const QModelIndex leftIndex  = ui->paramCfgTableView->model()->index(idx.row(), ConfigParamTableModel::COLUMN_PARAM_KEY);
            const QModelIndex rightIndex = ui->paramCfgTableView->model()->index(idx.row(), ConfigParamTableModel::COLUMN_ENTRY_NUMBER);
            const QItemSelection rowSelection(leftIndex, rightIndex);
            selection.merge(rowSelection, QItemSelectionModel::Select);
        }
        ui->paramCfgTableView->selectionModel()->select(selection, QItemSelectionModel::Select);

        const bool singleEntryExisted = (indices.size()==1);
        const bool multipleEntryExisted = (indices.size()>1);
        if (singleEntryExisted ) {
            const QString detailText = QString("Entry:  '%1'\nDescription:  %2 %3")
                .arg(getParameterTableEntry(indices.at(0).row()),
                "When a GAMS config file contains multiple entries of the same parameter, only the value of the last entry will be utilized by GAMS.",
                "The value of all other entries except the last entry will be ignored.");
            const int answer = MsgBox::question("Parameter Entry exists", "Parameter '" + optionNameData + "' already exists.",
                                          "How do you want to proceed?", detailText,
                                          nullptr, "Replace existing entry", "Add new entry", "Abort", 2, 2);
            switch(answer) {
            case 0: // replace
                replaceExistingEntry = true;
                indices = ui->paramCfgTableView->model()->match(ui->paramCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                    Qt::DisplayRole,
                                                                    optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
                rowToBeAdded = (indices.size()>0) ? indices.at(0).row() : 0;
                break;
            case 1: // add
                break;
            default:
                return;
            }
        } else if (multipleEntryExisted) {
            QString entryDetailedText = QString("Entries:\n");
            int i = 0;
            for (const QModelIndex &idx : indices)
                entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getParameterTableEntry(idx.row())));
            const QString detailText = QString("%1Description:  %2 %3").arg(entryDetailedText,
                "When a GAMS config file contains multiple entries of the same parameter, only the value of the last entry will be utilized by GAMS.",
                "The value of all other entries except the last entry will be ignored.");
            int answer = MsgBox::question("Multiple Parameter Entries exist",
                                          "Multiple entries of Parameter '" + optionNameData + "' already exist.",
                                          "How do you want to proceed?", detailText, nullptr,
                                          "Replace first entry and delete other entries", "Add new entry", "Abort", 2, 2);
            switch(answer) {
            case 0: // delete and replace
                disconnect( mParameterTableModel, &ConfigParamTableModel::configParamItemRemoved, mParameterTableModel, &ConfigParamTableModel::on_removeConfigParamItem);
                ui->paramCfgTableView->selectionModel()->clearSelection();
                for(int i=1; i<indices.size(); i++) {
                    ui->paramCfgTableView->selectionModel()->select( indices.at(i), QItemSelectionModel::Select|QItemSelectionModel::Rows );
                }
                deleteOption();
                connect( mParameterTableModel, &ConfigParamTableModel::configParamItemRemoved,
                         mParameterTableModel, &ConfigParamTableModel::on_removeConfigParamItem, Qt::UniqueConnection);
                replaceExistingEntry = true;
                indices = ui->paramCfgTableView->model()->match(ui->paramCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                    Qt::DisplayRole,
                                                                    optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
                rowToBeAdded = (indices.size()>0) ? indices.at(0).row() : 0;
                break;
            case 1: // add
                break;
            default:
                return;
            }

        } // else entry not exist
//    }

    ui->paramCfgTableView->selectionModel()->clearSelection();
//    QString synonymData = ui->ParamCfgDefTreeView->model()->data(synonymIndex).toString();
    const QString selectedValueData = ui->paramCfgDefTreeView->model()->data(selectedValueIndex).toString();
    mOptionTokenizer->getOption()->setModified(optionNameData, true);
    ui->paramCfgDefTreeView->model()->setData(optionNameIndex, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);

    // insert option row
    if (rowToBeAdded == ui->paramCfgTableView->model()->rowCount()) {
        ui->paramCfgTableView->model()->insertRows(rowToBeAdded, 1, QModelIndex());
    }
    const QModelIndex insertKeyIndex = ui->paramCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_PARAM_KEY);
    const QModelIndex insertValueIndex = ui->paramCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    const QModelIndex minVersionIndex = ui->paramCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_MIN_VERSION);
    const QModelIndex maxVersionIndex = ui->paramCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_MIN_VERSION);
    ui->paramCfgTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->paramCfgTableView->model()->setData( insertValueIndex, (selectedValueData.simplified().isEmpty()
                                                                   ? OptionTokenizer::valueGeneratedStr
                                                                   : selectedValueData)
                                                             , Qt::EditRole);
    ui->paramCfgTableView->model()->setData( minVersionIndex, "", Qt::EditRole);
    ui->paramCfgTableView->model()->setData( maxVersionIndex, "", Qt::EditRole);

    QModelIndex insertNumberIndex = ui->paramCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_ENTRY_NUMBER);
    const int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;
    ui->paramCfgTableView->model()->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    if (selectedValueData.isEmpty())
        ui->paramCfgTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Checked), Qt::CheckStateRole );
    else
       ui->paramCfgTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
    ui->paramCfgTableView->selectRow(rowToBeAdded);
    selectAnOption();

    const QString text =  mParameterTableModel->getParameterTableEntry(insertNumberIndex.row());
    if (replaceExistingEntry)
        mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been replaced").arg(text), LogMsgType::Info);
    else
        mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been added").arg(text), LogMsgType::Info);

    const int lastColumn = ui->paramCfgTableView->model()->columnCount()-1;
    const int lastRow = rowToBeAdded;
    int firstRow = lastRow;
    if (firstRow<0)
        firstRow = 0;
     mParameterTableModel->on_updateConfigParamItem( ui->paramCfgTableView->model()->index(firstRow, lastColumn),
                                                     ui->paramCfgTableView->model()->index(lastRow, lastColumn),
                                                     {Qt::EditRole});

     ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_KEY);
     ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_VALUE);

    connect( mParameterTableModel, &QAbstractTableModel::dataChanged,  mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);

    showOptionDefinition(true);

    if (parentIndex.row()<0) {
        if (mOptionTokenizer->getOption()->getOptionSubType(optionNameData) != optsubNoValue)
            ui->paramCfgTableView->edit(insertValueIndex);
    }

}

void ParamConfigEditor::showOptionDefinition(bool selectRow)
{
    if (ui->paramCfgTableView->model()->rowCount() <= 0)
        return;

    const QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    if (indexSelection.count() <= 0)
        return;

    disconnect(ui->paramCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
               this, &ParamConfigEditor::findAndSelectionParameterFromDefinition);

    ui->paramCfgDefSearch->clear();

    QModelIndexList selection;
    if (selectRow) {
       selectAnOption();
       selection = ui->paramCfgTableView->selectionModel()->selectedRows();
    } else {
         selection = indexSelection;
         ui->paramCfgTableView->selectionModel()->setCurrentIndex ( selection.first(), QItemSelectionModel::Select );
    }

    QModelIndexList selectIndices;
    for (int i=0; i<selection.count(); i++) {
         const QModelIndex index = selection.at(i);
         if (Qt::CheckState(ui->paramCfgTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
                continue;

         const QString value = ui->paramCfgTableView->model()->data( index.sibling(index.row(), ConfigParamTableModel::COLUMN_PARAM_VALUE), Qt::DisplayRole).toString();
         const QVariant optionId = ui->paramCfgTableView->model()->data( index.sibling(index.row(), ConfigParamTableModel::COLUMN_ENTRY_NUMBER), Qt::DisplayRole);
         QModelIndexList indices = ui->paramCfgDefTreeView->model()->match(ui->paramCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                                                               Qt::DisplayRole,
                                                                               optionId, 1, Qt::MatchExactly|Qt::MatchRecursive);
         for(const QModelIndex idx : std::as_const(indices)) {
             const QModelIndex  parentIndex =  ui->paramCfgDefTreeView->model()->parent(idx);
             const QModelIndex optionIdx = ui->paramCfgDefTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);

             if (parentIndex.row() < 0) {
                if (ui->paramCfgDefTreeView->model()->hasChildren(optionIdx) && !ui->paramCfgDefTreeView->isExpanded(optionIdx))
                    ui->paramCfgDefTreeView->expand(optionIdx);
            }
            bool found = false;
            for(int r=0; r <ui->paramCfgDefTreeView->model()->rowCount(optionIdx); ++r) {
                const QModelIndex i = ui->paramCfgDefTreeView->model()->index(r, OptionDefinitionModel::COLUMN_OPTION_NAME, optionIdx);
                QString enumValue = ui->paramCfgDefTreeView->model()->data(i, Qt::DisplayRole).toString();
                if (QString::compare(value, enumValue, Qt::CaseInsensitive) == 0) {
                    selectIndices << i;
                    found = true;
                    break;
                }
            }
            if (!found)
               selectIndices << optionIdx;
        }
    }
    ui->paramCfgDefTreeView->selectionModel()->clearSelection();
    for(const QModelIndex idx : std::as_const(selectIndices)) {
        QItemSelection selection = ui->paramCfgDefTreeView->selectionModel()->selection();
        const QModelIndex  parentIdx =  ui->paramCfgDefTreeView->model()->parent(idx);
        if (parentIdx.row() < 0) {
            selection.select(ui->paramCfgDefTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME),
                             ui->paramCfgDefTreeView->model()->index(idx.row(), ui->paramCfgDefTreeView->model()->columnCount()-1));
        } else  {
            selection.select(ui->paramCfgDefTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIdx),
                             ui->paramCfgDefTreeView->model()->index(idx.row(), ui->paramCfgDefTreeView->model()->columnCount()-1, parentIdx));
        }
        ui->paramCfgDefTreeView->selectionModel()->select(selection, QItemSelectionModel::Select);
    }
    if (selectIndices.size() > 0) {
        const QModelIndex parentIndex = ui->paramCfgDefTreeView->model()->parent(selectIndices.first());
        const QModelIndex scrollToIndex = (parentIndex.row() < 0  ? ui->paramCfgDefTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                            : ui->paramCfgDefTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex));
        ui->paramCfgDefTreeView->scrollTo(scrollToIndex, QAbstractItemView::EnsureVisible);
        if (parentIndex.row() >= 0) {
            ui->paramCfgDefTreeView->scrollTo(parentIndex, QAbstractItemView::EnsureVisible);
            const QRect r = ui->paramCfgDefTreeView->visualRect(parentIndex);
            ui->paramCfgDefTreeView->horizontalScrollBar()->setValue(r.x());
        } else {
            const QRect r = ui->paramCfgDefTreeView->visualRect(scrollToIndex);
            ui->paramCfgDefTreeView->horizontalScrollBar()->setValue(r.x());
        }
    }

    connect(ui->paramCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParamConfigEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParamConfigEditor::copyDefinitionToClipboard(int column)
{
    if (ui->paramCfgDefTreeView->selectionModel()->selectedRows().count() <= 0)
        return;

    const QModelIndex index = ui->paramCfgDefTreeView->selectionModel()->selectedRows().at(0);
    if (!index.isValid())
        return;

    QString text = "";
    const QModelIndex parentIndex = ui->paramCfgDefTreeView->model()->parent(index);
    if (column == -1) { // copy all
        QStringList strList;
        if (parentIndex.isValid()) {
            QModelIndex idx = ui->paramCfgDefTreeView->model()->index(index.row(), ConfigOptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex);
            strList << ui->paramCfgDefTreeView->model()->data(idx, Qt::DisplayRole).toString();
            idx = ui->paramCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DESCIPTION, parentIndex);
            strList << ui->paramCfgDefTreeView->model()->data(idx, Qt::DisplayRole).toString();
            text = strList.join(", ");
        } else {
           for (int j=0; j<ui->paramCfgDefTreeView->model()->columnCount(); j++) {
               if (j==ConfigOptionDefinitionModel::COLUMN_ENTRY_NUMBER)
                  continue;
               const QModelIndex columnindex = ui->paramCfgDefTreeView->model()->index(index.row(), j);
               strList << ui->paramCfgDefTreeView->model()->data(columnindex, Qt::DisplayRole).toString();
           }
           text = strList.join(", ");
        }
    } else {
        if (parentIndex.isValid()) {
            QModelIndex idx = ui->paramCfgDefTreeView->model()->index(index.row(), column, parentIndex);
            text = ui->paramCfgDefTreeView->model()->data( idx, Qt::DisplayRole ).toString();
        } else {
            text = ui->paramCfgDefTreeView->model()->data( ui->paramCfgDefTreeView->model()->index(index.row(), column), Qt::DisplayRole ).toString();
        }
    }
    QClipboard* clip = QApplication::clipboard();
    clip->setText( text );
}

void ParamConfigEditor::on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    emit modificationChanged(true);

    QModelIndexList toDefinitionItems = ui->paramCfgDefTreeView->model()->match(ui->paramCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     ui->paramCfgTableView->model()->data( topLeft, Qt::DisplayRole), 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->paramCfgDefTreeView->model()->match(ui->paramCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                         Qt::DisplayRole,
                                                                         ui->paramCfgTableView->model()->data( topLeft, Qt::DisplayRole), 1);
    }

    ui->paramCfgDefTreeView->clearSelection();
    for(const QModelIndex item : std::as_const(toDefinitionItems)) {
        ui->paramCfgDefTreeView->selectionModel()->select(
                    QItemSelection (
                        ui->paramCfgDefTreeView->model ()->index (item.row() , 0),
                        ui->paramCfgDefTreeView->model ()->index (item.row(), ui->paramCfgDefTreeView->model ()->columnCount () - 1)),
                    QItemSelectionModel::Select);
        ui->paramCfgDefTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
    ui->paramCfgTableView->selectionModel()->select(topLeft, QItemSelectionModel::Select);

    updateActionsState(topLeft);
}

void ParamConfigEditor::on_newTableRowDropped(const QModelIndex &index)
{
    disconnect(ui->paramCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParamConfigEditor::findAndSelectionParameterFromDefinition);
    ui->paramCfgTableView->selectRow(index.row());

    const QString optionName = ui->paramCfgTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->paramCfgDefTreeView->model()->match(ui->paramCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(const QModelIndex item : std::as_const(definitionItems)) {
        ui->paramCfgDefTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    if (mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumStr &&
        mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumInt &&
        mOptionTokenizer->getOption()->getOptionSubType(optionName) != optsubNoValue)
        ui->paramCfgTableView->edit( mParameterTableModel->index(index.row(), ConfigParamTableModel::COLUMN_PARAM_VALUE));

    showOptionDefinition(true);
    connect(ui->paramCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParamConfigEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

QList<int> ParamConfigEditor::getRecurrentOption(const QModelIndex &index)
{
    QList<int> optionList;

//    if (!isInFocus(focusWidget()))
//        return optionList;

    QVariant data = ui->paramCfgTableView->model()->headerData(index.row(), Qt::Vertical,  Qt::CheckStateRole);
    if (Qt::CheckState(data.toUInt())==Qt::PartiallyChecked)
        return optionList;

    const QString optionId = ui->paramCfgTableView->model()->data( index.sibling(index.row(), ConfigParamTableModel::COLUMN_ENTRY_NUMBER), Qt::DisplayRole).toString();
    QModelIndexList indices = ui->paramCfgTableView->model()->match(ui->paramCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                        Qt::DisplayRole,
                                                                        optionId, -1);
    for(const QModelIndex idx : std::as_const(indices)) {
        if (idx.row() == index.row())
            continue;
        else
            optionList << idx.row();
    }
    return optionList;
}

QString ParamConfigEditor::getParameterTableEntry(int row)
{
    const QModelIndex keyIndex = ui->paramCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_PARAM_KEY);
    const QVariant optionKey = ui->paramCfgTableView->model()->data(keyIndex, Qt::DisplayRole);
    const QModelIndex valueIndex = ui->paramCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    const QVariant optionValue = ui->paramCfgTableView->model()->data(valueIndex, Qt::DisplayRole);
    const QModelIndex minVersionIndex = ui->paramCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    const QVariant minVersionValue = ui->paramCfgTableView->model()->data(minVersionIndex, Qt::DisplayRole);
    const QModelIndex maxVersionIndex = ui->paramCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    const QVariant maxVersionValue = ui->paramCfgTableView->model()->data(maxVersionIndex, Qt::DisplayRole);
    return QString("%1%2%3 %4 %5").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(),
                                       optionValue.toString(), minVersionValue.toString(), maxVersionValue.toString());
}

//void ParamConfigEditor::deleteParameter()
//{
//     QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
//     for(QModelIndex index : indexSelection) {
//         ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
//     }

//     QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
//     if (selection.count() <= 0)
//        return;

//    disconnect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
//               this, &ParamConfigEditor::findAndSelectionParameterFromDefinition);

//    QModelIndex index = selection.at(0);
//    QModelIndex removeTableIndex = ui->ParamCfgTableView->model()->index(index.row(), 0);
//    QVariant optionName = ui->ParamCfgTableView->model()->data(removeTableIndex, Qt::DisplayRole);

//    QModelIndexList items = ui->ParamCfgTableView->model()->match(ui->ParamCfgTableView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
//                                                                  Qt::DisplayRole,
//                                                                  optionName, -1);
//    QModelIndexList definitionItems = ui->ParamCfgDefTreeView->model()->match(ui->ParamCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
//                                                                     Qt::DisplayRole,
//                                                                     optionName, 1);

//    QList<int> rows;
//    for(const QModelIndex & index : ui->ParamCfgTableView->selectionModel()->selectedRows()) {
//        rows.append( index.row() );
//    }
//    std::sort(rows.begin(), rows.end());
//    int prev = -1;
//    for(int i=rows.count()-1; i>=0; i--) {
//        int current = rows[i];
//        if (current != prev) {
//            ui->ParamCfgTableView->model()->removeRows( current, 1 );
//            prev = current;
//        }
//    }

//    ui->ParamCfgDefTreeView->clearSelection();
//    ui->ParamCfgTableView->setFocus();
//    initActions();
//    connect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
//            this, &ParamConfigEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
//}

bool ParamConfigEditor::isThereARow() const
{
    return (ui->paramCfgTableView->model()->rowCount() > 0);
}

bool ParamConfigEditor::isThereAnIndexSelection() const
{
    QModelIndexList selection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    return (selection.count() > 0);
}

bool ParamConfigEditor::isThereARowSelection() const
{
    QModelIndexList selection = ui->paramCfgTableView->selectionModel()->selectedRows();
    return (selection.count() > 0);
}

bool ParamConfigEditor::isEverySelectionARow() const
{
    const QModelIndexList selection = ui->paramCfgTableView->selectionModel()->selectedRows();
    const QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    return ((selection.count() > 0) && (indexSelection.count() % ui->paramCfgTableView->model()->columnCount() == 0));

}

void ParamConfigEditor::on_actionInsert_triggered()
{
    if (!ui->actionInsert->isEnabled())
        return;

    QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->paramCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    disconnect(mParameterTableModel, &QAbstractTableModel::dataChanged, mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem);
    int rowToBeInserted = -1;
    if (isThereAnIndexSelection()) {
        QList<int> rows;
        const auto indexes = ui->paramCfgTableView->selectionModel()->selectedIndexes();
        for(const QModelIndex idx : indexes) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        rowToBeInserted = rows.at(0);
        if (rowToBeInserted < ui->paramCfgTableView->model()->rowCount()) {
             QVariant header = ui->paramCfgTableView->model()->headerData(rowToBeInserted, Qt::Vertical, Qt::CheckStateRole);
             ui->paramCfgTableView->model()->insertRows(rowToBeInserted, 1, QModelIndex());
             ui->paramCfgTableView->model()->setHeaderData(rowToBeInserted+1, Qt::Vertical,
                                                           header,
                                                           Qt::CheckStateRole );
        } else {
            ui->paramCfgTableView->model()->insertRows(rowToBeInserted, 1, QModelIndex());
        }
    } else {
        ui->paramCfgTableView->model()->insertRows(ui->paramCfgTableView->model()->rowCount(), 1, QModelIndex());
        rowToBeInserted = mParameterTableModel->rowCount()-1;
    }

    const QModelIndex insertKeyIndex = ui->paramCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_KEY);
    const QModelIndex insertValueIndex = ui->paramCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    const QModelIndex insertNumberIndex = ui->paramCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_ENTRY_NUMBER);
    const QModelIndex minVersionIndex = ui->paramCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_MIN_VERSION);
    const QModelIndex maxVersionIndex = ui->paramCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_MIN_VERSION);

    ui->paramCfgTableView->model()->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                  Qt::CheckState(Qt::Checked),
                                                  Qt::CheckStateRole );

    ui->paramCfgTableView->model()->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
    ui->paramCfgTableView->model()->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
    ui->paramCfgTableView->model()->setData( minVersionIndex, "", Qt::EditRole);
    ui->paramCfgTableView->model()->setData( maxVersionIndex, "", Qt::EditRole);
    ui->paramCfgTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);
    ui->paramCfgTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

    connect(mParameterTableModel, &QAbstractTableModel::dataChanged,
            mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);

    emit modificationChanged(true);

    ui->paramCfgTableView->selectionModel()->select(insertKeyIndex, QItemSelectionModel::ClearAndSelect );

    const QModelIndex index = mParameterTableModel->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_KEY);
    ui->paramCfgTableView->edit( index );
    updateActionsState(index);
}

void ParamConfigEditor::on_actionDelete_triggered()
{
    if (!ui->actionDelete->isEnabled())
        return;

    QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->paramCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        const QItemSelection selection( ui->paramCfgTableView->selectionModel()->selection() );

        QList<int> rows;
        const auto indexes = ui->paramCfgTableView->selectionModel()->selectedRows();
        for(const QModelIndex & index : indexes) {
            rows.append( index.row() );
        }

        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            int current = rows[i];
            if (current != prev) {
                const QString text = mParameterTableModel->getParameterTableEntry(current);
                ui->paramCfgTableView->model()->removeRows( current, 1 );
                mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been deleted").arg(text), LogMsgType::Info);
                prev = current;
            }
        }
        emit modificationChanged(true);
    }
}

void ParamConfigEditor::on_actionMoveUp_triggered()
{
    if (!ui->actionMoveUp->isEnabled())
        return;
    QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->paramCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  ( !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    const QModelIndexList selection = ui->paramCfgTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() <= 0)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        const QModelIndex idx = idxSelection.at(i);
        mParameterTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                                         QModelIndex(), idx.row()-1);
    }

    emit modificationChanged(true);
    updateActionsState();
}

void ParamConfigEditor::on_actionMoveDown_triggered()
{
    if (!ui->actionMoveDown->isEnabled())
        return;
    QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->paramCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  ( !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    const QModelIndexList selection = ui->paramCfgTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() >= mParameterTableModel->rowCount()-1)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        const QModelIndex idx = idxSelection.at(i);
        mParameterTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                    QModelIndex(), idx.row()+2);
    }
    emit modificationChanged(true);
    updateActionsState( );
}

void ParamConfigEditor::on_actionSelect_Current_Row_triggered()
{
    if (!ui->actionSelect_Current_Row->isEnabled())
        return;
    QList<int> rowList;
    const auto indexes = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &idx : indexes) {
        if (!rowList.contains(idx.row())) {
            on_selectRow(idx.row());
            rowList << idx.row();
        }
    }
}

void ParamConfigEditor::on_actionSelectAll_triggered()
{
    if (!ui->actionSelectAll->isEnabled())
        return;

    selectAll();
}

void ParamConfigEditor::on_actionShowRecurrence_triggered()
{
    if (!ui->actionShowRecurrence->isEnabled())
        return;

    const QModelIndexList indexSelection = ui->paramCfgTableView->selectionModel()->selectedIndexes();
    if (indexSelection.size() <= 0) {
        showOptionDefinition();
        return;
    }

    QItemSelection selection = ui->paramCfgTableView->selectionModel()->selection();
    selection.select(ui->paramCfgTableView->model()->index(indexSelection.at(0).row(), 0),
                     ui->paramCfgTableView->model()->index(indexSelection.at(0).row(), ConfigParamTableModel::COLUMN_ENTRY_NUMBER));
    ui->paramCfgTableView->selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows );

    QList<int> rowList = getRecurrentOption(indexSelection.at(0));
    if (rowList.size() <= 0) {
        showOptionDefinition();
        return;
    }

    for(const int row : std::as_const(rowList)) {
        QItemSelection rowSelection = ui->paramCfgTableView->selectionModel()->selection();
        rowSelection.select(ui->paramCfgTableView->model()->index(row, 0),
                            ui->paramCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_ENTRY_NUMBER));
        ui->paramCfgTableView->selectionModel()->select(rowSelection, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    }

    showOptionDefinition();
}

void ParamConfigEditor::on_actionResize_Columns_To_Contents_triggered()
{
    if (!ui->actionResize_Columns_To_Contents->isEnabled())
        return;

    if (focusWidget()==ui->paramCfgTableView) {
        if (ui->paramCfgTableView->model()->rowCount()<=0)
            return;
        ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_KEY);
        ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_VALUE);
        ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_MIN_VERSION);
        ui->paramCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_MAX_VERSION);
    } else  if (focusWidget()==ui->paramCfgDefTreeView) {
        ui->paramCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->paramCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
        ui->paramCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
        ui->paramCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_RANGE);
        ui->paramCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_TYPE);
    }
}

void ParamConfigEditor::on_actionShow_Option_Definition_triggered()
{
    if (!ui->actionShow_Option_Definition->isEnabled())
        return;

    showOptionDefinition(true);
}

void ParamConfigEditor::on_actionAdd_This_Parameter_triggered()
{
    if (!ui->actionAdd_This_Parameter->isEnabled())
        return;

    const QModelIndexList selection = ui->paramCfgDefTreeView->selectionModel()->selectedIndexes(); // Rows();
    if (selection.size()>0) {
        addParameterFromDefinition(selection.at(0));
        const QModelIndex index = ui->paramCfgDefTreeView->selectionModel()->currentIndex();
        updateDefinitionActionsState(index);
    }
}

void ParamConfigEditor::on_actionRemove_This_Parameter_triggered()
{
    if (!ui->actionRemove_This_Parameter->isEnabled())
        return;

    findAndSelectionParameterFromDefinition();
    deleteOption();
}

} // namespace option
} // namespace studio
} // namespace gams
