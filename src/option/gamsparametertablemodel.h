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
#ifndef GAMSPARAMETERTABLEMODEL_H
#define GAMSPARAMETERTABLEMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>

#include "optiontokenizer.h"

namespace gams {
namespace studio {
namespace option {

class GamsParameterTableModel : public QAbstractTableModel
{
     Q_OBJECT
public:
    GamsParameterTableModel(const QString &normalizedCommandLineStr, OptionTokenizer* tokenizer, QObject *parent = nullptr);
    GamsParameterTableModel(const QList<OptionItem> &itemList, OptionTokenizer* tokenizer, QObject *parent = nullptr);

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

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList & indexes) const override;

    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;

    static const int COLUMN_OPTION_KEY = 0;
    static const int COLUMN_OPTION_VALUE = 1;
    static const int COLUMN_ENTRY_NUMBER = 2;

signals:
    void newTableRowDropped(const QModelIndex &index);
    void optionModelChanged(const QList<gams::studio::option::OptionItem> &optionItem);
    void optionNameChanged(const QString &from, const QString &to);
    void optionValueChanged(const QModelIndex &index);

public slots:
    void toggleActiveOptionItem(int index);
    void on_ParameterTableModelChanged(const QString &text);

private:
    QList<OptionItem> mOptionItem;
    QList<QString> mHeader;
    QMap<int, QVariant> mCheckState;

    OptionTokenizer* mOptionTokenizer;
    Option* mOption;

    bool mTokenizerUsed;

    void setRowCount(int rows);
    void itemizeOptionFromCommandLineStr(const QString &text);

    QList<OptionItem> getCurrentListOfOptionItems();
    QString getParameterTableEntry(int row);
};

} // namepsace option
} // namespace studio
} // namespace gams

#endif // GAMSPARAMETERTABLEMODEL_H
