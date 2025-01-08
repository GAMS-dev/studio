/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMSCONFIGPARAMETERTABLEMODEL_H
#define GAMSCONFIGPARAMETERTABLEMODEL_H

#include <QAbstractTableModel>

#include "optiontokenizer.h"

namespace gams {
namespace studio {
namespace option {

class ConfigParamTableModel : public QAbstractTableModel
{
     Q_OBJECT
public:
    ConfigParamTableModel(const QList<ParamConfigItem *> &itemList, OptionTokenizer* tokenizer, QObject *parent = nullptr);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList & indexes) const override;

    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;

    static const int COLUMN_PARAM_KEY = 0;
    static const int COLUMN_PARAM_VALUE = 1;
    static const int COLUMN_MIN_VERSION = 2;
    static const int COLUMN_MAX_VERSION = 3;
    static const int COLUMN_ENTRY_NUMBER = 4;

    const QList<ParamConfigItem *> parameterConfigItems();

signals:
    void newTableRowDropped(const QModelIndex &index);
    void configParamModelChanged(const QList<gams::studio::option::ParamConfigItem *> &optionItem);
    void configParamItemRemoved();

public slots:
    void on_groupDefinitionReloaded();
    void on_reloadConfigParamModel(const QList<gams::studio::option::ParamConfigItem *> &optionItem);
    void on_updateConfigParamItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_removeConfigParamItem();
    void updateRecurrentStatus();
    QString getParameterTableEntry(int row);

private slots:
    void setRowCount(int rows);
    void updateCheckState();

private:
    QList<ParamConfigItem *> mOptionItem;
    QList<QString> mHeader;
    QMap<int, QVariant> mCheckState;

    OptionTokenizer* mOptionTokenizer;
    Option* mOption;
};

} // namepsace option
} // namespace studio
} // namespace gams
#endif // GAMSCONFIGPARAMETERTABLEMODEL_H
