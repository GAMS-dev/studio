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
#include <QDebug>
#include <QStandardItemModel>

#include "connecteditor.h"
#include "connectdatakeydelegate.h"
#include "connectdatavaluedelegate.h"
#include "connectdataactiondelegate.h"
#include "schemadefinitionmodel.h"
#include "theme.h"
#include "headerviewproxy.h"
#include "ui_connecteditor.h"

namespace gams {
namespace studio {
namespace connect {

ConnectEditor::ConnectEditor(const QString& connectDataFileName, QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ConnectEditor),
    mLocation(connectDataFileName)
{
    init();
}

bool ConnectEditor::init()
{
    ui->setupUi(this);
    setFocusProxy(ui->dataTreeView);

    mConnect = new Connect();
    QStringList schema = mConnect->getSchemaNames();

    QStandardItemModel* schemaItemModel = new QStandardItemModel( mConnect->getSchemaNames().size(), 1, this );
//    QStandardItemModel* schemaHelpModel = new QStandardItemModel( mConnect->getSchemaNames().size(), 1, this );

    for(int row=0; row<mConnect->getSchemaNames().size(); row++) {
        QStandardItem *item = new QStandardItem();
        item->setData( mConnect->getSchemaNames().at(row), Qt::DisplayRole );
        item->setIcon(Theme::icon(":/%1/plus",  QIcon::Mode::Disabled));
        item->setEditable(false);
        item->setSelectable(true);
        item->setTextAlignment(Qt::AlignLeft);
        item->setForeground(Theme::color(Theme::Syntax_embedded));
        schemaItemModel->setItem(row, 0, item);
    }

    ui->schemaControlListView->setModel(schemaItemModel);

    ui->schemaControlListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->schemaControlListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->schemaControlListView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->schemaControlListView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ui->schemaControlListView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    ui->schemaControlListView->setViewMode(QListView::ListMode);
    ui->schemaControlListView->setIconSize(QSize(14,14));
    ui->schemaControlListView->setAutoFillBackground(true);
    ui->schemaControlListView->setBackgroundRole(QPalette::NoRole);
    ui->schemaControlListView->viewport()->setAutoFillBackground(true);
//    QPalette palette = ui->schemaControlListView->viewport()->palette();
//    palette.setColor(QPalette::Background, Qt::gray);
//    ui->schemaControlListView->viewport()->setPalette(palette);
    ui->schemaControlListView->setCurrentIndex(schemaItemModel->index(0,0));

    ui->connectHSplitter->setStretchFactor(0, 4);
    ui->connectHSplitter->setStretchFactor(1, 3);
    ui->connectHSplitter->setStretchFactor(2, 5);

    mDataModel = new ConnectDataModel(mLocation, mConnect, this);
    ui->dataTreeView->setModel( mDataModel );

    ConnectDataValueDelegate* itemdelegate = new ConnectDataValueDelegate(mConnect, ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn(1, itemdelegate );

    ConnectDataKeyDelegate* keydelegate = new ConnectDataKeyDelegate( ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::Key, keydelegate);
    ConnectDataActionDelegate* actiondelegate = new ConnectDataActionDelegate( ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::Delete, actiondelegate);
    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::MoveDown, actiondelegate);
    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::MoveUp, actiondelegate);
//    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::Expand, actiondelegate);

//    ui->dataTreeView->header()->setSectionResizeMode((int)DataItemColumn::Key, QHeaderView::Fixed);
//    ui->dataTreeView->header()->setSectionResizeMode((int)DataItemColumn::Value, QHeaderView::ResizeToContents);
    ui->dataTreeView->header()->setSectionResizeMode((int)DataItemColumn::Delete, QHeaderView::ResizeToContents);
    ui->dataTreeView->header()->setSectionResizeMode((int)DataItemColumn::MoveDown, QHeaderView::ResizeToContents);
    ui->dataTreeView->header()->setSectionResizeMode((int)DataItemColumn::MoveUp, QHeaderView::ResizeToContents);
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->dataTreeView->header()->setStyle(HeaderViewProxy::instance());
    ui->dataTreeView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::SelectedClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->dataTreeView->setSelectionMode(QAbstractItemView::NoSelection); //:SingleSelection);
    ui->dataTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->dataTreeView->setItemsExpandable(true);
    ui->dataTreeView->setAutoScroll(true);
//    updateDataColumnSpan();
    ui->dataTreeView->expandAll();
    for (int i=0; i< ui->dataTreeView->model()->columnCount(); i++)
        ui->dataTreeView->resizeColumnToContents(i);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::CheckState, true);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::SchemaType, true);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::AllowedValue, true);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::Expand, true);
    headerRegister(ui->dataTreeView->header());

    SchemaDefinitionModel* defmodel = new SchemaDefinitionModel(mConnect, mConnect->getSchemaNames().first(), this);
    ui->helpTreeView->setModel( defmodel );
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->helpTreeView->header()->setStyle(HeaderViewProxy::instance());
    ui->helpTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->helpTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->helpTreeView->setItemsExpandable(true);
    ui->helpTreeView->expandAll();
    ui->helpTreeView->resizeColumnToContents(0);
    ui->helpTreeView->resizeColumnToContents(1);
    ui->helpTreeView->resizeColumnToContents(2);
    ui->helpTreeView->resizeColumnToContents(3);
    ui->helpTreeView->resizeColumnToContents(4);
    headerRegister(ui->helpTreeView->header());

    connect(keydelegate, &ConnectDataKeyDelegate::requestSchemaHelp, this, &ConnectEditor::schemaHelpRequested);

    connect(ui->schemaControlListView, &QListView::clicked,  [=](const QModelIndex &index) {
        defmodel->loadSchemaFromName( schemaItemModel->data( schemaItemModel->index(index.row(),0) ).toString() );
    });
    connect(ui->schemaControlListView, &QListView::doubleClicked, this, &ConnectEditor::schemaDoubleClicked);

    connect(mDataModel, &ConnectDataModel::rowsAboutToBeInserted, [this]() {
        saveExpandedState();
    });
    connect(mDataModel, &ConnectDataModel::modelReset, [this]() {
        restoreExpandedState();
        ui->dataTreeView->resizeColumnToContents(0);
        ui->dataTreeView->resizeColumnToContents(1);
    });
///    connect(ui->dataTreeView->selectionModel(),
///            &QItemSelectionModel::selectionChanged, this,
///            &ConnectEditor::on_dataTreeSelectionChanged, Qt::UniqueConnection);

    connect(defmodel, &SchemaDefinitionModel::modelReset, [this]() {
        ui->helpTreeView->expandAll();
        ui->helpTreeView->resizeColumnToContents(0);
        ui->helpTreeView->resizeColumnToContents(1);
        ui->helpTreeView->resizeColumnToContents(2);
        ui->helpTreeView->resizeColumnToContents(3);
        ui->helpTreeView->resizeColumnToContents(4);
    });
    return true;
}

ConnectEditor::~ConnectEditor()
{
    delete ui;
    if (mDataModel)
        delete mDataModel;
    if (mConnect)
        delete mConnect;
}

void ConnectEditor::schemaDoubleClicked(const QModelIndex &modelIndex)
{

    qDebug() << "doubleclikced row=" << modelIndex.row() << ", col=" << modelIndex.column()
             << ui->schemaControlListView->model()->data( modelIndex ).toString();

    QStringList strlist;
    strlist << ui->schemaControlListView->model()->data( modelIndex ).toString();

    mDataModel->addFromSchema( mConnect->createDataHolder(strlist), mDataModel->rowCount() );
    ui->dataTreeView->expandRecursively( mDataModel->index( mDataModel->rowCount()-1, 0) );
//    updateDataColumnSpan();
    for (int i=0; i< ui->dataTreeView->model()->columnCount(); i++)
        ui->dataTreeView->resizeColumnToContents(i);
}

void ConnectEditor::updateDataColumnSpan(const QModelIndex &modelIndex)
{
    qDebug() << "updateColumnSpan " << mDataModel->rowCount();
    iterateModelItem( ui->dataTreeView->rootIndex());
}

void ConnectEditor::schemaHelpRequested(const QString &schemaName)
{
   for(int row=0; row<mConnect->getSchemaNames().size(); row++) {
       QString str = mConnect->getSchemaNames().at(row);
       if (str.compare(schemaName, Qt::CaseInsensitive)==0) {
           QModelIndex index = ui->schemaControlListView->model()->index(row, 0);
           ui->schemaControlListView->setCurrentIndex( index );
           emit ui->schemaControlListView->clicked( index );
           break;
       }
   }
}

void ConnectEditor::saveExpandedState()
{
    for(int row = 0; row < mDataModel->rowCount(); ++row)
        saveExpandedOnLevel(mDataModel->index(row,0));
}

void ConnectEditor::restoreExpandedState()
{
    ui->dataTreeView->setUpdatesEnabled(false);

    for(int row = 0; row < mDataModel->rowCount(); ++row)
        restoreExpandedOnLevel(mDataModel->index(row,0));

    ui->dataTreeView->setUpdatesEnabled(true);
}

void ConnectEditor::saveExpandedOnLevel(const QModelIndex &index)
{
    if (ui->dataTreeView->isExpanded(index)) {
        if(index.isValid())
            mExpandIDs << index.data(Qt::UserRole).toInt();
        qDebug() << index.data(Qt::UserRole).toInt();
        for(int row = 0; row < mDataModel->rowCount(index); ++row)
            saveExpandedOnLevel( mDataModel->index(row,0, index) );
    }
}

void ConnectEditor::restoreExpandedOnLevel(const QModelIndex &index)
{
    if(mExpandIDs.contains(index.data(Qt::UserRole).toInt())) {
        ui->dataTreeView->setExpanded(index, true);
        for(int row = 0; row < mDataModel->rowCount(index); ++row)
            restoreExpandedOnLevel( mDataModel->index(row,0, index) );
    }
}

void ConnectEditor::iterateModelItem(QModelIndex parent)
{
    for (int i=0; i<mDataModel->rowCount(parent); i++) {
//        QModelIndex index = mDataModel->index(i, 0, parent);
        if ( mDataModel->data( mDataModel->index(i, (int)DataItemColumn::CheckState, parent), Qt::DisplayRole).toInt()<=(int)DataCheckState::ListItem ||
             mDataModel->data( mDataModel->index(i, (int)DataItemColumn::CheckState, parent), Qt::DisplayRole).toInt()==(int)DataCheckState::MapAppend  )
            ui->dataTreeView->setFirstColumnSpanned(i, parent, true);

//        if (mDataModel->hasChildren(index)) {
//            iterateModelItem(index);
//        }
    }
}



}
}
}
