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
#include <QStandardItemModel>
#include <QBitmap>

#include "connecteditor.h"
#include "connectdatakeydelegate.h"
#include "connectdatavaluedelegate.h"
#include "connectdataactiondelegate.h"
#include "schemadefinitionmodel.h"
#include "schemalistmodel.h"
#include "headerviewproxy.h"
#include "mainwindow.h"
#include "exception.h"
#include "treecellresizer.h"
#include "ui_connecteditor.h"

namespace gams {
namespace studio {
namespace connect {

ConnectEditor::ConnectEditor(const QString& connectDataFileName, const FileId &id,  QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ConnectEditor),
    mFileId(id),
    mLocation(connectDataFileName)
{
    init(false);
}

bool ConnectEditor::init(bool quiet)
{
    try {
       mConnect = new Connect();
    } catch(Exception &) {
        ui->setupUi(this);
        connect(ui->openAsTextButton, &QPushButton::clicked, this, &ConnectEditor::openAsTextButton_clicked, Qt::UniqueConnection);
        mConnect = nullptr;
        mDataModel = nullptr;
        setModified(false);
        return false;
    }

    ui->setupUi(this);
    try {
        mDataModel = new ConnectDataModel(mLocation, mConnect, this);
    } catch(Exception& e) {
        if (!quiet) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Unable to Open File");
            msgBox.setText("Unable to open file: " + mLocation + ".\n"
                           + e.what() + ".\n"
                           + "You can reopen the file using text editor to edit the content."
                           );
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Ok);
            if (msgBox.exec() == QMessageBox::Ok) {  }
            EXCEPT() << e.what();
        }
    }

    setFocusProxy(ui->dataTreeView);

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

    ui->expandAllIcon->setPixmap(Theme::icon(":solid/triangle-down").pixmap(QSize(12, 12)));
    ui->collapseAllIcon->setPixmap(Theme::icon(":solid/triangle-right").pixmap(QSize(12, 12)));
    ui->connectHSplitter->setStretchFactor(0, 4);
    ui->connectHSplitter->setStretchFactor(1, 3);
    ui->connectHSplitter->setStretchFactor(2, 5);

    ui->dataTreeView->setModel( mDataModel );
    ConnectDataValueDelegate* valuedelegate = new ConnectDataValueDelegate(ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn(static_cast<int>(DataItemColumn::Value), valuedelegate );

    ConnectDataKeyDelegate* keydelegate = new ConnectDataKeyDelegate(mConnect, ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn( static_cast<int>(DataItemColumn::Key), keydelegate);
    ConnectDataActionDelegate* actiondelegate = new ConnectDataActionDelegate( ui->dataTreeView);
    ui->dataTreeView->setItemDelegateForColumn( static_cast<int>(DataItemColumn::Delete), actiondelegate);
    ui->dataTreeView->setItemDelegateForColumn( static_cast<int>(DataItemColumn::MoveDown), actiondelegate);
    ui->dataTreeView->setItemDelegateForColumn( static_cast<int>(DataItemColumn::MoveUp), actiondelegate);

    ui->dataTreeView->header()->hide();     // hide header
    new TreeCellResizer(ui->dataTreeView);  // resize tree cell instead of header
    ui->dataTreeView->header()->setSectionResizeMode(static_cast<int>(DataItemColumn::Key), QHeaderView::Interactive);
    ui->dataTreeView->header()->setSectionResizeMode(static_cast<int>(DataItemColumn::Value), QHeaderView::Stretch);
    ui->dataTreeView->header()->setSectionResizeMode(static_cast<int>(DataItemColumn::Delete), QHeaderView::ResizeToContents);
    ui->dataTreeView->header()->setSectionResizeMode(static_cast<int>(DataItemColumn::MoveDown), QHeaderView::ResizeToContents);
    ui->dataTreeView->header()->setSectionResizeMode(static_cast<int>(DataItemColumn::MoveUp), QHeaderView::ResizeToContents);
    ui->dataTreeView->header()->setMinimumSectionSize(16);
    ui->dataTreeView->header()->resizeSection(static_cast<int>(DataItemColumn::Delete),   16 /*width*/);
    ui->dataTreeView->header()->resizeSection(static_cast<int>(DataItemColumn::MoveDown), 16 /*width*/);
    ui->dataTreeView->header()->resizeSection(static_cast<int>(DataItemColumn::MoveUp),   16 /*width*/);
    ui->dataTreeView->header()->setStretchLastSection(false);
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
    ui->dataTreeView->expandAll();
    for (int i=0; i< ui->dataTreeView->model()->columnCount(); i++)
        ui->dataTreeView->resizeColumnToContents(i);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::CheckState), true);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::SchemaType), true);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::AllowedValue), true);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::ElementID), true);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::SchemaKey), true);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::Undefined), true);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::InvalidValue), true);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::ExcludedKeys), true);
    ui->dataTreeView->setColumnHidden( static_cast<int>(DataItemColumn::DefaultValue), true);
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
    ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::Field));
    ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::Required));
    ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::Type));
    ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::Default));
    ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::AllowedValue));
    ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::min));
    ui->helpTreeView->setColumnHidden( static_cast<int>(SchemaItemColumn::Nullable), true );
    ui->helpTreeView->setColumnHidden( static_cast<int>(SchemaItemColumn::SchemaKey), true );
    ui->helpTreeView->setColumnHidden( static_cast<int>(SchemaItemColumn::DragEnabled), true );
    ui->helpTreeView->setColumnHidden( static_cast<int>(SchemaItemColumn::min), true );
    headerRegister(ui->helpTreeView->header());

    ui->onlyRequiredAttribute->setCheckState(Qt::Unchecked);
    ui->openAsTextButton->setEnabled(true);

    connect(keydelegate, &ConnectDataKeyDelegate::requestSchemaHelp, this, &ConnectEditor::schemaHelpRequested, Qt::UniqueConnection);
    connect(keydelegate, &ConnectDataKeyDelegate::requestAppendItem, this, &ConnectEditor::appendItemRequested, Qt::UniqueConnection);
    connect(keydelegate, &ConnectDataKeyDelegate::requestInsertSchemaItem, this, &ConnectEditor::appendSchemaItemRequested, Qt::UniqueConnection);
    connect(keydelegate, &ConnectDataKeyDelegate::modificationChanged, this, &ConnectEditor::setModified, Qt::UniqueConnection);
    connect(keydelegate, &ConnectDataKeyDelegate::modificationChanged, this, [this]() { ui->dataTreeView->resizeColumnToContents(static_cast<int>(DataItemColumn::Key)); });

    connect(valuedelegate, &ConnectDataValueDelegate::modificationChanged, this, &ConnectEditor::setModified, Qt::UniqueConnection);
    connect(valuedelegate, &ConnectDataValueDelegate::modificationChanged, this, [this]() { ui->dataTreeView->resizeColumnToContents(static_cast<int>(DataItemColumn::Value)); });
    connect(actiondelegate, &ConnectDataActionDelegate::requestDeleteItem, this, &ConnectEditor::deleteDataItemRequested, Qt::UniqueConnection);
    connect(actiondelegate, &ConnectDataActionDelegate::requestMoveUpItem, this, &ConnectEditor::moveUpDatatItemRequested, Qt::UniqueConnection);
    connect(actiondelegate, &ConnectDataActionDelegate::requestMoveDownItem, this, &ConnectEditor::moveDownDatatItemRequested, Qt::UniqueConnection);

    connect(ui->schemaControlListView, &QListView::clicked, this,  [=](const QModelIndex &index) {
        defmodel->loadSchemaFromName( schemaItemModel->data( schemaItemModel->index(index.row(),0) ).toString() );
        schemaItemModel->setToolTip(index);
    });
    connect(ui->schemaControlListView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [=](const QModelIndex &current, const QModelIndex &previous) {
        Q_UNUSED(previous)
        defmodel->loadSchemaFromName( current.data(Qt::DisplayRole).toString() );
    });
    connect(schemaItemModel, &SchemaListModel::schemaItemChanged, this, [=](const QString& schemaname) {
        defmodel->loadSchemaFromName( schemaname );
    });
    connect(ui->schemaControlListView, &QListView::doubleClicked, this, &ConnectEditor::schemaDoubleClicked, Qt::UniqueConnection);
    connect(ui->openAsTextButton, &QPushButton::clicked, this, &ConnectEditor::openAsTextButton_clicked, Qt::UniqueConnection);
    connect(ui->onlyRequiredAttribute, &QCheckBox::stateChanged, mDataModel, &ConnectDataModel::onlyRequriedAttributedChanged, Qt::UniqueConnection);
    connect(ui->expandAllIcon, &ClickableLabel::clicked, this, [=]() {  ui->dataTreeView->expandAll(); });
    connect(ui->expandAllLabel, &ClickableLabel::clicked, this, [=]() {  ui->dataTreeView->expandAll(); });
    connect(ui->collapseAllLabel, &ClickableLabel::clicked, this, [=]() {  ui->dataTreeView->collapseAll(); });
    connect(ui->collapseAllIcon, &ClickableLabel::clicked, this, [=]() {  ui->dataTreeView->collapseAll(); });

    connect(mDataModel, &ConnectDataModel::modificationChanged, this, &ConnectEditor::setModified, Qt::UniqueConnection);
    connect(mDataModel, &ConnectDataModel::fromSchemaInserted, this, &ConnectEditor::fromSchemaInserted, Qt::UniqueConnection);
    connect(mDataModel, &ConnectDataModel::indexExpandedAndResized, this, &ConnectEditor::expandAndResizedToContents, Qt::UniqueConnection);
    connect(mDataModel, &ConnectDataModel::dataChanged, mDataModel, &ConnectDataModel::onEditDataChanged, Qt::UniqueConnection);

    connect(mDataModel, &ConnectDataModel::rowsAboutToBeInserted, this, [this]() { saveExpandedState(); });
    connect(mDataModel, &ConnectDataModel::rowsAboutToBeMoved   , this, [this]() { saveExpandedState(); });
    connect(mDataModel, &ConnectDataModel::rowsAboutToBeRemoved , this, [this]() { saveExpandedState(); });
    connect(mDataModel, &ConnectDataModel::rowsInserted, this, [this]() { restoreExpandedState();  });
    connect(mDataModel, &ConnectDataModel::rowsRemoved , this, [this]() { restoreExpandedState();  });
    connect(mDataModel, &ConnectDataModel::rowsMoved   , this, [this]() { restoreExpandedState();  });
    connect(mDataModel, &ConnectDataModel::modelReset  , this, [this]() {
        ui->dataTreeView->expandRecursively(mDataModel->index(0,0));
        ui->dataTreeView->setRootIndex( mDataModel->index(0,0, QModelIndex()) );    // hide root
        ui->dataTreeView->resizeColumnToContents(static_cast<int>(DataItemColumn::Key));
        ui->dataTreeView->resizeColumnToContents(static_cast<int>(DataItemColumn::Value));
    });

    connect(defmodel, &SchemaDefinitionModel::modelReset, this, [this]() {
        ui->helpTreeView->expandAll();
        ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::Field));
        ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::Required));
        ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::Type));
        ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::Default));
        ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::AllowedValue));
        ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::min));
        ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::SchemaKey));
        ui->helpTreeView->resizeColumnToContents(static_cast<int>(SchemaItemColumn::DragEnabled));
    });

    setModified(false);

    ui->dataTreeView->clearSelection();
    ui->helpTreeView->clearSelection();
    return true;
}

ConnectEditor::~ConnectEditor()
{
    delete ui;
    mExpandIDs.clear();
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
    bool success = false;
    ConnectData* data = mDataModel->getConnectData();
    if (data) {
       setModified(false);
       data->unload(location);
       delete data;
       data = NULL;
       success = true;
    }
    return success;
}

bool ConnectEditor::isModified() const
{
    return mModified;
}

void ConnectEditor::setModified(bool modified)
{
    mModified = modified;
    ui->openAsTextButton->setEnabled(!modified);
    emit modificationChanged( mModified );
}

bool ConnectEditor::saveConnectFile(const QString &location)
{
    return saveAs(location);
}

void ConnectEditor::openAsTextButton_clicked(bool checked)
{
    Q_UNUSED(checked)
    MainWindow* main = getMainWindow();
    if (!main) return;

    emit main->projectRepo()->closeFileEditors(fileId());

    FileMeta* fileMeta = main->fileRepo()->fileMeta(fileId());
    PExFileNode* fileNode = main->projectRepo()->findFileNode(this);
    PExProjectNode* project = (fileNode ? fileNode->assignedProject() : nullptr);

    emit main->projectRepo()->openFile(fileMeta, true, project, -1, true);
}

void ConnectEditor::fromSchemaInserted(const QString &schemaname, int position)
{
    setModified(true);
    mDataModel->addFromSchema( schemaname, position );
}

void ConnectEditor::schemaDoubleClicked(const QModelIndex &modelIndex)
{
    const QString schemaname = ui->schemaControlListView->model()->data( modelIndex ).toString();

    emit mDataModel->fromSchemaInserted(schemaname, mDataModel->rowCount(mDataModel->index(0,0)) );
}

void ConnectEditor::expandAndResizedToContents(const QModelIndex &idx)
{
    if (!idx.isValid())
        return;
    if (idx.parent()!=ui->dataTreeView->model()->index(0,0, QModelIndex()) &&
        !ui->dataTreeView->isExpanded(idx.parent())) {
          ui->dataTreeView->expandRecursively( idx.parent() );
    } else {
        ui->dataTreeView->expandRecursively( idx );
    }
    for (int i=0; i< ui->dataTreeView->model()->columnCount(); i++)
        ui->dataTreeView->resizeColumnToContents(i);
    ui->dataTreeView->scrollTo(idx);
    saveExpandedState();
}

void ConnectEditor::schemaHelpRequested(const QString &schemaName)
{
   for(int row=0; row<mConnect->getSchemaNames().size(); row++) {
       QString str = mConnect->getSchemaNames().at(row);
       if (str.compare(schemaName, Qt::CaseInsensitive)==0) {
           const QModelIndex index = ui->schemaControlListView->model()->index(row, 0);
           ui->schemaControlListView->setCurrentIndex( index );
           emit ui->schemaControlListView->clicked( index );
           break;
       }
   }
}

void ConnectEditor::appendItemRequested(const QModelIndex &index)
{
    setModified(true);

    const QModelIndex checkstate_idx = index.sibling(index.row(), static_cast<int>(DataItemColumn::CheckState));
    if (static_cast<int>(DataCheckState::ListAppend)==checkstate_idx.data(Qt::DisplayRole).toInt()) {
        if (index.parent().isValid() &&
            index.parent().siblingAtColumn(static_cast<int>(DataItemColumn::Undefined)).data(Qt::DisplayRole).toBool())
            return;
        const QModelIndex values_idx = index.sibling(index.row(), static_cast<int>(DataItemColumn::SchemaKey));
        QStringList schema = values_idx.data(Qt::DisplayRole).toString().split(":");
        if ( !schema.isEmpty() ) {
            const QString schemaname = schema.at(0);
            schema.removeFirst();
            if (schema.last().compare("-")==0)
                schema.removeLast();
            ConnectData* schemadata = mConnect->createDataHolderFromSchema(schemaname, schema, (ui->onlyRequiredAttribute->checkState()==Qt::Checked), true);
            mDataModel->appendListElement(schemaname, schema, schemadata, index);
        }
    } else if (static_cast<int>(DataCheckState::MapAppend)==checkstate_idx.data(Qt::DisplayRole).toInt()) {
               mDataModel->appendMapElement(index);
    }
}

void ConnectEditor::appendSchemaItemRequested(const int schemaNumber, const QModelIndex &index)
{
    const QModelIndex checkstate_idx = index.sibling(index.row(), static_cast<int>(DataItemColumn::CheckState));
    if (static_cast<int>(DataCheckState::SchemaAppend)!=checkstate_idx.data(Qt::DisplayRole).toInt())
        return;

    if (index.parent().isValid() &&
        index.parent().siblingAtColumn(static_cast<int>(DataItemColumn::Undefined)).data(Qt::DisplayRole).toBool())
        return;

    const QModelIndex values_idx = index.parent().siblingAtColumn( static_cast<int>(DataItemColumn::SchemaKey) );
    QStringList schema = values_idx.data().toString().split(":");
    QString schemaname = "";
    schema << "["+QString::number(schemaNumber)+"]";
    ConnectData* data = mConnect->createDataHolderFromSchema(schema , (ui->onlyRequiredAttribute->checkState()==Qt::Checked), true);
    if (!schema.isEmpty()) {
        schemaname = schema.first();
        schema.removeFirst();
    }
    if (data->getRootNode().Type()==YAML::NodeType::Sequence) {
        mDataModel->insertLastListElement(schemaname, schema, data, index.parent());
    }
}

void ConnectEditor::deleteDataItemRequested(const QModelIndex &index)
{
    setModified(true);

    ui->dataTreeView->setUpdatesEnabled(false);
    mDataModel->removeItem(index);
    ui->dataTreeView->setUpdatesEnabled(true);
}

void ConnectEditor::moveUpDatatItemRequested(const QModelIndex &index)
{
    setModified(true);

    ui->dataTreeView->setUpdatesEnabled(false);
    mDataModel->moveRows( index.parent(), index.row(), 1,
                          index.parent(), index.row()-1 );
    ui->dataTreeView->setUpdatesEnabled(true);
}

void ConnectEditor::moveDownDatatItemRequested(const QModelIndex &index)
{
    setModified(true);

    ui->dataTreeView->setUpdatesEnabled(false);
    mDataModel->moveRows( index.parent(), index.row(), 1,
                          index.parent(), index.row()+2 );
    ui->dataTreeView->setUpdatesEnabled(true);
}

void ConnectEditor::on_reloadConnectFile()
{
    try {
       mDataModel->reloadConnectDataModel();
       setModified(false);
    } catch(Exception& e) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Unable to Reload File");
            msgBox.setText("Unable to reload the contents from file : " + mLocation + ".\n"
                           + e.what() + ".\n"
                           + "Contnue editing contents without reloading the file contents may cause data loss or conflict.\n"
                           + "You can reopen the file using text editor to edit the contents."
                           );
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Ok);
            if (msgBox.exec() == QMessageBox::Ok) {  }
    }
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
        if (index.isValid() && !mExpandIDs.contains(index.data(Qt::UserRole).toInt())) {
            ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
            if (item->childCount() > 0)
                mExpandIDs << index.data(Qt::UserRole).toInt();
        }
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
    } else {
        ui->dataTreeView->setExpanded(index, false);
    }
}

MainWindow *ConnectEditor::getMainWindow()
{
    const QWidgetList widgets = qApp->topLevelWidgets();
    for (QWidget *widget : widgets)
        if (MainWindow *mainWindow = qobject_cast<MainWindow*>(widget))
            return mainWindow;
    return nullptr;
}

}
}
}
