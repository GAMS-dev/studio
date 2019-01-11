/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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

    mSymbolTableModel = new SymbolTableModel(mReference, mType, this);

    mSymbolTableProxyModel= new QSortFilterProxyModel(this);
    mSymbolTableProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mSymbolTableProxyModel->setSourceModel( mSymbolTableModel );
    if (mType == SymbolDataType::File)
        mSymbolTableProxyModel->setFilterKeyColumn(0);
    else
        mSymbolTableProxyModel->setFilterKeyColumn(1);
    mSymbolTableProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->symbolView->setModel( mSymbolTableProxyModel );
    ui->symbolView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->symbolView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->symbolView->sortByColumn(0, Qt::AscendingOrder);
    ui->symbolView->setSortingEnabled(true);
    ui->symbolView->resizeColumnsToContents();
    ui->symbolView->setAlternatingRowColors(true);

    ui->symbolView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->symbolView, &QAbstractItemView::doubleClicked, this, &SymbolReferenceWidget::jumpToFile);
    connect(ui->symbolView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SymbolReferenceWidget::updateSelectedSymbol);
    connect(ui->symbolSearchLineEdit, &QLineEdit::textChanged, mSymbolTableProxyModel, &QSortFilterProxyModel::setFilterWildcard);
    connect(ui->allColumnToggleSearch, &QCheckBox::toggled, this, &SymbolReferenceWidget::toggleSearchColumns);

    mReferenceTreeProxyModel = new QSortFilterProxyModel(this);
    mReferenceTreeModel =  new ReferenceTreeModel(mReference, this);

    mReferenceTreeProxyModel->setFilterKeyColumn(-1);
    mReferenceTreeProxyModel->setSourceModel( mReferenceTreeModel );
    mReferenceTreeProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mReferenceTreeProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    ui->referenceView->setModel( mReferenceTreeProxyModel );
    ui->referenceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->referenceView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->referenceView->setItemsExpandable(true);
    ui->referenceView->expandAll();
    ui->referenceView->resizeColumnToContents(0);
    ui->referenceView->resizeColumnToContents(1);
    ui->referenceView->setAlternatingRowColors(true);
    ui->referenceView->setColumnHidden(3, true);

    connect(ui->referenceView, &QAbstractItemView::doubleClicked, this, &SymbolReferenceWidget::jumpToReferenceItem);
    connect( mReferenceTreeModel, &ReferenceTreeModel::modelReset, this, &SymbolReferenceWidget::expandResetModel);
}

SymbolReferenceWidget::~SymbolReferenceWidget()
{
    delete ui;
}

void SymbolReferenceWidget::toggleSearchColumns(bool checked)
{
    if (checked) {
        mSymbolTableProxyModel->setFilterKeyColumn(-1);
    } else {
        if (mType == SymbolDataType::File)
            mSymbolTableProxyModel->setFilterKeyColumn(0);
        else
            mSymbolTableProxyModel->setFilterKeyColumn(1);
    }
}

void SymbolReferenceWidget::updateSelectedSymbol(QItemSelection selected, QItemSelection deselected)
{
    Q_UNUSED(deselected);
    if (selected.indexes().size() > 0) {
        if (mType == SymbolDataType::FileUsed) {
            return;
        }

        SymbolId id = selected.indexes().at(0).data().toInt();
        mReferenceTreeModel->updateSelectedSymbol(id);
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

} // namespace reference
} // namespace studio
} // namespace gams
