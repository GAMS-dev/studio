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
#include "envvarconfigeditor.h"
#include "ui_envvarconfigeditor.h"

#include <QDebug>

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

void EnvVarConfigEditor::currentTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    qDebug() << "current:" << current.row() << "," << current.column();

     ui->actionInsert->setEnabled(mEnvVarTableModel->rowCount()>0);
     ui->actionDelete->setEnabled(mEnvVarTableModel->rowCount()>0);
     ui->actionMoveUp->setEnabled(current.row() > 0);
     ui->actionMoveDown->setEnabled( current.row() < mEnvVarTableModel->rowCount()-1 );
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

void EnvVarConfigEditor::showContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    menu.addAction(ui->actionInsert);
    menu.addAction(ui->actionDelete);
    menu.addSeparator();
    menu.addAction(ui->actionMoveUp);
    menu.addAction(ui->actionMoveDown);
    menu.exec(ui->EnvVarConfigTableView->viewport()->mapToGlobal(pos));
}

void EnvVarConfigEditor::deSelectOptions()
{
    initActions();
    if (ui->EnvVarConfigTableView->hasFocus() && ui->EnvVarConfigTableView->selectionModel()->hasSelection())
        ui->EnvVarConfigTableView->selectionModel()->clearSelection();
    else
        this->focusNextChild();
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
    ui->EnvVarConfigTableView->horizontalHeader()->setStretchLastSection(true);
    ui->EnvVarConfigTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->EnvVarConfigTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PARAM_KEY);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PARAM_VALUE);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_MIN_VERSION);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_MAX_VERSION);
    ui->EnvVarConfigTableView->resizeColumnToContents(EnvVarTableModel::COLUMN_PATH_VAR);

    ui->EnvVarConfigTableView->setTabKeyNavigation(true);

    connect(ui->EnvVarConfigTableView, &QTableView::customContextMenuRequested,this, &EnvVarConfigEditor::showContextMenu, Qt::UniqueConnection);
    connect(ui->EnvVarConfigTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &EnvVarConfigEditor::currentTableSelectionChanged);
}

void EnvVarConfigEditor::initActions()
{
    ui->actionInsert->setEnabled(true);
    ui->actionDelete->setEnabled(false);
    ui->actionMoveUp->setEnabled(false);
    ui->actionMoveDown->setEnabled(false);
    ui->actionInsert->icon().pixmap( QSize(16, 16), QIcon::Selected, QIcon::Off);
    ui->actionDelete->icon().pixmap( QSize(16, 16), QIcon::Disabled, QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), QIcon::Disabled, QIcon::Off);
    ui->actionDelete->icon().pixmap( QSize(16, 16), QIcon::Disabled, QIcon::Off);
}

void EnvVarConfigEditor::on_actionInsert_triggered()
{
    qDebug() << "insert";
}

void EnvVarConfigEditor::on_actionDelete_triggered()
{
    qDebug() << "delete";
}

void EnvVarConfigEditor::on_actionMoveUp_triggered()
{
    qDebug() << "move up";
}

void EnvVarConfigEditor::on_actionMoveDown_triggered()
{
    qDebug() << "move down";
}

}
}
}
