/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "symbolreferencewidget.h"
#include "ui_symbolreferencewidget.h"
#include "sortedfileheaderview.h"
#include "filterlineedit.h"

namespace gams {
namespace studio {
namespace reference {

SymbolReferenceWidget::SymbolReferenceWidget(Reference* ref, SymbolDataType::SymbolType type, ReferenceViewer *parent) :
    ui(new Ui::SymbolReferenceWidget),
    mReference(ref),
    mType(type),
    mReferenceViewer(parent)
{
    ui->setupUi(this);
    ui->symbolReferenceSplitter->setSizes(QList<int>({2500,1500}));

    mSymbolTableModel = new SymbolTableModel(mType, this);
    ui->symbolView->setModel( mSymbolTableModel );

    ui->symbolView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->symbolView->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->symbolView->setAlternatingRowColors(true);
    ui->symbolView->setContextMenuPolicy(Qt::CustomContextMenu);

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->symbolView->horizontalHeader()->setStyle(HeaderViewProxy::instance());
    ui->symbolView->horizontalHeader()->setStretchLastSection(true);
    ui->symbolView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->symbolView->horizontalHeader()->setSectionResizeMode( mSymbolTableModel->getLastSectionIndex(), QHeaderView::Stretch );
    ui->symbolView->verticalHeader()->setMinimumSectionSize(1);
    ui->symbolView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    ui->symbolView->horizontalHeader()-> setContextMenuPolicy(Qt::CustomContextMenu);

    if (mType != SymbolDataType::SymbolType::FileUsed) {
        ui->symbolView->sortByColumn(1, Qt::AscendingOrder);
        ui->symbolView->setSortingEnabled(true);
        ui->symbolSearchLineEdit->setKeyColumn(1);
    } else {
        ui->symbolView->setHorizontalHeader(new SortedFileHeaderView(Qt::Horizontal, ui->symbolView));
        if (HeaderViewProxy::platformShouldDrawBorder())
            ui->symbolView->horizontalHeader()->setStyle(HeaderViewProxy::instance());
    }
    QAction* resizeColumn = QWidget::addAction("Auto Resize Columns", QKeySequence("Ctrl+R"), this, [this]() { resizeColumnToContents(); });
    resizeColumn->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    resizeColumn->setShortcutVisibleInContextMenu(true);
    ui->symbolView->addAction(resizeColumn);

    connect(mSymbolTableModel, &SymbolTableModel::symbolSelectionToBeUpdated, this, &SymbolReferenceWidget::updateSymbolSelection);
    connect(ui->symbolView, &QAbstractItemView::doubleClicked, this, &SymbolReferenceWidget::jumpToFile);
    connect(ui->symbolView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SymbolReferenceWidget::updateSelectedSymbol);
    connect(ui->symbolView, &QTableView::customContextMenuRequested, this, &SymbolReferenceWidget::showContextMenu);
    connect(ui->symbolSearchLineEdit, &FilterLineEdit::regExpChanged, mSymbolTableModel, &SymbolTableModel::setFilterPattern);
    connect(ui->symbolSearchLineEdit, &FilterLineEdit::columnScopeChanged, mSymbolTableModel, [this]() {
        mSymbolTableModel->toggleSearchColumns(ui->symbolSearchLineEdit->effectiveKeyColumn() < 0);
    });

    mReferenceTreeModel =  new ReferenceTreeModel(mReference, this);
    ui->referenceView->setModel( mReferenceTreeModel );

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->referenceView->header()->setStyle(HeaderViewProxy::instance());
    ui->referenceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->referenceView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->referenceView->setItemsExpandable(true);
    ui->referenceView->setAlternatingRowColors(true);
    ui->referenceView->setColumnHidden(3, true);
    expandResetModel();

    connect(ui->referenceView, &QAbstractItemView::doubleClicked, this, &SymbolReferenceWidget::jumpToReferenceItem);
    connect( mReferenceTreeModel, &ReferenceTreeModel::modelReset, this, &SymbolReferenceWidget::expandResetModel);
}

SymbolReferenceWidget::~SymbolReferenceWidget()
{
    delete ui;
    delete mSymbolTableModel;
    delete mReferenceTreeModel;
}

void SymbolReferenceWidget::selectSearchField() const
{
    ui->symbolSearchLineEdit->setFocus();
}

QList<QHeaderView *> SymbolReferenceWidget::headers()
{
    return QList<QHeaderView *>() << ui->symbolView->horizontalHeader()
                                  << ui->referenceView->header()
                                  << ui->symbolView->verticalHeader();
}

bool SymbolReferenceWidget::isModelLoaded() const
{
    return mSymbolTableModel->isModelLoaded();
}

void SymbolReferenceWidget::updateSelectedSymbol(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    if (selected.indexes().size() > 0) {
        if (mType == SymbolDataType::FileUsed) {
            mCurrentSymbolID = -1;
            QModelIndex idx = mSymbolTableModel->index(selected.indexes().at(0).row(), SymbolTableModel::COLUMN_FILEUSED_NAME);
            mCurrentSymbolSelection = mSymbolTableModel->data( idx ).toString();
            mReferenceTreeModel->resetModel();
        } else {
            QModelIndex idx = mSymbolTableModel->index(selected.indexes().at(0).row(), SymbolTableModel::COLUMN_SYMBOL_ID);
            mCurrentSymbolID = mSymbolTableModel->data( idx ).toInt();
            idx = mSymbolTableModel->index(selected.indexes().at(0).row(), SymbolTableModel::COLUMN_SYMBOL_NAME);
            mCurrentSymbolSelection = mSymbolTableModel->data( idx ).toString();
            mReferenceTreeModel->updateSelectedSymbol( mCurrentSymbolSelection );
        }

    }
}

void SymbolReferenceWidget::expandResetModel()
{
    ui->referenceView->expandAll();
    ui->referenceView->resizeColumnToContents(0);
    ui->referenceView->resizeColumnToContents(1);
}

void SymbolReferenceWidget::resetModel()
{
    mSymbolTableModel->resetModel();
    mReferenceTreeModel->resetModel();

    if (mCurrentSymbolSelection.isEmpty() && mCurrentSymbolID==-1)
        return;

    QModelIndex idx = (mType==SymbolDataType::FileUsed ?
        mSymbolTableModel->index(0, SymbolTableModel::COLUMN_FILEUSED_NAME):
        mSymbolTableModel->index(0, SymbolTableModel::COLUMN_SYMBOL_NAME) );
    QModelIndexList items = mSymbolTableModel->match(idx, Qt::DisplayRole,
                                                     mCurrentSymbolSelection, 1,
                                                     Qt::MatchExactly);
    for(QModelIndex itemIdx : std::as_const(items)) {
        ui->symbolView->selectionModel()->select(
                    QItemSelection(
                        mSymbolTableModel->index(itemIdx.row(),0),
                        mSymbolTableModel->index(itemIdx.row(),mSymbolTableModel->columnCount()- 1)),
                    QItemSelectionModel::ClearAndSelect);
    }

}

void SymbolReferenceWidget::initModel()
{
    if (!mSymbolTableModel->isModelLoaded()) {
        mSymbolTableModel->initModel(mReference);
        resizeColumnToContents();
    }

    mReferenceTreeModel->resetModel();
}

void SymbolReferenceWidget::initModel(Reference *ref)
{
    if (!ref)
       return;
    mReference = ref;
    initModel();
}

void SymbolReferenceWidget::jumpToFile(const QModelIndex &index)
{
    if (mType == SymbolDataType::FileUsed) {
        ReferenceItem item(-1, ReferenceDataType::Unknown, ui->symbolView->model()->data(index.sibling(index.row(), 0)).toString(), 0, 0);
        emit mReferenceViewer->jumpTo( item );
    }
}

void SymbolReferenceWidget::jumpToReferenceItem(const QModelIndex &index)
{
    QModelIndex  parentIndex =  ui->referenceView->model()->parent(index);
    if (parentIndex.row() >= 0) {
        QVariant location = ui->referenceView->model()->data(index.sibling(index.row(), 0), Qt::UserRole);
        QVariant lineNumber = ui->referenceView->model()->data(index.sibling(index.row(), 1), Qt::UserRole);
        QVariant colNumber = ui->referenceView->model()->data(index.sibling(index.row(), 2), Qt::UserRole);
        QVariant typeName = ui->referenceView->model()->data(index.sibling(index.row(), 3), Qt::UserRole);
        ReferenceItem item(-1, ReferenceDataType::typeFrom(typeName.toString()), location.toString(), lineNumber.toInt(), colNumber.toInt());
        emit mReferenceViewer->jumpTo( item );
    }
}

void SymbolReferenceWidget::updateSymbolSelection()
{
    int updatedSelectedRow = mSymbolTableModel->getSortedIndexOf( mCurrentSymbolSelection );
    ui->symbolView->selectionModel()->clearCurrentIndex();
    ui->symbolView->selectionModel()->clearSelection();
    if (updatedSelectedRow >= 0 && updatedSelectedRow < ui->symbolView->model()->rowCount()) {
        QModelIndex topLeftIdx = mSymbolTableModel->index( updatedSelectedRow, 0 );
        QModelIndex bottomRightIdx = mSymbolTableModel->index( updatedSelectedRow, mSymbolTableModel->columnCount()-1 );
        ui->symbolView->selectionModel()->select( QItemSelection(topLeftIdx, bottomRightIdx), QItemSelectionModel::Select);
    } else {
         mReferenceTreeModel->resetModel();
    }
}

void SymbolReferenceWidget::resizeColumnToContents()
{
    ui->symbolView->resizeColumnsToContents();
    ui->symbolView->horizontalHeader()->setSectionResizeMode( mSymbolTableModel->getLastSectionIndex(), QHeaderView::Stretch );
}

void SymbolReferenceWidget::showContextMenu(QPoint p)
{
    QModelIndexList indexSelection = ui->symbolView->selectionModel()->selectedIndexes();
    if (indexSelection.size()<=0)
        return;

    if (mSymbolTableModel->rowCount()>0 && ui->symbolView->horizontalHeader()->logicalIndexAt(p)>=0) {
        QMenu m(this);
        const auto actions = ui->symbolView->actions();
        for(QAction* action : actions) {
            action->setEnabled(true);
            m.addAction(action);
        }
        m.exec(ui->symbolView->viewport()->mapToGlobal(p));
    }
}

} // namespace reference
} // namespace studio
} // namespace gams
