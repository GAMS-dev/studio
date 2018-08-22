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

#include <QDebug>

#include "symbolreferencewidget.h"
#include "ui_symbolreferencewidget.h"

namespace gams {
namespace studio {

SymbolReferenceWidget::SymbolReferenceWidget(Reference* ref, SymbolDataType::SymbolType type, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SymbolReferenceWidget),
    mReference(ref),
    mType(type)
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
    ui->symbolView->setSortingEnabled(true);
    ui->symbolView->sortByColumn(0, Qt::AscendingOrder);
    ui->symbolView->resizeColumnsToContents();
    ui->symbolView->setAlternatingRowColors(true);

    ui->symbolView->horizontalHeader()->setStretchLastSection(true);
//    ui->symbolView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

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
    ui->referenceView->setItemsExpandable(true);
    ui->referenceView->expandAll();
    ui->referenceView->resizeColumnToContents(0);
    ui->referenceView->resizeColumnToContents(1);
    ui->referenceView->setAlternatingRowColors(true);
    ui->referenceView->setColumnHidden(3, true);

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
        if (mType == SymbolDataType::FileUsed) // TODO
            return;

        SymbolId id = selected.indexes().at(0).data().toInt();
        emit mReferenceTreeModel->updateSelectedSymbol(id);
    }
}

void SymbolReferenceWidget::expandResetModel()
{
    ui->referenceView->expandAll();
    ui->referenceView->resizeColumnToContents(0);
    ui->referenceView->resizeColumnToContents(1);
}

} // namespace studio
} // namespace gams
