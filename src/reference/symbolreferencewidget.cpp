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
#include "symbolreferencemodel.h"
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

    SymbolReferenceModel* refModel = new SymbolReferenceModel(mReference, mType, this);
    ui->symbolView->setModel( refModel );
    ui->symbolView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->symbolView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->symbolView->setAutoScroll(true);
    ui->symbolView->setSortingEnabled(true);
    ui->symbolView->sortByColumn(0, Qt::AscendingOrder);
    ui->symbolView->setAlternatingRowColors(true);

    ui->symbolView->horizontalHeader()->setStretchLastSection(true);
    ui->symbolView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


}

SymbolReferenceWidget::~SymbolReferenceWidget()
{
    delete ui;
}

} // namespace studio
} // namespace gams
