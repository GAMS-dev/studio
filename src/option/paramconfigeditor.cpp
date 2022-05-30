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
#include "common.h"
#include "definitionitemdelegate.h"
#include "configoptiondefinitionmodel.h"
#include "optionsortfilterproxymodel.h"
#include "paramconfigeditor.h"
#include "theme.h"
#include "ui_paramconfigeditor.h"
#include "headerviewproxy.h"

#include <QScrollBar>
#include <QMessageBox>
#include <QMenu>
#include <QClipboard>

namespace gams {
namespace studio {
namespace option {

ParamConfigEditor::ParamConfigEditor(const QList<ConfigItem *> &initParams,  QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParamConfigEditor),
    mModified(false)
{
    ui->setupUi(this);
    mPrevFontHeight = fontMetrics().height();
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
    ui->ParamCfgCtrl->layout()->setMenuBar(mToolBar);

    setFocusPolicy(Qt::StrongFocus);
    mOptionTokenizer = new OptionTokenizer(GamsOptDefFile);

    QList<ParamConfigItem *> optionItem;
    for(ConfigItem* item: initParamItems) {
        optionItem.append( new ParamConfigItem(-1, item->key, item->value, item->minVersion, item->maxVersion) );
    }
    mParameterTableModel = new ConfigParamTableModel(optionItem, mOptionTokenizer, this);
    ui->ParamCfgTableView->setModel( mParameterTableModel );

    mOptionCompleter = new OptionCompleterDelegate(mOptionTokenizer, ui->ParamCfgTableView);
    ui->ParamCfgTableView->setItemDelegate( mOptionCompleter );
    connect(mOptionCompleter, &QStyledItemDelegate::commitData, this, &ParamConfigEditor::parameterItemCommitted);

    ui->ParamCfgTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::SelectedClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->ParamCfgTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->ParamCfgTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->ParamCfgTableView->setAutoScroll(true);
    ui->ParamCfgTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->ParamCfgTableView->setSortingEnabled(false);

    ui->ParamCfgTableView->setDragEnabled(true);
    ui->ParamCfgTableView->viewport()->setAcceptDrops(true);
    ui->ParamCfgTableView->setDropIndicatorShown(true);
    ui->ParamCfgTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->ParamCfgTableView->setDragDropOverwriteMode(true);
    ui->ParamCfgTableView->setDefaultDropAction(Qt::CopyAction);

    ui->ParamCfgTableView->setColumnHidden(ConfigParamTableModel::COLUMN_ENTRY_NUMBER, true);
    ui->ParamCfgTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->ParamCfgTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->ParamCfgTableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());

    ui->ParamCfgTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_PARAM_KEY, QHeaderView::Stretch);
    ui->ParamCfgTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_PARAM_VALUE, QHeaderView::Stretch);

    ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_KEY);
    ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_VALUE);
    ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_MIN_VERSION);
    ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_MAX_VERSION);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    ConfigOptionDefinitionModel* optdefmodel =  new ConfigOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->ParamCfgDefSearch, &QLineEdit::textChanged,
            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->ParamCfgDefTreeView->header()->setStyle(HeaderViewProxy::instance());
    ui->ParamCfgDefTreeView->setModel( proxymodel );
    ui->ParamCfgDefTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ParamCfgDefTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->ParamCfgDefTreeView->setDragEnabled(true);
    ui->ParamCfgDefTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->ParamCfgDefTreeView->setItemDelegate( new DefinitionItemDelegate(ui->ParamCfgDefTreeView) );
    ui->ParamCfgDefTreeView->setItemsExpandable(true);
    ui->ParamCfgDefTreeView->setSortingEnabled(true);
    ui->ParamCfgDefTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
    ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
    ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
    ui->ParamCfgDefTreeView->setExpandsOnDoubleClick(false);
    ui->ParamCfgDefTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true);
    ui->ParamCfgDefTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->ParamCfgTableView->setTabKeyNavigation(true);

    connect(ui->ParamCfgTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &ParamConfigEditor::on_selectRow, Qt::UniqueConnection);
    connect(ui->ParamCfgTableView, &QTableView::customContextMenuRequested,this, &ParamConfigEditor::showParameterContextMenu, Qt::UniqueConnection);
    connect(ui->ParamCfgTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &ParamConfigEditor::currentTableSelectionChanged);
    connect(mParameterTableModel, &ConfigParamTableModel::newTableRowDropped, this, &ParamConfigEditor::on_newTableRowDropped, Qt::UniqueConnection);

    connect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParamConfigEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
    connect(ui->ParamCfgDefTreeView, &QTreeView::customContextMenuRequested, this, &ParamConfigEditor::showDefinitionContextMenu, Qt::UniqueConnection);
    connect(ui->ParamCfgDefTreeView, &QAbstractItemView::doubleClicked, this, &ParamConfigEditor::addParameterFromDefinition, Qt::UniqueConnection);

    connect(mParameterTableModel, &QAbstractTableModel::dataChanged, this, &ParamConfigEditor::on_dataItemChanged, Qt::UniqueConnection);
    connect(mParameterTableModel, &QAbstractTableModel::dataChanged, mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigParamTableModel::configParamModelChanged, optdefmodel, &ConfigOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigParamTableModel::configParamItemChanged, optdefmodel, &ConfigOptionDefinitionModel::modifyOptionDefinitionItem, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigParamTableModel::configParamItemRemoved, mParameterTableModel, &ConfigParamTableModel::on_removeConfigParamItem, Qt::UniqueConnection);

    connect(this, &ParamConfigEditor::modificationChanged, this, &ParamConfigEditor::setModified, Qt::UniqueConnection);
    emit mParameterTableModel->configParamModelChanged(optionItem);
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
    return (focusWidget==ui->ParamCfgTableView || focusWidget==ui->ParamCfgDefTreeView);
}

QString ParamConfigEditor::getSelectedParameterName(QWidget *widget) const
{
    if (widget == ui->ParamCfgTableView) {
        QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QVariant headerData = ui->ParamCfgTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::PartiallyChecked) {
                return "";
            }
            QVariant data = ui->ParamCfgTableView->model()->data( index.sibling(index.row(),0) );
            if (mOptionTokenizer->getOption()->isValid(data.toString()))
               return data.toString();
            else if (mOptionTokenizer->getOption()->isASynonym(data.toString()))
                    return mOptionTokenizer->getOption()->getNameFromSynonym(data.toString());
            else
               return "";
        }
    } else if (widget == ui->ParamCfgDefTreeView) {
        QModelIndexList selection = ui->ParamCfgDefTreeView->selectionModel()->selectedRows();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QModelIndex  parentIndex =  ui->ParamCfgDefTreeView->model()->parent(index);
            if (parentIndex.row() >= 0) {
                return ui->ParamCfgDefTreeView->model()->data( parentIndex.sibling(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            } else {
                return ui->ParamCfgDefTreeView->model()->data( index.sibling(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            }
        }
    }
    return "";

}

void ParamConfigEditor::currentTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (ui->ParamCfgTableView->selectionModel()->selectedRows().isEmpty()) {
        updateActionsState(current);
    } else {
        if (ui->ParamCfgTableView->selectionModel()->isRowSelected(current.row(), current.parent()) )
            updateActionsState();
        else
            updateActionsState(current);
    }
}

void ParamConfigEditor::updateActionsState(const QModelIndex &index)
{
    ui->actionInsert->setEnabled( true );
    ui->actionDelete->setEnabled( false );
    ui->actionMoveUp->setEnabled( false );
    ui->actionMoveDown->setEnabled( false );
    ui->actionSelect_Current_Row->setEnabled( isThereAnIndexSelection() );
    ui->actionSelectAll->setEnabled( isThereARow() );
    ui->actionShow_Option_Definition->setEnabled( index.row() < mParameterTableModel->rowCount() );
    ui->actionResize_Columns_To_Contents->setEnabled( index.row() < mParameterTableModel->rowCount() );
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
    QModelIndexList idxSelection = ( ui->ParamCfgTableView->selectionModel()->selectedRows().isEmpty()
                                         ? ui->ParamCfgTableView->selectionModel()->selectedIndexes()
                                         : ui->ParamCfgTableView->selectionModel()->selectedRows() );

    if (idxSelection.isEmpty())
        return;
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });

    ui->actionInsert->setEnabled( isThereARow() );
    ui->actionDelete->setEnabled( idxSelection.first().row() < mParameterTableModel->rowCount() );

    ui->actionMoveUp->setEnabled( idxSelection.first().row() > 0 );
    ui->actionMoveDown->setEnabled( idxSelection.last().row() < mParameterTableModel->rowCount()-1 );

    ui->actionSelect_Current_Row->setEnabled( isThereAnIndexSelection() );
    ui->actionSelectAll->setEnabled( isThereARow( ));

    ui->actionShow_Option_Definition->setEnabled( idxSelection.first().row() < mParameterTableModel->rowCount() );
    ui->actionResize_Columns_To_Contents->setEnabled( idxSelection.first().row() < mParameterTableModel->rowCount() );
    ui->actionShowRecurrence->setEnabled( idxSelection.first().row() < mParameterTableModel->rowCount()
                                          && getRecurrentOption(idxSelection.first()).size() >0 );
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
    QModelIndex parentIndex =  ui->ParamCfgDefTreeView->model()->parent(index);
    QVariant data = (parentIndex.row() < 0) ? ui->ParamCfgDefTreeView->model()->data(index, Qt::CheckStateRole)
                                            : ui->ParamCfgDefTreeView->model()->data(parentIndex, Qt::CheckStateRole);
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
    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
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

    menu.exec(ui->ParamCfgTableView->viewport()->mapToGlobal(pos));
}

void ParamConfigEditor::showDefinitionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->ParamCfgDefTreeView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    QMenu menu(this);
    menu.addAction(ui->actionAdd_This_Parameter);
    menu.addAction(ui->actionRemove_This_Parameter);
    menu.addSeparator();
    menu.addAction(ui->actionResize_Columns_To_Contents);
    menu.exec(ui->ParamCfgDefTreeView->viewport()->mapToGlobal(pos));
}

void ParamConfigEditor::parameterItemCommitted(QWidget *editor)
{
    Q_UNUSED(editor)
    if (mOptionCompleter->currentEditedIndex().isValid()) {
        ui->ParamCfgTableView->resizeColumnToContents( mOptionCompleter->currentEditedIndex().column() );
        ui->ParamCfgTableView->selectionModel()->select( mOptionCompleter->currentEditedIndex(), QItemSelectionModel::ClearAndSelect );
        ui->ParamCfgTableView->setCurrentIndex( mOptionCompleter->currentEditedIndex() );
        ui->ParamCfgTableView->setFocus();
    }
}

void ParamConfigEditor::on_selectRow(int logicalIndex)
{
    if (ui->ParamCfgTableView->model()->rowCount() <= 0)
        return;

    QItemSelectionModel *selectionModel = ui->ParamCfgTableView->selectionModel();
    QModelIndex topLeft = ui->ParamCfgTableView->model()->index(logicalIndex, ConfigParamTableModel::COLUMN_PARAM_KEY, QModelIndex());
    QModelIndex  bottomRight = ui->ParamCfgTableView->model()->index(logicalIndex, ConfigParamTableModel::COLUMN_ENTRY_NUMBER, QModelIndex());
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
    ui->ParamCfgTableView->setFocus();
    ui->ParamCfgTableView->selectAll();

    updateActionsState( );
}

void ParamConfigEditor::findAndSelectionParameterFromDefinition()
{
    if (ui->ParamCfgTableView->model()->rowCount() <= 0)
        return;

    QModelIndex index = ui->ParamCfgDefTreeView->selectionModel()->currentIndex();

    updateDefinitionActionsState(index);

    QModelIndex parentIndex =  ui->ParamCfgDefTreeView->model()->parent(index);

    QModelIndex idx = (parentIndex.row()<0) ? ui->ParamCfgDefTreeView->model()->index( index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER )
                                            : ui->ParamCfgDefTreeView->model()->index( parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER );
    QVariant data = ui->ParamCfgDefTreeView->model()->data( idx, Qt::DisplayRole );
    QModelIndexList indices = ui->ParamCfgTableView->model()->match(ui->ParamCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                       Qt::DisplayRole,
                                                                       data, -1, Qt::MatchExactly|Qt::MatchRecursive);
    ui->ParamCfgTableView->clearSelection();
    ui->ParamCfgTableView->clearFocus();
    QItemSelection selection;
    for(QModelIndex i :indices) {
        QModelIndex valueIndex = ui->ParamCfgTableView->model()->index(i.row(), ConfigParamTableModel::COLUMN_PARAM_VALUE);
        QString value =  ui->ParamCfgTableView->model()->data( valueIndex, Qt::DisplayRole).toString();
        bool selected = false;
        if (parentIndex.row() < 0) {
            selected = true;
        } else {
            QModelIndex enumIndex = ui->ParamCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex);
            QString enumValue = ui->ParamCfgDefTreeView->model()->data( enumIndex, Qt::DisplayRole).toString();
            if (QString::compare(value, enumValue, Qt::CaseInsensitive)==0)
                selected = true;
        }
        if (selected) {
           QModelIndex leftIndex  = ui->ParamCfgTableView->model()->index(i.row(), 0);
           QModelIndex rightIndex = ui->ParamCfgTableView->model()->index(i.row(), ui->ParamCfgTableView->model()->columnCount() -1);

           QItemSelection rowSelection(leftIndex, rightIndex);
           selection.merge(rowSelection, QItemSelectionModel::Select);
        }
        ui->actionAdd_This_Parameter->setEnabled(!selected);
        ui->actionRemove_This_Parameter->setEnabled(selected);
    }

    ui->ParamCfgTableView->selectionModel()->select(selection, QItemSelectionModel::Select);
    ui->ParamCfgDefTreeView->setFocus();

}

void ParamConfigEditor::selectAnOption()
{
    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    if (indexSelection.empty())
        indexSelection <<  ui->ParamCfgTableView->selectionModel()->currentIndex();

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
    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        QItemSelection selection( ui->ParamCfgTableView->selectionModel()->selection() );

        QList<int> rows;
        for(const QModelIndex & index : ui->ParamCfgTableView->selectionModel()->selectedRows()) {
            rows.append( index.row() );
        }

        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            int current = rows[i];
            if (current != prev) {
                QString text = mParameterTableModel->getParameterTableEntry(current);
                ui->ParamCfgTableView->model()->removeRows( current, 1 );
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
    if (ui->ParamCfgTableView->hasFocus() && ui->ParamCfgTableView->selectionModel()->hasSelection())
        ui->ParamCfgTableView->selectionModel()->clearSelection();
    else if (ui->ParamCfgDefTreeView->hasFocus() && ui->ParamCfgDefTreeView->selectionModel()->hasSelection())
             ui->ParamCfgDefTreeView->selectionModel()->clearSelection();
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

QList<ConfigItem *> ParamConfigEditor::parameterConfigItems()
{
    QList<ConfigItem *> itemList;
    for(ParamConfigItem* item : mParameterTableModel->parameterConfigItems()) {
        itemList.append( new ConfigItem(item->key, item->value, item->minVersion, item->maxVersion) );
    }
    return itemList;
}

bool ParamConfigEditor::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        ui->ParamCfgTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
//        ui->ParamCfgDefTreeView->header()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
        qreal scale = qreal(fontMetrics().height()) / qreal(mPrevFontHeight);
        for (int i = 0; i < ui->ParamCfgTableView->horizontalHeader()->count(); ++i) {
            ui->ParamCfgTableView->horizontalHeader()->resizeSection(i, qRound(ui->ParamCfgTableView->horizontalHeader()->sectionSize(i) * scale));
        }
        for (int i = 0; i < ui->ParamCfgDefTreeView->header()->count(); ++i) {
            ui->ParamCfgDefTreeView->header()->resizeSection(i, qRound(ui->ParamCfgDefTreeView->header()->sectionSize(i) * scale));
        }
        mPrevFontHeight = fontMetrics().height();
    }
    return QWidget::event(event);
}

void ParamConfigEditor::addParameterFromDefinition(const QModelIndex &index)
{
    emit modificationChanged(true);

    QModelIndex parentIndex =  ui->ParamCfgDefTreeView->model()->parent(index);
    QModelIndex optionNameIndex = (parentIndex.row()<0) ? ui->ParamCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) :
                                                          ui->ParamCfgDefTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    QModelIndex defValueIndex = (parentIndex.row()<0) ? ui->ParamCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) :
                                                        ui->ParamCfgDefTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex :
                                                             ui->ParamCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex) ;

    disconnect( mParameterTableModel, &QAbstractTableModel::dataChanged,  mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem);

    bool replaceExistingEntry = false;
    QString optionNameData = ui->ParamCfgDefTreeView->model()->data(optionNameIndex).toString();
    QModelIndex optionIdIndex = (parentIndex.row()<0) ? ui->ParamCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) :
                                                        ui->ParamCfgDefTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) ;
    QVariant optionIdData = ui->ParamCfgDefTreeView->model()->data(optionIdIndex);

    int rowToBeAdded = ui->ParamCfgTableView->model()->rowCount();
//    StudioSettings* settings = SettingsLocator::settings();
//    if (settings && settings->overridExistingOption()) {
        QModelIndexList indices = ui->ParamCfgTableView->model()->match(ui->ParamCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                            Qt::DisplayRole,
                                                                            optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
        ui->ParamCfgTableView->clearSelection();
        QItemSelection selection;
        for(QModelIndex &idx: indices) {
            QModelIndex leftIndex  = ui->ParamCfgTableView->model()->index(idx.row(), ConfigParamTableModel::COLUMN_PARAM_KEY);
            QModelIndex rightIndex = ui->ParamCfgTableView->model()->index(idx.row(), ConfigParamTableModel::COLUMN_ENTRY_NUMBER);
            QItemSelection rowSelection(leftIndex, rightIndex);
            selection.merge(rowSelection, QItemSelectionModel::Select);
        }
        ui->ParamCfgTableView->selectionModel()->select(selection, QItemSelectionModel::Select);

        bool singleEntryExisted = (indices.size()==1);
        bool multipleEntryExisted = (indices.size()>1);
        if (singleEntryExisted ) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Parameter Entry exists");
            msgBox.setText("Parameter '" + optionNameData+ "' already exists.");
            msgBox.setInformativeText("How do you want to proceed?");
            msgBox.setDetailedText(QString("Entry:  '%1'\nDescription:  %2 %3").arg(getParameterTableEntry(indices.at(0).row()))
                    .arg("When a Gams config file contains multiple entries of the same parameters, only the value of the last entry will be utilized by Gams.")
                    .arg("The value of all other entries except the last entry will be ignored."));
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Replace existing entry", QMessageBox::ActionRole);
            msgBox.addButton("Add new entry", QMessageBox::ActionRole);

            switch(msgBox.exec()) {
            case 0: // replace
                replaceExistingEntry = true;
                indices = ui->ParamCfgTableView->model()->match(ui->ParamCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                    Qt::DisplayRole,
                                                                    optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
                rowToBeAdded = (indices.size()>0) ? indices.at(0).row() : 0;
                break;
            case 1: // add
                break;
            case QMessageBox::Abort:
                return;
            }
        } else if (multipleEntryExisted) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Multiple Parameter Entries exist");
            msgBox.setText(QString("%1 entries of Parameter '%2' already exist.").arg(indices.size()).arg(optionNameData));
            msgBox.setInformativeText("How do you want to proceed?");
            QString entryDetailedText = QString("Entries:\n");
            int i = 0;
            for (QModelIndex &idx : indices)
                entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getParameterTableEntry(idx.row())));
            msgBox.setDetailedText(QString("%1Description:  %2 %3").arg(entryDetailedText)
                    .arg("When a Gams config file contains multiple entries of the same parameters, only the value of the last entry will be utilized by Gams.")
                    .arg("The value of all other entries except the last entry will be ignored."));
            msgBox.setText("Multiple entries of Parameter '" + optionNameData + "' already exist.");
            msgBox.setInformativeText("How do you want to proceed?");
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Replace first entry and delete other entries", QMessageBox::ActionRole);
            msgBox.addButton("Add new entry", QMessageBox::ActionRole);

            switch(msgBox.exec()) {
            case 0: // delete and replace
                disconnect( mParameterTableModel, &ConfigParamTableModel::configParamItemRemoved, mParameterTableModel, &ConfigParamTableModel::on_removeConfigParamItem);
                ui->ParamCfgTableView->selectionModel()->clearSelection();
                for(int i=1; i<indices.size(); i++) {
                    ui->ParamCfgTableView->selectionModel()->select( indices.at(i), QItemSelectionModel::Select|QItemSelectionModel::Rows );
                }
                deleteOption();
                connect( mParameterTableModel, &ConfigParamTableModel::configParamItemRemoved,
                         mParameterTableModel, &ConfigParamTableModel::on_removeConfigParamItem, Qt::UniqueConnection);
                replaceExistingEntry = true;
                indices = ui->ParamCfgTableView->model()->match(ui->ParamCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                    Qt::DisplayRole,
                                                                    optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
                rowToBeAdded = (indices.size()>0) ? indices.at(0).row() : 0;
                break;
            case 1: // add
                break;
            case QMessageBox::Abort:
                return;
            }

        } // else entry not exist
//    }

    ui->ParamCfgTableView->selectionModel()->clearSelection();
//    QString synonymData = ui->ParamCfgDefTreeView->model()->data(synonymIndex).toString();
    QString selectedValueData = ui->ParamCfgDefTreeView->model()->data(selectedValueIndex).toString();
    mOptionTokenizer->getOption()->setModified(optionNameData, true);
    ui->ParamCfgDefTreeView->model()->setData(optionNameIndex, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);

    // insert option row
    if (rowToBeAdded == ui->ParamCfgTableView->model()->rowCount()) {
        ui->ParamCfgTableView->model()->insertRows(rowToBeAdded, 1, QModelIndex());
    }
    QModelIndex insertKeyIndex = ui->ParamCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_PARAM_KEY);
    QModelIndex insertValueIndex = ui->ParamCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    QModelIndex minVersionIndex = ui->ParamCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_MIN_VERSION);
    QModelIndex maxVersionIndex = ui->ParamCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_MIN_VERSION);
    ui->ParamCfgTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->ParamCfgTableView->model()->setData( insertValueIndex, selectedValueData, Qt::EditRole);
    ui->ParamCfgTableView->model()->setData( minVersionIndex, "", Qt::EditRole);
    ui->ParamCfgTableView->model()->setData( maxVersionIndex, "", Qt::EditRole);

    QModelIndex insertNumberIndex = ui->ParamCfgTableView->model()->index(rowToBeAdded, ConfigParamTableModel::COLUMN_ENTRY_NUMBER);
    int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;
    ui->ParamCfgTableView->model()->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    if (selectedValueData.isEmpty())
        ui->ParamCfgTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Checked), Qt::CheckStateRole );
    else
       ui->ParamCfgTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
    ui->ParamCfgTableView->selectRow(rowToBeAdded);
    selectAnOption();

    QString text =  mParameterTableModel->getParameterTableEntry(insertNumberIndex.row());
    if (replaceExistingEntry)
        mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been replaced").arg(text), LogMsgType::Info);
    else
        mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been added").arg(text), LogMsgType::Info);

    int lastColumn = ui->ParamCfgTableView->model()->columnCount()-1;
    int lastRow = rowToBeAdded;
    int firstRow = lastRow;
    if (firstRow<0)
        firstRow = 0;
     mParameterTableModel->on_updateConfigParamItem( ui->ParamCfgTableView->model()->index(firstRow, lastColumn),
                                                     ui->ParamCfgTableView->model()->index(lastRow, lastColumn),
                                                     {Qt::EditRole});

     ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_KEY);
     ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_VALUE);

    connect( mParameterTableModel, &QAbstractTableModel::dataChanged,  mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);

    showOptionDefinition(true);

    if (parentIndex.row()<0) {
        if (mOptionTokenizer->getOption()->getOptionSubType(optionNameData) != optsubNoValue)
            ui->ParamCfgTableView->edit(insertValueIndex);
    }

}

void ParamConfigEditor::showOptionDefinition(bool selectRow)
{
    if (ui->ParamCfgTableView->model()->rowCount() <= 0)
        return;

    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    if (indexSelection.count() <= 0)
        return;

    disconnect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
               this, &ParamConfigEditor::findAndSelectionParameterFromDefinition);

    ui->ParamCfgDefSearch->clear();

    QModelIndexList selection;
    if (selectRow) {
       selectAnOption();
       selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
    } else {
         selection = indexSelection;
         ui->ParamCfgTableView->selectionModel()->setCurrentIndex ( selection.first(), QItemSelectionModel::Select );
    }

    QModelIndexList selectIndices;
    for (int i=0; i<selection.count(); i++) {
         QModelIndex index = selection.at(i);
         if (Qt::CheckState(ui->ParamCfgTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
                continue;

         QString value = ui->ParamCfgTableView->model()->data( index.sibling(index.row(), ConfigParamTableModel::COLUMN_PARAM_VALUE), Qt::DisplayRole).toString();
         QVariant optionId = ui->ParamCfgTableView->model()->data( index.sibling(index.row(), ConfigParamTableModel::COLUMN_ENTRY_NUMBER), Qt::DisplayRole);
         QModelIndexList indices = ui->ParamCfgDefTreeView->model()->match(ui->ParamCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                                                               Qt::DisplayRole,
                                                                               optionId, 1, Qt::MatchExactly|Qt::MatchRecursive);
         for(QModelIndex idx : indices) {
             QModelIndex  parentIndex =  ui->ParamCfgDefTreeView->model()->parent(idx);
             QModelIndex optionIdx = ui->ParamCfgDefTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);

             if (parentIndex.row() < 0) {
                if (ui->ParamCfgDefTreeView->model()->hasChildren(optionIdx) && !ui->ParamCfgDefTreeView->isExpanded(optionIdx))
                    ui->ParamCfgDefTreeView->expand(optionIdx);
            }
            bool found = false;
            for(int r=0; r <ui->ParamCfgDefTreeView->model()->rowCount(optionIdx); ++r) {
                QModelIndex i = ui->ParamCfgDefTreeView->model()->index(r, OptionDefinitionModel::COLUMN_OPTION_NAME, optionIdx);
                QString enumValue = ui->ParamCfgDefTreeView->model()->data(i, Qt::DisplayRole).toString();
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
    ui->ParamCfgDefTreeView->selectionModel()->clearSelection();
    for(QModelIndex idx : selectIndices) {
        QItemSelection selection = ui->ParamCfgDefTreeView->selectionModel()->selection();
        QModelIndex  parentIdx =  ui->ParamCfgDefTreeView->model()->parent(idx);
        if (parentIdx.row() < 0) {
            selection.select(ui->ParamCfgDefTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME),
                             ui->ParamCfgDefTreeView->model()->index(idx.row(), ui->ParamCfgDefTreeView->model()->columnCount()-1));
        } else  {
            selection.select(ui->ParamCfgDefTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIdx),
                             ui->ParamCfgDefTreeView->model()->index(idx.row(), ui->ParamCfgDefTreeView->model()->columnCount()-1, parentIdx));
        }
        ui->ParamCfgDefTreeView->selectionModel()->select(selection, QItemSelectionModel::Select);
    }
    if (selectIndices.size() > 0) {
        QModelIndex parentIndex = ui->ParamCfgDefTreeView->model()->parent(selectIndices.first());
        QModelIndex scrollToIndex = (parentIndex.row() < 0  ? ui->ParamCfgDefTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                            : ui->ParamCfgDefTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex));
        ui->ParamCfgDefTreeView->scrollTo(scrollToIndex, QAbstractItemView::EnsureVisible);
        if (parentIndex.row() >= 0) {
            ui->ParamCfgDefTreeView->scrollTo(parentIndex, QAbstractItemView::EnsureVisible);
            const QRect r = ui->ParamCfgDefTreeView->visualRect(parentIndex);
            ui->ParamCfgDefTreeView->horizontalScrollBar()->setValue(r.x());
        } else {
            const QRect r = ui->ParamCfgDefTreeView->visualRect(scrollToIndex);
            ui->ParamCfgDefTreeView->horizontalScrollBar()->setValue(r.x());
        }
    }

    connect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParamConfigEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParamConfigEditor::copyDefinitionToClipboard(int column)
{
    if (ui->ParamCfgDefTreeView->selectionModel()->selectedRows().count() <= 0)
        return;

    QModelIndex index = ui->ParamCfgDefTreeView->selectionModel()->selectedRows().at(0);
    if (!index.isValid())
        return;

    QString text = "";
    QModelIndex parentIndex = ui->ParamCfgDefTreeView->model()->parent(index);
    if (column == -1) { // copy all
        QStringList strList;
        if (parentIndex.isValid()) {
            QModelIndex idx = ui->ParamCfgDefTreeView->model()->index(index.row(), ConfigOptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex);
            strList << ui->ParamCfgDefTreeView->model()->data(idx, Qt::DisplayRole).toString();
            idx = ui->ParamCfgDefTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DESCIPTION, parentIndex);
            strList << ui->ParamCfgDefTreeView->model()->data(idx, Qt::DisplayRole).toString();
            text = strList.join(", ");
        } else {
           for (int j=0; j<ui->ParamCfgDefTreeView->model()->columnCount(); j++) {
               if (j==ConfigOptionDefinitionModel::COLUMN_ENTRY_NUMBER)
                  continue;
               QModelIndex columnindex = ui->ParamCfgDefTreeView->model()->index(index.row(), j);
               strList << ui->ParamCfgDefTreeView->model()->data(columnindex, Qt::DisplayRole).toString();
           }
           text = strList.join(", ");
        }
    } else {
        if (parentIndex.isValid()) {
            QModelIndex idx = ui->ParamCfgDefTreeView->model()->index(index.row(), column, parentIndex);
            text = ui->ParamCfgDefTreeView->model()->data( idx, Qt::DisplayRole ).toString();
        } else {
            text = ui->ParamCfgDefTreeView->model()->data( ui->ParamCfgDefTreeView->model()->index(index.row(), column), Qt::DisplayRole ).toString();
        }
    }
    QClipboard* clip = QApplication::clipboard();
    clip->setText( text );
}

void ParamConfigEditor::on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    emit modificationChanged(true);

    QModelIndexList toDefinitionItems = ui->ParamCfgDefTreeView->model()->match(ui->ParamCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     ui->ParamCfgTableView->model()->data( topLeft, Qt::DisplayRole), 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->ParamCfgDefTreeView->model()->match(ui->ParamCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                         Qt::DisplayRole,
                                                                         ui->ParamCfgTableView->model()->data( topLeft, Qt::DisplayRole), 1);
    }

    for(QModelIndex item : toDefinitionItems) {
        ui->ParamCfgDefTreeView->selectionModel()->select(
                    QItemSelection (
                        ui->ParamCfgDefTreeView->model ()->index (item.row() , 0),
                        ui->ParamCfgDefTreeView->model ()->index (item.row(), ui->ParamCfgDefTreeView->model ()->columnCount () - 1)),
                    QItemSelectionModel::Select);
        ui->ParamCfgDefTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
    ui->ParamCfgTableView->selectionModel()->select(topLeft, QItemSelectionModel::Select);

    updateActionsState(topLeft);
}

void ParamConfigEditor::on_newTableRowDropped(const QModelIndex &index)
{
    disconnect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParamConfigEditor::findAndSelectionParameterFromDefinition);
    ui->ParamCfgTableView->selectRow(index.row());

    QString optionName = ui->ParamCfgTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->ParamCfgDefTreeView->model()->match(ui->ParamCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(QModelIndex item : definitionItems) {
        ui->ParamCfgDefTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    if (mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumStr &&
        mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumInt &&
        mOptionTokenizer->getOption()->getOptionSubType(optionName) != optsubNoValue)
        ui->ParamCfgTableView->edit( mParameterTableModel->index(index.row(), ConfigParamTableModel::COLUMN_PARAM_VALUE));

    showOptionDefinition(true);
    connect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParamConfigEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

QList<int> ParamConfigEditor::getRecurrentOption(const QModelIndex &index)
{
    QList<int> optionList;

//    if (!isInFocus(focusWidget()))
//        return optionList;

    QVariant data = ui->ParamCfgTableView->model()->headerData(index.row(), Qt::Vertical,  Qt::CheckStateRole);
    if (Qt::CheckState(data.toUInt())==Qt::PartiallyChecked)
        return optionList;

    QString optionId = ui->ParamCfgTableView->model()->data( index.sibling(index.row(), ConfigParamTableModel::COLUMN_ENTRY_NUMBER), Qt::DisplayRole).toString();
    QModelIndexList indices = ui->ParamCfgTableView->model()->match(ui->ParamCfgTableView->model()->index(0, ConfigParamTableModel::COLUMN_ENTRY_NUMBER),
                                                                        Qt::DisplayRole,
                                                                        optionId, -1);
    for(QModelIndex idx : indices) {
        if (idx.row() == index.row())
            continue;
        else
            optionList << idx.row();
    }
    return optionList;
}

QString ParamConfigEditor::getParameterTableEntry(int row)
{
    QModelIndex keyIndex = ui->ParamCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_PARAM_KEY);
    QVariant optionKey = ui->ParamCfgTableView->model()->data(keyIndex, Qt::DisplayRole);
    QModelIndex valueIndex = ui->ParamCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    QVariant optionValue = ui->ParamCfgTableView->model()->data(valueIndex, Qt::DisplayRole);
    QModelIndex minVersionIndex = ui->ParamCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    QVariant minVersionValue = ui->ParamCfgTableView->model()->data(minVersionIndex, Qt::DisplayRole);
    QModelIndex maxVersionIndex = ui->ParamCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    QVariant maxVersionValue = ui->ParamCfgTableView->model()->data(maxVersionIndex, Qt::DisplayRole);
    return QString("%1%2%3 %4 %5").arg(optionKey.toString()).arg(mOptionTokenizer->getOption()->getDefaultSeparator())
                                                            .arg(optionValue.toString())
                                                            .arg(minVersionValue.toString())
                                                            .arg(maxVersionValue.toString());
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
    return (ui->ParamCfgTableView->model()->rowCount() > 0);
}

bool ParamConfigEditor::isThereAnIndexSelection() const
{
    QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    return (selection.count() > 0);
}

bool ParamConfigEditor::isThereARowSelection() const
{
    QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
    return (selection.count() > 0);
}

bool ParamConfigEditor::isEverySelectionARow() const
{
    QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    return ((selection.count() > 0) && (indexSelection.count() % ui->ParamCfgTableView->model()->columnCount() == 0));

}

void ParamConfigEditor::on_actionInsert_triggered()
{
    if (!ui->actionInsert->isEnabled())
        return;

    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    disconnect(mParameterTableModel, &QAbstractTableModel::dataChanged, mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem);
    int rowToBeInserted = -1;
    if (isThereARowSelection()) {
        QList<int> rows;
        for(QModelIndex idx : ui->ParamCfgTableView->selectionModel()->selectedRows()) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        rowToBeInserted = rows.at(0);
        ui->ParamCfgTableView->model()->insertRows(rowToBeInserted, 1, QModelIndex());
    } else {
        ui->ParamCfgTableView->model()->insertRows(ui->ParamCfgTableView->model()->rowCount(), 1, QModelIndex());
        rowToBeInserted = mParameterTableModel->rowCount()-1;
    }
    QModelIndex insertKeyIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_KEY);
    QModelIndex insertValueIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_VALUE);
    QModelIndex insertNumberIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_ENTRY_NUMBER);
    QModelIndex minVersionIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_MIN_VERSION);
    QModelIndex maxVersionIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_MIN_VERSION);

    ui->ParamCfgTableView->model()->setHeaderData(ui->ParamCfgTableView->model()->rowCount()-1, Qt::Vertical,
                                                      Qt::CheckState(Qt::Checked),
                                                      Qt::CheckStateRole );

    ui->ParamCfgTableView->model()->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
    ui->ParamCfgTableView->model()->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
    ui->ParamCfgTableView->model()->setData( minVersionIndex, "", Qt::EditRole);
    ui->ParamCfgTableView->model()->setData( maxVersionIndex, "", Qt::EditRole);
    ui->ParamCfgTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);
    ui->ParamCfgTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

    connect(mParameterTableModel, &QAbstractTableModel::dataChanged,
            mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);

    emit modificationChanged(true);

    ui->ParamCfgTableView->clearSelection();
    ui->ParamCfgTableView->selectRow(rowToBeInserted);

    QModelIndex index = mParameterTableModel->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_KEY);
    ui->ParamCfgTableView->edit( index );
    updateActionsState(index);
}

void ParamConfigEditor::on_actionDelete_triggered()
{
    if (!ui->actionDelete->isEnabled())
        return;

    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        QItemSelection selection( ui->ParamCfgTableView->selectionModel()->selection() );

        QList<int> rows;
        for(const QModelIndex & index : ui->ParamCfgTableView->selectionModel()->selectedRows()) {
            rows.append( index.row() );
        }

        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            int current = rows[i];
            if (current != prev) {
                QString text = mParameterTableModel->getParameterTableEntry(current);
                ui->ParamCfgTableView->model()->removeRows( current, 1 );
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
    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  ( !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() <= 0)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
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
    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  ( !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() >= mParameterTableModel->rowCount()-1)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
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
    for(QModelIndex idx : ui->ParamCfgTableView->selectionModel()->selectedIndexes()) {
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

    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    if (indexSelection.size() <= 0) {
        showOptionDefinition();
        return;
    }

    QItemSelection selection = ui->ParamCfgTableView->selectionModel()->selection();
    selection.select(ui->ParamCfgTableView->model()->index(indexSelection.at(0).row(), 0),
                     ui->ParamCfgTableView->model()->index(indexSelection.at(0).row(), ConfigParamTableModel::COLUMN_ENTRY_NUMBER));
    ui->ParamCfgTableView->selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows );

    QList<int> rowList = getRecurrentOption(indexSelection.at(0));
    if (rowList.size() <= 0) {
        showOptionDefinition();
        return;
    }

    for(int row : rowList) {
        QItemSelection rowSelection = ui->ParamCfgTableView->selectionModel()->selection();
        rowSelection.select(ui->ParamCfgTableView->model()->index(row, 0),
                            ui->ParamCfgTableView->model()->index(row, ConfigParamTableModel::COLUMN_ENTRY_NUMBER));
        ui->ParamCfgTableView->selectionModel()->select(rowSelection, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    }

    showOptionDefinition();
}

void ParamConfigEditor::on_actionResize_Columns_To_Contents_triggered()
{
    if (!ui->actionResize_Columns_To_Contents->isEnabled())
        return;

    if (focusWidget()==ui->ParamCfgTableView) {
        if (ui->ParamCfgTableView->model()->rowCount()<=0)
            return;
        ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_KEY);
        ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_PARAM_VALUE);
        ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_MIN_VERSION);
        ui->ParamCfgTableView->resizeColumnToContents(ConfigParamTableModel::COLUMN_MAX_VERSION);
    } else  if (focusWidget()==ui->ParamCfgDefTreeView) {
        ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
        ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
        ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_RANGE);
        ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_TYPE);
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

    QModelIndexList selection = ui->ParamCfgDefTreeView->selectionModel()->selectedIndexes(); // Rows();
    if (selection.size()>0) {
        addParameterFromDefinition(selection.at(0));
        QModelIndex index = ui->ParamCfgDefTreeView->selectionModel()->currentIndex();
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


}
}
}
