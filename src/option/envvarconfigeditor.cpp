﻿/*
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
#include "envvarconfigeditor.h"
#include "envvarcfgcompleterdelegate.h"
#include "ui_envvarconfigeditor.h"
#include "theme.h"

#include <QTimer>

namespace gams {
namespace studio {
namespace option {

EnvVarConfigEditor::EnvVarConfigEditor(const QList<EnvVarConfigItem *> &initItems, QWidget *parent):
    QWidget(parent),
    ui(new Ui::EnvVarConfigEditor),
    mModified(false)
{
    ui->setupUi(this);
    init(initItems);
}

EnvVarConfigEditor::~EnvVarConfigEditor()
{
    delete ui;
    if (mEnvVarTableModel)
        delete mEnvVarTableModel;
}

QList<QHeaderView *> EnvVarConfigEditor::headers()
{
    return QList<QHeaderView *>() << ui->EnvVarConfigTableView->horizontalHeader()
                                  << ui->EnvVarConfigTableView->verticalHeader();
}

void EnvVarConfigEditor::parameterItemCommitted(QWidget *editor)
{
    Q_UNUSED(editor)
    if (mCompleter->currentEditedIndex().isValid()) {
        ui->EnvVarConfigTableView->resizeColumnToContents( mCompleter->currentEditedIndex().column() );
    }
}

void EnvVarConfigEditor::currentTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (ui->EnvVarConfigTableView->selectionModel()->selectedRows().isEmpty()) {
        updateActionsState(current);
    } else {
        if (ui->EnvVarConfigTableView->selectionModel()->isRowSelected(current.row(), current.parent()) )
            updateActionsState();
        else
            updateActionsState(current);
    }
}

void EnvVarConfigEditor::updateActionsState(const QModelIndex &index)
{
    ui->actionInsert->setEnabled( true );
    ui->actionDelete->setEnabled( false );
    ui->actionMoveUp->setEnabled( false );
    ui->actionMoveDown->setEnabled( false );
    ui->actionSelect_Current_Row->setEnabled( isThereAnIndexSelection() );
    ui->actionSelectAll->setEnabled( isThereARow() );
    ui->actionResize_Columns_To_Contents->setEnabled( index.row() < mEnvVarTableModel->rowCount() );
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

void EnvVarConfigEditor::updateActionsState()
{
    QModelIndexList idxSelection = ( ui->EnvVarConfigTableView->selectionModel()->selectedRows().isEmpty()
                                         ? ui->EnvVarConfigTableView->selectionModel()->selectedIndexes()
                                         : ui->EnvVarConfigTableView->selectionModel()->selectedRows() );

    if (idxSelection.isEmpty())
        return;

    ui->actionInsert->setEnabled( true );
    ui->actionDelete->setEnabled( idxSelection.first().row() < mEnvVarTableModel->rowCount() );

    ui->actionMoveUp->setEnabled( idxSelection.first().row() > 0 );
    ui->actionMoveDown->setEnabled( idxSelection.last().row() < mEnvVarTableModel->rowCount()-1 );

    ui->actionSelect_Current_Row->setEnabled( isThereAnIndexSelection() );
    ui->actionSelectAll->setEnabled( isThereARow( ));

    ui->actionResize_Columns_To_Contents->setEnabled( idxSelection.first().row() < mEnvVarTableModel->rowCount() );
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

void EnvVarConfigEditor::on_selectRow(int logicalIndex)
{
    if (ui->EnvVarConfigTableView->model()->rowCount() <= 0)
        return;

    QItemSelectionModel *selectionModel = ui->EnvVarConfigTableView->selectionModel();
    QModelIndex topLeft = ui->EnvVarConfigTableView->model()->index(logicalIndex, EnvVarTableModel::COLUMN_PARAM_KEY, QModelIndex());
    QModelIndex  bottomRight = ui->EnvVarConfigTableView->model()->index(logicalIndex, EnvVarTableModel::COLUMN_PARAM_VALUE, QModelIndex());
    QItemSelection selection( topLeft, bottomRight);
    selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    updateActionsState();
}

void EnvVarConfigEditor::showContextMenu(const QPoint &pos)
{
    QModelIndexList indexSelection = ui->EnvVarConfigTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
        ui->EnvVarConfigTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
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
    menu.addAction(ui->actionResize_Columns_To_Contents);
    menu.exec(ui->EnvVarConfigTableView->viewport()->mapToGlobal(pos));
}

void EnvVarConfigEditor::selectAll()
{
    ui->EnvVarConfigTableView->setFocus();
    ui->EnvVarConfigTableView->selectAll();

    updateActionsState( );
}

void EnvVarConfigEditor::deSelect()
{
    initActions();
    if (ui->EnvVarConfigTableView->hasFocus() && ui->EnvVarConfigTableView->selectionModel()->hasSelection())
        ui->EnvVarConfigTableView->selectionModel()->clearSelection();
    this->focusNextChild();
}

void EnvVarConfigEditor::on_reloadGamsUserConfigFile(const QList<EnvVarConfigItem *> &initItems)
{
    mEnvVarTableModel->on_reloadEnvVarModel(initItems);
    setModified(false);
}

void EnvVarConfigEditor::setModified(bool modified)
{
    mModified = modified;
}

bool EnvVarConfigEditor::isModified() const
{
    return mModified;
}

QList<EnvVarConfigItem *> EnvVarConfigEditor::envVarConfigItems()
{
    return mEnvVarTableModel->envVarConfigItems();
}

void EnvVarConfigEditor::init(const QList<EnvVarConfigItem *> &initItems)
{
    initActions();

    mToolBar = new QToolBar();
    mToolBar ->setIconSize(QSize(16,16));
    mToolBar->addAction(ui->actionInsert);
    mToolBar->addAction(ui->actionDelete);
    mToolBar->addSeparator();
    mToolBar->addAction(ui->actionMoveUp);
    mToolBar->addAction(ui->actionMoveDown);
    this->layout()->setMenuBar(mToolBar);

    setFocusPolicy(Qt::StrongFocus);

    mEnvVarTableModel = new EnvVarTableModel(initItems, this);
    ui->EnvVarConfigTableView->setModel( mEnvVarTableModel );

    mCompleter = new  EnvVarCfgCompleterDelegate(ui->EnvVarConfigTableView);
    ui->EnvVarConfigTableView->setItemDelegate( mCompleter );
    connect(mCompleter, &QStyledItemDelegate::commitData, this, &EnvVarConfigEditor::parameterItemCommitted);

    ui->EnvVarConfigTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::SelectedClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->EnvVarConfigTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->EnvVarConfigTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->EnvVarConfigTableView->setAutoScroll(true);
    ui->EnvVarConfigTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->EnvVarConfigTableView->setSortingEnabled(false);

    ui->EnvVarConfigTableView->setDragEnabled(true);
    ui->EnvVarConfigTableView->viewport()->setAcceptDrops(true);
    ui->EnvVarConfigTableView->setDropIndicatorShown(true);
    ui->EnvVarConfigTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->EnvVarConfigTableView->setDragDropOverwriteMode(true);
    ui->EnvVarConfigTableView->setDefaultDropAction(Qt::CopyAction);

    ui->EnvVarConfigTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->EnvVarConfigTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    ui->EnvVarConfigTableView->horizontalHeader()->setSectionResizeMode(EnvVarTableModel::COLUMN_PARAM_KEY, QHeaderView::Stretch);
    ui->EnvVarConfigTableView->horizontalHeader()->setSectionResizeMode(EnvVarTableModel::COLUMN_PARAM_VALUE, QHeaderView::Stretch);
    ui->EnvVarConfigTableView->horizontalHeader()->setStretchLastSection(true);

    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PARAM_KEY);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PARAM_VALUE);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_MIN_VERSION);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_MAX_VERSION);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PATH_VAR);

    ui->EnvVarConfigTableView->setTabKeyNavigation(true);

    connect(ui->EnvVarConfigTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &EnvVarConfigEditor::on_selectRow, Qt::UniqueConnection);
    connect(ui->EnvVarConfigTableView, &QTableView::customContextMenuRequested,this, &EnvVarConfigEditor::showContextMenu, Qt::UniqueConnection);
    connect(ui->EnvVarConfigTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &EnvVarConfigEditor::currentTableSelectionChanged);

    connect(mEnvVarTableModel, &QAbstractTableModel::dataChanged, this, &EnvVarConfigEditor::on_dataItemChanged, Qt::UniqueConnection);
    connect(mEnvVarTableModel, &QAbstractTableModel::dataChanged, mEnvVarTableModel, &EnvVarTableModel::on_updateEnvVarItem, Qt::UniqueConnection);
    connect(mEnvVarTableModel, &EnvVarTableModel::envVarItemRemoved, mEnvVarTableModel, &EnvVarTableModel::on_removeEnvVarItem, Qt::UniqueConnection);

    connect(this, &EnvVarConfigEditor::modificationChanged, this, &EnvVarConfigEditor::setModified, Qt::UniqueConnection);

    QTimer::singleShot(0, this, [this]() {
        ui->EnvVarConfigTableView->horizontalHeader()->setSectionResizeMode(EnvVarTableModel::COLUMN_PARAM_KEY, QHeaderView::Interactive);
        ui->EnvVarConfigTableView->horizontalHeader()->setSectionResizeMode(EnvVarTableModel::COLUMN_PARAM_VALUE, QHeaderView::Interactive);
    });
}

void EnvVarConfigEditor::initActions()
{
    ui->actionInsert->setEnabled(true);
    ui->actionDelete->setEnabled(false);
    ui->actionMoveUp->setEnabled(false);
    ui->actionMoveDown->setEnabled(false);
    ui->actionSelect_Current_Row->setEnabled(true);
    ui->actionSelectAll->setEnabled(true);
    ui->actionResize_Columns_To_Contents->setEnabled(true);

    ui->actionInsert->setIcon(Theme::icon(":/%1/plus", true));
    ui->actionDelete->setIcon(Theme::icon(":/%1/delete-all", true));
    ui->actionMoveUp->setIcon(Theme::icon(":/%1/move-up", true));
    ui->actionMoveDown->setIcon(Theme::icon(":/%1/move-down", true));
}

void EnvVarConfigEditor::on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    emit modificationChanged(true);
    updateActionsState(topLeft);
}

bool EnvVarConfigEditor::isThereARow() const
{
    return (ui->EnvVarConfigTableView->model()->rowCount() > 0);
}

bool EnvVarConfigEditor::isThereAnIndexSelection() const
{
    QModelIndexList selection = ui->EnvVarConfigTableView->selectionModel()->selectedIndexes();
    return (selection.count() > 0);
}

bool EnvVarConfigEditor::isThereARowSelection() const
{
    QModelIndexList selection = ui->EnvVarConfigTableView->selectionModel()->selectedRows();
    return (selection.count() > 0);
}

bool EnvVarConfigEditor::isEverySelectionARow() const
{
    QModelIndexList selection = ui->EnvVarConfigTableView->selectionModel()->selectedRows();
    QModelIndexList indexSelection = ui->EnvVarConfigTableView->selectionModel()->selectedIndexes();

    return ((selection.count() > 0) && (indexSelection.count() % ui->EnvVarConfigTableView->model()->columnCount() == 0));

}

void EnvVarConfigEditor::on_actionInsert_triggered()
{
    QModelIndexList indexSelection = ui->EnvVarConfigTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
        ui->EnvVarConfigTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    disconnect(mEnvVarTableModel, &QAbstractTableModel::dataChanged, mEnvVarTableModel, &EnvVarTableModel::on_updateEnvVarItem);
    int rowToBeInserted = -1;
    if (isThereARowSelection()) {
        QList<int> rows;
        for(QModelIndex idx : ui->EnvVarConfigTableView->selectionModel()->selectedRows()) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        rowToBeInserted = rows.at(0);
        ui->EnvVarConfigTableView->model()->insertRows(rowToBeInserted, 1, QModelIndex());
    } else {
        ui->EnvVarConfigTableView->model()->insertRows(ui->EnvVarConfigTableView->model()->rowCount(), 1, QModelIndex());
        rowToBeInserted = mEnvVarTableModel->rowCount()-1;
    }

    QModelIndex insertKeyIndex = ui->EnvVarConfigTableView->model()->index(rowToBeInserted, EnvVarTableModel::COLUMN_PARAM_KEY);
    QModelIndex insertValueIndex = ui->EnvVarConfigTableView->model()->index(rowToBeInserted, EnvVarTableModel::COLUMN_PARAM_VALUE);
    QModelIndex minVersionIndex = ui->EnvVarConfigTableView->model()->index(rowToBeInserted, EnvVarTableModel::COLUMN_MIN_VERSION);
    QModelIndex maxVersionIndex = ui->EnvVarConfigTableView->model()->index(rowToBeInserted, EnvVarTableModel::COLUMN_MIN_VERSION);
    QModelIndex pathVarIndex = ui->EnvVarConfigTableView->model()->index(rowToBeInserted, EnvVarTableModel::COLUMN_PATH_VAR);
    ui->EnvVarConfigTableView->model()->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                    Qt::CheckState(Qt::Checked),
                                                    Qt::CheckStateRole );

    ui->EnvVarConfigTableView->model()->setData( insertKeyIndex, "", Qt::EditRole);
    ui->EnvVarConfigTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
    ui->EnvVarConfigTableView->model()->setData( minVersionIndex, "", Qt::EditRole);
    ui->EnvVarConfigTableView->model()->setData( maxVersionIndex, "", Qt::EditRole);
    ui->EnvVarConfigTableView->model()->setData( pathVarIndex, "", Qt::EditRole);
    ui->EnvVarConfigTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);

    connect(mEnvVarTableModel, &QAbstractTableModel::dataChanged, mEnvVarTableModel, &EnvVarTableModel::on_updateEnvVarItem, Qt::UniqueConnection);

    emit modificationChanged(true);

    ui->EnvVarConfigTableView->clearSelection();
    ui->EnvVarConfigTableView->selectRow(rowToBeInserted);

    QModelIndex index = mEnvVarTableModel->index(rowToBeInserted, EnvVarTableModel::COLUMN_PARAM_KEY);
    ui->EnvVarConfigTableView->edit( index );
    updateActionsState(index);
}

void EnvVarConfigEditor::on_actionDelete_triggered()
{
    QModelIndexList indexSelection = ui->EnvVarConfigTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
        ui->EnvVarConfigTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        QItemSelection selection( ui->EnvVarConfigTableView->selectionModel()->selection() );

        QList<int> rows;
        for(const QModelIndex & index : ui->EnvVarConfigTableView->selectionModel()->selectedRows()) {
            rows.append( index.row() );
        }

        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            int current = rows[i];
            if (current != prev) {
                ui->EnvVarConfigTableView->model()->removeRows( current, 1 );
                prev = current;
            }
        }
        emit modificationChanged(true);
    }}

void EnvVarConfigEditor::on_actionMoveUp_triggered()
{
    QModelIndexList indexSelection =ui->EnvVarConfigTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
       ui->EnvVarConfigTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  ( !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection =ui->EnvVarConfigTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() <= 0)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
        mEnvVarTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                                         QModelIndex(), idx.row()-1);
    }
    emit modificationChanged(true);
    updateActionsState();
}

void EnvVarConfigEditor::on_actionMoveDown_triggered()
{
    QModelIndexList indexSelection =ui->EnvVarConfigTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
       ui->EnvVarConfigTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  ( !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection =ui->EnvVarConfigTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() >= mEnvVarTableModel->rowCount()-1)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
        mEnvVarTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                    QModelIndex(), idx.row()+2);
    }
    emit modificationChanged(true);
    updateActionsState();
}

void EnvVarConfigEditor::on_actionSelect_Current_Row_triggered()
{
   QList<int> rowList;
   for(QModelIndex idx : ui->EnvVarConfigTableView->selectionModel()->selectedIndexes()) {
       if (!rowList.contains(idx.row())) {
           on_selectRow(idx.row());
           rowList << idx.row();
       }
   }
}

void EnvVarConfigEditor::on_actionSelectAll_triggered()
{
    selectAll();
}

void EnvVarConfigEditor::on_actionResize_Columns_To_Contents_triggered()
{
    if (ui->EnvVarConfigTableView->model()->rowCount()<=0)
        return;

    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PARAM_KEY);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PARAM_VALUE);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_MIN_VERSION);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_MAX_VERSION);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PATH_VAR);
}

}
}
}
