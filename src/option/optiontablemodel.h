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

#include <QAbstractItemModel>

#include "commandlinetokenizer.h"
#include "option.h"

namespace gams {
namespace studio {

class OptionTableModel : public QAbstractTableModel
{
     Q_OBJECT
public:
    OptionTableModel(const QString normalizedCommandLineStr, CommandLineTokenizer* tokenizer, QObject *parent = 0);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;
    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    QList<OptionItem> getCurrentListOfOptionItems();

signals:
    void optionModelChanged(const QList<OptionItem> &optionItem);

public slots:
    void toggleActiveOptionItem(int index);
    void on_optionTableModelChanged(const QString &text);

private:
    QList<OptionItem> mOptionItem;
    QList<QString> mHeader;
    QMap<int, QVariant> mCheckState;

    CommandLineTokenizer* commandLineTokenizer;
    Option* gamsOption;

    void setRowCount(int rows);
    void itemizeOptionFromCommandLineStr(const QString text);
    void validateOption();
};

} // namespace studio
} // namespace gams

#endif // OPTIONTABLEMODEL_H
