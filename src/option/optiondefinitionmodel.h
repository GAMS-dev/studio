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
#ifndef OPTIONDEFINITIONMODEL_H
#define OPTIONDEFINITIONMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>

#include "option.h"
#include "optiondefinitionitem.h"

namespace gams {
namespace studio {
namespace option {

class OptionDefinitionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    OptionDefinitionModel(Option* data, int optionGroup=0, QObject* parent=nullptr);
    ~OptionDefinitionModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                     int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    OptionDefinitionItem* getItem(const QModelIndex &index) const;
    void insertItem(int position, OptionDefinitionItem* item, const QModelIndex &parent);
    bool removeItem(const QModelIndex &index);

//    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    virtual QStringList mimeTypes() const override = 0;
    virtual QMimeData* mimeData(const QModelIndexList & indexes) const override = 0;

    static const int COLUMN_OPTION_NAME = 0;
    static const int COLUMN_SYNONYM = 1;
    static const int COLUMN_DEF_VALUE = 2;
    static const int COLUMN_RANGE = 3;
    static const int COLUMN_TYPE = 4;
    static const int COLUMN_DESCIPTION = 5;
    static const int COLUMN_ENTRY_NUMBER = 6;

    static const int NUMBER_DISPLAY_ENUMSTR_RANGE = 3;

public slots:
    void loadOptionFromGroup(const int group);

protected:
    void setupTreeItemModelData(Option* option, OptionDefinitionItem* parent);

    int mOptionGroup;
    Option* mOption;
    OptionDefinitionItem *rootItem;
};

} // namespace option
} // namespace studio
} // namespace gams

#endif // OPTIONDEFINITIONMODEL_H
