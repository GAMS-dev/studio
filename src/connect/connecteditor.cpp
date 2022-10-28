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
#include "schemalistmodel.h"
#include "headerviewproxy.h"
#include "ui_connecteditor.h"

namespace gams {
namespace studio {
namespace connect {

ConnectEditor::ConnectEditor(const QString& connectDataFileName,
                             FileId id,  QTextCodec* codec,  QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ConnectEditor),
    mFileId(id),
    mCodec(codec),
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

    SchemaListModel* schemaItemModel = new SchemaListModel( mConnect->getSchemaNames(), this );
    ui->schemaControlListView->setModel(schemaItemModel);

    ui->schemaControlListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->schemaControlListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->schemaControlListView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->schemaControlListView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ui->schemaControlListView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    ui->schemaControlListView->setViewMode(QListView::ListMode);
    ui->schemaControlListView->setIconSize(QSize(14,14));
    ui->schemaControlListView->setAutoFillBackground(false);
    ui->schemaControlListView->setBackgroundRole(QPalette::NoRole);
    ui->schemaControlListView->viewport()->setAutoFillBackground(true);
    ui->schemaControlListView->setDragEnabled(true);
    ui->schemaControlListView->setDropIndicatorShown(true);
//    QPalette palette = ui->schemaControlListView->viewport()->palette();
//    palette.setColor(QPalette::Background, Qt::gray);
//    ui->schemaControlListView->viewport()->setPalette(palette);
    ui->schemaControlListView->setCurrentIndex(schemaItemModel->index(0,0));

    ui->connectHSplitter->setStretchFactor(0, 4);
    ui->connectHSplitter->setStretchFactor(1, 3);
    ui->connectHSplitter->setStretchFactor(2, 5);

    mDataModel = new ConnectDataModel(mLocation, mConnect, this);
    ui->dataTreeView->setModel( mDataModel );

    ConnectDataValueDelegate* valuedelegate = new ConnectDataValueDelegate(ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn(1, valuedelegate );

    ConnectDataKeyDelegate* keydelegate = new ConnectDataKeyDelegate( ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::Key, keydelegate);
    ConnectDataActionDelegate* actiondelegate = new ConnectDataActionDelegate( ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::Delete, actiondelegate);
    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::MoveDown, actiondelegate);
    ui->dataTreeView->setItemDelegateForColumn( (int)DataItemColumn::MoveUp, actiondelegate);

//    ui->dataTreeView->header()->hide();
    ui->dataTreeView->header()->setSectionResizeMode((int)DataItemColumn::Key, QHeaderView::Interactive);
    ui->dataTreeView->header()->setSectionResizeMode((int)DataItemColumn::Value, QHeaderView::Interactive);
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
    ui->dataTreeView->viewport()->setAcceptDrops(true);
    ui->dataTreeView->setDropIndicatorShown(true);
    ui->dataTreeView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->dataTreeView->setDefaultDropAction(Qt::CopyAction);
//    updateDataColumnSpan();
    ui->dataTreeView->expandAll();
    for (int i=0; i< ui->dataTreeView->model()->columnCount(); i++)
        ui->dataTreeView->resizeColumnToContents(i);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::CheckState, true);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::SchemaType, true);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::AllowedValue, true);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::ElementID, true);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::SchemaKey, true);
    ui->dataTreeView->setColumnHidden( (int)DataItemColumn::Unknown, true);
    ui->dataTreeView->setRootIndex( mDataModel->index(0,0, QModelIndex()) );    // hide root
    headerRegister(ui->dataTreeView->header());

    SchemaDefinitionModel* defmodel = new SchemaDefinitionModel(mConnect, mConnect->getSchemaNames().at(0), this);
    ui->helpTreeView->setModel( defmodel );
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->helpTreeView->header()->setStyle(HeaderViewProxy::instance());
    ui->helpTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->helpTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->helpTreeView->setItemsExpandable(true);
    ui->helpTreeView->setDragEnabled(true);
    ui->helpTreeView->setDragDropMode(QAbstractItemView::DragOnly);
    ui->helpTreeView->expandAll();
    ui->helpTreeView->resizeColumnToContents((int)SchemaItemColumn::Field);
    ui->helpTreeView->resizeColumnToContents((int)SchemaItemColumn::Required);
    ui->helpTreeView->resizeColumnToContents((int)SchemaItemColumn::Type);
    ui->helpTreeView->resizeColumnToContents((int)SchemaItemColumn::Default);
    ui->helpTreeView->resizeColumnToContents((int)SchemaItemColumn::AllowedValue);
    ui->helpTreeView->resizeColumnToContents((int)SchemaItemColumn::min);
    ui->helpTreeView->setColumnHidden( (int)SchemaItemColumn::SchemaKey, true );
    ui->helpTreeView->setColumnHidden( (int)SchemaItemColumn::DragEnabled, true );
    headerRegister(ui->helpTreeView->header());

    connect(keydelegate, &ConnectDataKeyDelegate::requestSchemaHelp, this, &ConnectEditor::schemaHelpRequested, Qt::UniqueConnection);
    connect(keydelegate, &ConnectDataKeyDelegate::requestAppendItem, this, &ConnectEditor::appendItemRequested, Qt::UniqueConnection);
    connect(keydelegate, &ConnectDataKeyDelegate::modificationChanged, this, &ConnectEditor::setModified, Qt::UniqueConnection);

    connect(valuedelegate, &ConnectDataValueDelegate::modificationChanged, this, &ConnectEditor::setModified, Qt::UniqueConnection);
    connect(actiondelegate, &ConnectDataActionDelegate::requestDeleteItem, this, &ConnectEditor::deleteDataItemRequested, Qt::UniqueConnection);
    connect(actiondelegate, &ConnectDataActionDelegate::requestMoveUpItem, this, &ConnectEditor::moveUpDatatItemRequested, Qt::UniqueConnection);
    connect(actiondelegate, &ConnectDataActionDelegate::requestMoveDownItem, this, &ConnectEditor::moveDownDatatItemRequested, Qt::UniqueConnection);

    connect(ui->schemaControlListView, &QListView::clicked,  [=](const QModelIndex &index) {
        defmodel->loadSchemaFromName( schemaItemModel->data( schemaItemModel->index(index.row(),0) ).toString() );
        schemaItemModel->setToolTip(index);
    });
    connect(schemaItemModel, &SchemaListModel::schemaItemChanged, this, [=](const QString& schemaname) {
        defmodel->loadSchemaFromName( schemaname );
    });
    connect(ui->schemaControlListView, &QListView::doubleClicked, this, &ConnectEditor::schemaDoubleClicked, Qt::UniqueConnection);

    connect(mDataModel, &ConnectDataModel::modificationChanged, this, &ConnectEditor::setModified, Qt::UniqueConnection);
    connect(mDataModel, &ConnectDataModel::fromSchemaInserted, this, &ConnectEditor::fromSchemaInserted, Qt::UniqueConnection);
    connect(mDataModel, &ConnectDataModel::indexExpandedAndResized, this, &ConnectEditor::expandAndResizedToContents, Qt::UniqueConnection);

    connect(mDataModel, &ConnectDataModel::rowsAboutToBeInserted, [this]() { saveExpandedState(); });
    connect(mDataModel, &ConnectDataModel::rowsAboutToBeMoved   , [this]() { saveExpandedState(); });
    connect(mDataModel, &ConnectDataModel::rowsAboutToBeRemoved , [this]() { saveExpandedState(); });
    connect(mDataModel, &ConnectDataModel::rowsInserted, [this]() { restoreExpandedState();  });
    connect(mDataModel, &ConnectDataModel::rowsRemoved , [this]() { restoreExpandedState();  });
    connect(mDataModel, &ConnectDataModel::rowsMoved   , [this]() { restoreExpandedState();  });
    connect(mDataModel, &ConnectDataModel::modelReset, [this]() {
        restoreExpandedState();
    });

    connect(defmodel, &SchemaDefinitionModel::modelReset, [this]() {
        ui->helpTreeView->expandAll();
        ui->helpTreeView->resizeColumnToContents(0);
        ui->helpTreeView->resizeColumnToContents(1);
        ui->helpTreeView->resizeColumnToContents(2);
        ui->helpTreeView->resizeColumnToContents(3);
        ui->helpTreeView->resizeColumnToContents(4);
    });

    setModified(false);

    ui->dataTreeView->clearSelection();
    ui->helpTreeView->clearSelection();
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

FileId ConnectEditor::fileId() const
{
    return mFileId;
}

bool ConnectEditor::saveAs(const QString &location)
{
    setModified(false);
    bool successs = false;
    ConnectData* data = mDataModel->getConnectData();
    qDebug()<< data->str().c_str();
    data->unload(location);
    return successs;
}

bool ConnectEditor::isModified() const
{
    return mModified;
}

void ConnectEditor::setModified(bool modified)
{
    mModified = modified;
    qDebug() << "modified:" << mLocation << ":" << (mModified?"true":"false");
    emit modificationChanged( mModified );
}

bool ConnectEditor::saveConnectFile(const QString &location)
{
    return saveAs(location);
}

void ConnectEditor::fromSchemaInserted(const QString &schemaname, int position)
{
    setModified(true);
    QStringList strlist;
    strlist << schemaname;
    mDataModel->addFromSchema( mConnect->createDataHolder(strlist), position );
}

void ConnectEditor::schemaDoubleClicked(const QModelIndex &modelIndex)
{
    QString schemaname = ui->schemaControlListView->model()->data( modelIndex ).toString();

    emit mDataModel->fromSchemaInserted(schemaname, mDataModel->rowCount(mDataModel->index(0,0)) );
}

void ConnectEditor::updateDataColumnSpan(const QModelIndex &modelIndex)
{
    Q_UNUSED(modelIndex);
    qDebug() << "updateColumnSpan " << mDataModel->rowCount();
    iterateModelItem( ui->dataTreeView->rootIndex());
}

void ConnectEditor::expandAndResizedToContents(const QModelIndex &index)
{
    qDebug() << "expandAndResizedToContents: (" << index.row() << "," << index.column()
             <<") parent:(" << index.parent().row() << "," << index.parent().column() << ")";
    qDebug() << "expandAndResizedToContents: valid index:" << (index.isValid()?"Y":"N");
    ui->dataTreeView->expandRecursively( index );
    for (int i=0; i< ui->dataTreeView->model()->columnCount(); i++)
        ui->dataTreeView->resizeColumnToContents(i);
    ui->dataTreeView->scrollTo(index);
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

void ConnectEditor::appendItemRequested(const QModelIndex &index)
{
    setModified(true);

    qDebug() << "append item (" << index.row() <<"," << index.column() << ")";
    QModelIndex checkstate_idx = index.sibling(index.row(), (int)DataItemColumn::CheckState);
    if ((int)DataCheckState::ListAppend==checkstate_idx.data(Qt::DisplayRole).toInt()) {
        QModelIndex values_idx = index.sibling(index.row(), (int)DataItemColumn::AllowedValue);
        QStringList schema = values_idx.data().toStringList();
        if ( !schema.isEmpty() ) {
            QString schemaname = schema.at(0);
            schema.removeFirst();
            if (schema.last().compare("-")==0)
                schema.removeLast();
            ConnectData* schemadata = mConnect->createDataHolderFromSchema(schemaname, schema);
            qDebug() << schemadata->str().c_str();
            mDataModel->appendListElement(schemaname, schema, schemadata, index);
        }
    } else if ((int)DataCheckState::MapAppend==checkstate_idx.data(Qt::DisplayRole).toInt()) {
              mDataModel->appendMapElement(index);
    }
    expandAndResizedToContents( index );
}

void ConnectEditor::deleteDataItemRequested(const QModelIndex &index)
{
    setModified(true);

    ui->dataTreeView->setUpdatesEnabled(false);
    qDebug() << "delete (" << index.row() <<"," << index.column() << ") and all its children";
    mDataModel->removeItem(index);
    ui->dataTreeView->setUpdatesEnabled(true);
}

void ConnectEditor::moveUpDatatItemRequested(const QModelIndex &index)
{
    setModified(true);

    ui->dataTreeView->setUpdatesEnabled(false);
    qDebug() << "move " << index.row() <<"," << index.column() << ") and all its children up";
    mDataModel->moveRows( index.parent(), index.row(), 1,
                          index.parent(), index.row()-1 );
    ui->dataTreeView->setUpdatesEnabled(true);
}

void ConnectEditor::moveDownDatatItemRequested(const QModelIndex &index)
{
    setModified(true);

    ui->dataTreeView->setUpdatesEnabled(false);
    qDebug() << "move (" << index.row() <<"," << index.column() << ") and all its children down";
    mDataModel->moveRows( index.parent(), index.row(), 1,
                          index.parent(), index.row()+2 );
    ui->dataTreeView->setUpdatesEnabled(true);
}

void ConnectEditor::saveExpandedState()
{
    mExpandIDs.clear();
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
        if(index.isValid() && !mExpandIDs.contains(index.data(Qt::UserRole).toInt()))
            mExpandIDs << index.data(Qt::UserRole).toInt();
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
