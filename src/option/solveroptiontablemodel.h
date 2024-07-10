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
#ifndef SOLVEROPTIONTABLEMODEL_H
#define SOLVEROPTIONTABLEMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>

#include "optiontokenizer.h"
#include "option.h"

namespace gams {
namespace studio {
namespace option {

class SolverOptionTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SolverOptionTableModel(const QList<SolverOptionItem *> &itemList, OptionTokenizer* tokenizer, QObject *parent = nullptr);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QSize span(const QModelIndex &index) const override;

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

    QList<SolverOptionItem *> getCurrentListOfOptionItems() const;
    QString getOptionTableEntry(int row);

    static const int COLUMN_OPTION_KEY = 0;
    static const int COLUMN_OPTION_VALUE = 1;
    static const int COLUMN_EOL_COMMENT = 2;

    // temporary, for debug only
    int getColumnEntryNumber() const;
    void setColumnEntryNumber(int column);

signals:
    void newTableRowDropped(const QModelIndex &index);
    void solverOptionModelChanged(const QList<gams::studio::option::SolverOptionItem *> &optionItem);
    void solverOptionItemModelChanged(const gams::studio::option::SolverOptionItem* optionItem);
    void solverOptionItemRemoved();
    void columnSpanned(int row);
    void columnUnspanned(int row);

public slots:
    void reloadSolverOptionModel(const QList<gams::studio::option::SolverOptionItem *> &optionItem);
    void on_updateSolverOptionItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_removeSolverOptionItem();
    void on_toggleRowHeader(int logicalIndex);
    void on_groupDefinitionReloaded();
    void updateRecurrentStatus();

private:
    QList<SolverOptionItem *> mOptionItem;
    QList<QString> mHeader;
    QMap<int, QVariant> mCheckState;

    OptionTokenizer* mOptionTokenizer;
    Option* mOption;

    void setRowCount(int rows);
    void updateCheckState();

    // temporary, for debug only
    int columnEntryNumber = 2;
};

} // namepsace option
} // namespace studio
} // namespace gams

#endif // SOLVEROPTIONTABLEMODEL_H
