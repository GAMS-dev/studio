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
#ifndef OPTIONTABLEMODEL_H
#define OPTIONTABLEMODEL_H

#include <QAbstractTableModel>
#include <QMimeData>

#include "optiontokenizer.h"
#include "option.h"

namespace gams {
namespace studio {
namespace option {

class OptionTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    OptionTableModel(const QList<OptionItem> itemList, OptionTokenizer* tokenizer, QObject *parent = nullptr);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    QStringList mimeTypes() const override;

    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData * mimedata, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;

    QList<OptionItem> getCurrentListOfOptionItems() const;

signals:
    void newTableRowDropped(const QModelIndex &index);

public slots:
    void on_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void reloadOptionModel(const QList<OptionItem> &optionItem);

protected:
    QList<OptionItem> mOptionItem;
    QList<QString> mHeader;
    QMap<int, QVariant> mCheckState;

    OptionTokenizer* mOptionTokenizer;
    Option* mOption;

    void setRowCount(int rows);

};

} // namepsace option
} // namespace studio
} // namespace gams

#endif // OPTIONTABLEMODEL_H
