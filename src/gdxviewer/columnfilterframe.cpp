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
#include "columnfilterframe.h"
#include "gdxsymbol.h"

#include <QSet>
#include <QMenu>
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

ColumnFilterFrame::ColumnFilterFrame(GdxSymbol *symbol, int column, QWidget *parent)
    : QFrame(parent),
      mSymbol(symbol),
      mColumn(column),
      mModel(new FilterUelModel(symbol, column, this))
{
    ui.setupUi(this);

    connect(ui.pbApply, &QPushButton::clicked, this, &ColumnFilterFrame::apply);
    connect(ui.pbSelectAll, &QPushButton::clicked, this, &ColumnFilterFrame::selectAll);
    connect(ui.pbDeselectAll, &QPushButton::clicked, this, &ColumnFilterFrame::deselectAll);
    connect(ui.leSearch, &QLineEdit::textChanged, this, &ColumnFilterFrame::filterLabels);
    connect(ui.cbToggleHideUnselected, &QCheckBox::toggled, this, &ColumnFilterFrame::toggleHideUnselected);
    connect(mModel, &FilterUelModel::dataChanged, this, &ColumnFilterFrame::listDataHasChanged);

    ui.lvLabels->setModel(mModel);
}

ColumnFilterFrame::~ColumnFilterFrame()
{
    delete mModel;
}

void ColumnFilterFrame::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ColumnFilterFrame::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ColumnFilterFrame::apply()
{
    bool* showUelInColumn =  mSymbol->showUelInColumn().at(mColumn);
    std::vector<int>* uelsInColumn = mSymbol->uelsInColumn().at(mColumn);
    bool checked;
    std::vector<bool> filterActive = mSymbol->filterActive();
    filterActive[mColumn] = false;
    for (size_t idx=0; idx<uelsInColumn->size(); idx++) {
        checked = mModel->checked()[idx];
        showUelInColumn[uelsInColumn->at(idx)] = checked;
        if(!checked)
            filterActive[mColumn] = true;
    }
    mSymbol->setFilterActive(filterActive);
    mSymbol->filterRows();
    static_cast<QMenu*>(this->parent())->close();
    mSymbol->setFilterHasChanged(true);
}

void ColumnFilterFrame::selectAll()
{
    for(int row=0; row<mModel->rowCount(); row++)
        mModel->setData(mModel->index(row,0), true, Qt::CheckStateRole);
}

void ColumnFilterFrame::deselectAll()
{
    for(int row=0; row<mModel->rowCount(); row++)
        mModel->setData(mModel->index(row,0), false, Qt::CheckStateRole);
}

void ColumnFilterFrame::filterLabels()
{
    QString filterString = ui.leSearch->text();
    if(filterString.isEmpty())
        filterString = "*";
    mModel->filterLabels(filterString);
}

void ColumnFilterFrame::toggleHideUnselected(bool checked)
{
    if (checked) {
        for(int row=0; row<mModel->rowCount(); row++)
            ui.lvLabels->setRowHidden(row, !mModel->checked()[row]);
    }
    else
        ui.lvLabels->reset();
}

void ColumnFilterFrame::listDataHasChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(roles)
    if (ui.cbToggleHideUnselected->isChecked()) {
        for(int row=topLeft.row(); row<=bottomRight.row(); row++)
            ui.lvLabels->setRowHidden(row, !mModel->checked()[row]);
    }
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
