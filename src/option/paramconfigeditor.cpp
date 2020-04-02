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
#include "common.h"
#include "definitionitemdelegate.h"
#include "configoptiondefinitionmodel.h"
#include "optionsortfilterproxymodel.h"
#include "paramconfigeditor.h"
#include "scheme.h"
#include "ui_paramconfigeditor.h"

#include <QScrollBar>
#include <QMessageBox>
#include <QMenu>
#include <QClipboard>

#include <QDebug>

namespace gams {
namespace studio {
namespace option {

ParamConfigEditor::ParamConfigEditor(const QList<ConfigItem *> &initParams, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParamConfigEditor),
    mModified(false)
{
    ui->setupUi(this);
    addActions();
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

void ParamConfigEditor::addActions()
{
    mToolBar = new QToolBar();
    mToolBar ->setIconSize(QSize(16,16));
//    QIcon::Mode mode = QIcon::Disabled;
//    QIcon::State state = QIcon::Off;

//    QAction* commentAction = mContextMenu.addAction("Toggle comment/option selection", [this]() { toggleCommentOption(); });
//    commentAction->setObjectName("actionToggle_comment");
//    commentAction->setShortcut( QKeySequence("Ctrl+T") );
//    commentAction->setShortcutVisibleInContextMenu(true);
//    commentAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
//    ui->ParamCfgTableView->addAction(commentAction);
//    addAction(commentAction);

    QIcon plusIcon = Scheme::icon(":/img/plus");
    plusIcon.pixmap( QSize(16, 16), QIcon::Disabled
                                  , QIcon::Off );
    QAction* insertOptionAction = mContextMenu.addAction(plusIcon, "Insert new parameter", [this]() { insertOption(); });
    insertOptionAction->setObjectName("actionInsert_option");
    insertOptionAction->setShortcut( QKeySequence("Ctrl+Return") );
    insertOptionAction->setShortcutVisibleInContextMenu(true);
    insertOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
//    insertOptionAction->setEnabled(false);
//    insertOptionAction->setChecked(false);
    ui->ParamCfgTableView->addAction(insertOptionAction);
    mToolBar->addAction(insertOptionAction);

    QAction* deleteAction = mContextMenu.addAction(Scheme::icon(":/img/delete-all"), "Delete selection", [this]() { deleteOption(); });
    deleteAction->setObjectName("actionDelete_option");
    deleteAction->setShortcut( QKeySequence("Ctrl+Delete") );
    deleteAction->setShortcutVisibleInContextMenu(true);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgTableView->addAction(deleteAction);

    QAction* moveUpAction = mContextMenu.addAction(Scheme::icon(":/img/move-up"), "Move up", [this]() { moveOptionUp(); });
    moveUpAction->setObjectName("actionMoveUp_option");
    moveUpAction->setShortcut( QKeySequence("Ctrl+Up") );
    moveUpAction->setShortcutVisibleInContextMenu(true);
    moveUpAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgTableView->addAction(moveUpAction);

    QAction* moveDownAction = mContextMenu.addAction(Scheme::icon(":/img/move-down"), "Move down", [this]() { moveOptionDown(); });
    moveDownAction->setObjectName("actionMoveDown_option");
    moveDownAction->setShortcut( QKeySequence("Ctrl+Down") );
    moveDownAction->setShortcutVisibleInContextMenu(true);
    moveDownAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgTableView->addAction(moveDownAction);

    QAction* selectAll = mContextMenu.addAction("Select all", ui->ParamCfgTableView, [this]() { selectAllOptions(); });
    selectAll->setObjectName("actionSelect_all");
    selectAll->setShortcut( QKeySequence("Ctrl+A") );
    selectAll->setShortcutVisibleInContextMenu(true);
    selectAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgTableView->addAction(selectAll);

    QAction* anotehrSelectAll = mContextMenu.addAction("Select All", ui->ParamCfgDefTreeView, [this]() { selectAllOptions(); });
    anotehrSelectAll->setObjectName("actionSelect_all");
    anotehrSelectAll->setShortcut( QKeySequence("Ctrl+A") );
    anotehrSelectAll->setShortcutVisibleInContextMenu(true);
    anotehrSelectAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgDefTreeView->addAction(anotehrSelectAll);

    QAction* addThisOptionAction = mContextMenu.addAction(Scheme::icon(":/img/plus"), "Add this option", [this]() {
        QModelIndexList selection = ui->ParamCfgDefTreeView->selectionModel()->selectedRows();
        if (selection.size()>0) {
            ui->ParamCfgTableView->clearSelection();
            ui->ParamCfgDefTreeView->clearSelection();
            addParameterFromDefinition(selection.at(0));
        }
    });
    addThisOptionAction->setObjectName("actionAddThisOption");
    addThisOptionAction->setShortcut( QKeySequence(Qt::Key_Return) );
    addThisOptionAction->setShortcutVisibleInContextMenu(true);
    addThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgDefTreeView->addAction(addThisOptionAction);

    QAction* deleteThisOptionAction = mContextMenu.addAction(Scheme::icon(":/img/delete-all"), "Remove this option", [this]() {
        findAndSelectionParameterFromDefinition();
        deleteOption();
    });
    deleteThisOptionAction->setObjectName("actionDeleteThisOption");
    deleteThisOptionAction->setShortcut( QKeySequence(Qt::Key_Delete) );
    deleteThisOptionAction->setShortcutVisibleInContextMenu(true);
    deleteThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgDefTreeView->addAction(deleteThisOptionAction);

    QAction* copyDefinitionOptionNameAction = mContextMenu.addAction("Copy option name\tCtrl+C",
                                                                     [this]() { copyDefinitionToClipboard( ConfigOptionDefinitionModel::COLUMN_OPTION_NAME ); });
    copyDefinitionOptionNameAction->setObjectName("actionCopyDefinitionOptionName");
    copyDefinitionOptionNameAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgDefTreeView->addAction(copyDefinitionOptionNameAction);

    QAction* copyDefinitionOptionDescriptionAction = mContextMenu.addAction("Copy option description",
                                                                            [this]() { copyDefinitionToClipboard( ConfigOptionDefinitionModel::COLUMN_DESCIPTION ); });
    copyDefinitionOptionDescriptionAction->setObjectName("actionCopyDefinitionOptionDescription");
    copyDefinitionOptionDescriptionAction->setShortcut( QKeySequence("Shift+C") );
    copyDefinitionOptionDescriptionAction->setShortcutVisibleInContextMenu(true);
    copyDefinitionOptionDescriptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgDefTreeView->addAction(copyDefinitionOptionDescriptionAction);

    QAction* copyDefinitionTextAction = mContextMenu.addAction("Copy definition text",
                                                               [this]() { copyDefinitionToClipboard( -1 ); });
    copyDefinitionTextAction->setObjectName("actionCopyDefinitionText");
    copyDefinitionTextAction->setShortcut( QKeySequence("Ctrl+Shift+C") );
    copyDefinitionTextAction->setShortcutVisibleInContextMenu(true);
    copyDefinitionTextAction->setShortcutContext(Qt::WidgetShortcut);
    ui->ParamCfgDefTreeView->addAction(copyDefinitionTextAction);

    QAction* showDefinitionAction = mContextMenu.addAction("Show option definition", [this]() { showOptionDefinition(true); });
    showDefinitionAction->setObjectName("actionShowDefinition_option");
    showDefinitionAction->setShortcut( QKeySequence("Ctrl+F1") );
    showDefinitionAction->setShortcutVisibleInContextMenu(true);
    showDefinitionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgTableView->addAction(showDefinitionAction);

    QAction* showDuplicationAction = mContextMenu.addAction("Show all options of the same definition", [this]() { showOptionRecurrence(); });
    showDuplicationAction->setObjectName("actionShowRecurrence_option");
    showDuplicationAction->setShortcut( QKeySequence("Shift+F1") );
    showDuplicationAction->setShortcutVisibleInContextMenu(true);
    showDuplicationAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgTableView->addAction(showDuplicationAction);

    QAction* resizeColumns = mContextMenu.addAction("Resize columns to contents", [this]() { resizeColumnsToContents(); });
    resizeColumns->setObjectName("actionResize_columns");
    resizeColumns->setShortcut( QKeySequence("Ctrl+R") );
    resizeColumns->setShortcutVisibleInContextMenu(true);
    resizeColumns->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->ParamCfgTableView->addAction(resizeColumns);
    ui->ParamCfgDefTreeView->addAction(resizeColumns);

//    tb->addAction(insertOptionAction);
    mToolBar->addAction(deleteAction);
    mToolBar->addSeparator();
    mToolBar->addAction(moveUpAction);
    mToolBar->addAction(moveDownAction);
    ui-> ParamCfgCtrl->layout()->setMenuBar(mToolBar);

    //            action->icon().pixmap( QSize(16, 16), isEnabled ? QIcon::Active : QIcon::Disabled,
    //                                                  action->isChecked() ? QIcon::On : QIcon::Off);
//    ui->AddParamCfgButton->setIcon( Scheme::icon(":/img/plus").pixmap(QSize(16,16)) );

}

void ParamConfigEditor::init(const QList<ConfigItem *> &initParamItems)
{
    mOptionTokenizer = new OptionTokenizer(GamsOptDefFile);

    setFocusPolicy(Qt::StrongFocus);

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

    ui->ParamCfgTableView->setColumnHidden(ConfigParamTableModel::COLUMN_ENTRY_NUMBER, false); // TODO (JP) true);
    ui->ParamCfgTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->ParamCfgTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    ui->ParamCfgTableView->horizontalHeader()->setStretchLastSection(true);
    ui->ParamCfgTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->ParamCfgTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

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
}

bool ParamConfigEditor::isInFocus(QWidget *focusWidget) const
{
    return (focusWidget==ui->ParamCfgTableView || focusWidget==ui->ParamCfgDefTreeView);
}

void ParamConfigEditor::currentTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    qDebug() << "current:" << current.row() << "," << current.column();
    for(QAction* action : ui->ParamCfgTableView->actions()) {
        if (action->objectName().compare("actionInsert_option")==0) {
                    action->setEnabled(mParameterTableModel->rowCount()>0);
                } else if (action->objectName().compare("actionDelete_option")==0) {
                          action->setEnabled(mParameterTableModel->rowCount()>0);
                } else if (action->objectName().compare("actionMoveUp_option")==0) {
                          action->setEnabled(current.row() > 0);
                } else if (action->objectName().compare("actionMoveDown_option")==0) {
                          bool enabled = current.row() < mParameterTableModel->rowCount()-1;
                          action->setEnabled(enabled);
        } else {
            action->setDisabled(false);
        }
        action->icon().pixmap( QSize(16, 16), action->isEnabled() ? QIcon::Normal : QIcon::Disabled,
                                              action->isChecked() ? QIcon::On : QIcon::Off);
    }
    mToolBar->repaint();
}

void ParamConfigEditor::showParameterContextMenu(const QPoint &pos)
{
    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
    bool thereIsARowSelection = isThereARowSelection();
    bool thereIsAParamSelection = false;
    for (QModelIndex s : selection) {
        QVariant data = ui->ParamCfgTableView->model()->headerData(s.row(), Qt::Vertical,  Qt::CheckStateRole);
        if (Qt::CheckState(data.toUInt())!=Qt::PartiallyChecked) {
            thereIsAParamSelection = true;
            break;
        }
    }

    QMenu menu(this);
    for(QAction* action : ui->ParamCfgTableView->actions()) {
        if (action->objectName().compare("actionInsert_option")==0) {
            if (!isThereARow() || isThereARowSelection())
                 menu.addAction(action);
        } else if (action->objectName().compare("actionDelete_option")==0) {
                  if ( thereIsARowSelection )
                     menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionMoveUp_option")==0) {
                  if (  thereIsARowSelection && (selection.first().row() > 0) )
                     menu.addAction(action);
        } else if (action->objectName().compare("actionMoveDown_option")==0) {
                  if ( thereIsARowSelection && (selection.last().row() < mParameterTableModel->rowCount()-1) )
                     menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionSelect_all")==0) {
                  if ( isThereARow() )
                      menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionShowDefinition_option")==0) {
            if ( thereIsAParamSelection )
                menu.addAction(action);
        } else if (action->objectName().compare("actionShowRecurrence_option")==0) {
                  if ( indexSelection.size()>=1 && thereIsAParamSelection && getRecurrentOption(indexSelection.at(0)).size()>0 )
                      menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionResize_columns")==0) {
                  if ( isThereARow() )
                     menu.addAction(action);
        }
    }

    menu.exec(ui->ParamCfgTableView->viewport()->mapToGlobal(pos));
}

void ParamConfigEditor::showDefinitionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->ParamCfgDefTreeView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    bool hasSelectionBeenAdded = (selection.size()>0);
    // assume single selection
    for (QModelIndex idx : selection) {
        QModelIndex parentIdx = ui->ParamCfgDefTreeView->model()->parent(idx);
        QVariant data = (parentIdx.row() < 0) ?  ui->ParamCfgDefTreeView->model()->data(idx, Qt::CheckStateRole)
                                              : ui->ParamCfgDefTreeView->model()->data(parentIdx, Qt::CheckStateRole);
        hasSelectionBeenAdded = (Qt::CheckState(data.toInt()) == Qt::Checked);
    }

    QMenu menu(this);
    for(QAction* action : ui->ParamCfgDefTreeView->actions()) {
        if (action->objectName().compare("actionAddThisOption")==0) {
            if ( !hasSelectionBeenAdded && ui->ParamCfgTableView->selectionModel()->selectedRows().size() <= 0 )
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDeleteThisOption")==0) {
                  if ( hasSelectionBeenAdded && ui->ParamCfgTableView->selectionModel()->selectedRows().size() > 0)
                      menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionCopyDefinitionOptionName")==0) {
                  menu.addAction(action);
        } else if (action->objectName().compare("actionCopyDefinitionOptionDescription")==0) {
                  menu.addAction(action);
        } else if (action->objectName().compare("actionCopyDefinitionText")==0) {
                  menu.addAction(action);
                  menu.addSeparator();
        }  else if (action->objectName().compare("actionResize_columns")==0) {
                if ( ui->ParamCfgDefTreeView->model()->rowCount()>0 )
                   menu.addAction(action);
        }
    }

    menu.exec(ui->ParamCfgDefTreeView->viewport()->mapToGlobal(pos));
}

void ParamConfigEditor::parameterItemCommitted(QWidget *editor)
{
    Q_UNUSED(editor)
    if (mOptionCompleter->currentEditedIndex().isValid()) {
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
}

void ParamConfigEditor::findAndSelectionParameterFromDefinition()
{
    if (ui->ParamCfgTableView->model()->rowCount() <= 0)
        return;

    QModelIndex index = ui->ParamCfgDefTreeView->selectionModel()->currentIndex();
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

void ParamConfigEditor::insertOption()
{
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
        QModelIndex insertKeyIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_KEY);
        QModelIndex insertValueIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_VALUE);
        QModelIndex insertNumberIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_ENTRY_NUMBER);
        QModelIndex minVersionIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_MIN_VERSION);
        QModelIndex maxVersionIndex = ui->ParamCfgTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_MIN_VERSION);

        ui->ParamCfgTableView->model()->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                        Qt::CheckState(Qt::Checked),
                                                        Qt::CheckStateRole );

        ui->ParamCfgTableView->model()->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
        ui->ParamCfgTableView->model()->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
        ui->ParamCfgTableView->model()->setData( minVersionIndex, "", Qt::EditRole);
        ui->ParamCfgTableView->model()->setData( maxVersionIndex, "", Qt::EditRole);
        ui->ParamCfgTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);
        ui->ParamCfgTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);
    } else {
        ui->ParamCfgTableView->model()->insertRows(ui->ParamCfgTableView->model()->rowCount(), 1, QModelIndex());
        rowToBeInserted = mParameterTableModel->rowCount()-1;
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
    }
    connect(mParameterTableModel, &QAbstractTableModel::dataChanged,
            mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);

    emit modificationChanged(true);

    ui->ParamCfgTableView->clearSelection();
    ui->ParamCfgTableView->selectRow(rowToBeInserted);
    ui->ParamCfgTableView->edit( mParameterTableModel->index(rowToBeInserted, ConfigParamTableModel::COLUMN_PARAM_KEY));
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
// TODO (JP)
//        emit itemCountChanged(ui->ParamCfgTableView->model()->rowCount());
    }
}

void ParamConfigEditor::moveOptionUp()
{
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
        ui->ParamCfgTableView->model()->moveRows(QModelIndex(), idx.row(), 1,
                                                         QModelIndex(), idx.row()-1);
    }
//    ui->ParamCfgTableView->model()->moveRows(QModelIndex(), index.row(), selection.count(),
//                                                 QModelIndex(), index.row()-1);
    emit modificationChanged(true);
}

void ParamConfigEditor::moveOptionDown()
{
    QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() > b.row(); });
    if  (idxSelection.first().row() >= mParameterTableModel->rowCount()-1)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
        mParameterTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                    QModelIndex(), idx.row()+2);
    }
//    mOptionTableModel->moveRows(QModelIndex(), index.row(), selection.count(),
//                                QModelIndex(), index.row()+selection.count()+1);
    emit modificationChanged(true);
}

void ParamConfigEditor::selectAllOptions()
{
    ui->ParamCfgTableView->setFocus();
    ui->ParamCfgTableView->selectAll();
}

void ParamConfigEditor::resizeColumnsToContents()
{
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

void ParamConfigEditor::deSelectOptions()
{
    if (ui->ParamCfgTableView->hasFocus() && ui->ParamCfgTableView->selectionModel()->hasSelection())
        ui->ParamCfgTableView->selectionModel()->clearSelection();
    else if (ui->ParamCfgDefTreeView->hasFocus() && ui->ParamCfgDefTreeView->selectionModel()->hasSelection())
             ui->ParamCfgDefTreeView->selectionModel()->clearSelection();
    else
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
    ui->ParamCfgTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
    ui->ParamCfgTableView->selectRow(rowToBeAdded);
    selectAnOption();

    QString text =  mParameterTableModel->getParameterTableEntry(insertNumberIndex.row());
    if (replaceExistingEntry)
        mOptionTokenizer->logger()->append(QString("Option entry '%1' has been replaced").arg(text), LogMsgType::Info);
    else
        mOptionTokenizer->logger()->append(QString("Option entry '%1' has been added").arg(text), LogMsgType::Info);

    int lastColumn = ui->ParamCfgTableView->model()->columnCount()-1;
    int lastRow = rowToBeAdded;
    int firstRow = lastRow;
//    if (settings && settings->addCommentDescriptionAboveOption()) {
//        firstRow--;
//        if (parentIndex.row() >=0)
//            firstRow--;
//    }
    if (firstRow<0)
        firstRow = 0;
     mParameterTableModel->on_updateConfigParamItem( ui->ParamCfgTableView->model()->index(firstRow, lastColumn),
                                                     ui->ParamCfgTableView->model()->index(lastRow, lastColumn),
                                                     {Qt::EditRole});

    connect( mParameterTableModel, &QAbstractTableModel::dataChanged,  mParameterTableModel, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);

    showOptionDefinition(true);
// TODO (JP)
//    emit itemCountChanged(ui->ParamCfgTableView->model()->rowCount());

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

void ParamConfigEditor::showOptionRecurrence()
{
    if (!isInFocus(focusWidget()))
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

    if (!isInFocus(focusWidget()))
        return optionList;

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

void ParamConfigEditor::deleteParameter()
{
     QModelIndexList indexSelection = ui->ParamCfgTableView->selectionModel()->selectedIndexes();
     for(QModelIndex index : indexSelection) {
         ui->ParamCfgTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
     }

     QModelIndexList selection = ui->ParamCfgTableView->selectionModel()->selectedRows();
     if (selection.count() <= 0)
        return;

    disconnect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
               this, &ParamConfigEditor::findAndSelectionParameterFromDefinition);

    QModelIndex index = selection.at(0);
    QModelIndex removeTableIndex = ui->ParamCfgTableView->model()->index(index.row(), 0);
    QVariant optionName = ui->ParamCfgTableView->model()->data(removeTableIndex, Qt::DisplayRole);

    QModelIndexList items = ui->ParamCfgTableView->model()->match(ui->ParamCfgTableView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                  Qt::DisplayRole,
                                                                  optionName, -1);
    QModelIndexList definitionItems = ui->ParamCfgDefTreeView->model()->match(ui->ParamCfgDefTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);

    QList<int> rows;
    for(const QModelIndex & index : ui->ParamCfgTableView->selectionModel()->selectedRows()) {
        rows.append( index.row() );
    }
    std::sort(rows.begin(), rows.end());
    int prev = -1;
    for(int i=rows.count()-1; i>=0; i--) {
        int current = rows[i];
        if (current != prev) {
            ui->ParamCfgTableView->model()->removeRows( current, 1 );
            prev = current;
        }
    }

    ui->ParamCfgDefTreeView->clearSelection();
    ui->ParamCfgTableView->setFocus();
    connect(ui->ParamCfgDefTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ParamConfigEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

bool ParamConfigEditor::isThereARow() const
{
    return (ui->ParamCfgTableView->model()->rowCount() > 0);
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


}
}
}
