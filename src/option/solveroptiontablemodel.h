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
#ifndef SOLVEROPTIONTABLEMODEL_H
#define SOLVEROPTIONTABLEMODEL_H

#include "optiontablemodel.h"

namespace gams {
namespace studio {
namespace option {

class SolverOptionTableModel : public OptionTableModel
{
    Q_OBJECT
public:
    SolverOptionTableModel(const QList<OptionItem> itemList, OptionTokenizer* tokenizer, QObject *parent = nullptr);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool insertRows(int row, int count, const QModelIndex &parent) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData * mimedata, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;

};

} // namepsace option
} // namespace studio
} // namespace gams

#endif // SOLVEROPTIONTABLEMODEL_H
