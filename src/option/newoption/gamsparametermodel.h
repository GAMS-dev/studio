/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMSPARAMETERMODEL_H
#define GAMSPARAMETERMODEL_H

#include "optiontablemodel.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

class GamsParameterModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    GamsParameterModel(const GamsParameterItem &item, OptionTokenizer* tokenizer, QObject *parent = nullptr);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

//    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    static const int COLUMN_ENTRY = 0;
    static const int COLUMN_KEY   = 1;
    static const int COLUMN_VALUE = 2;

    static QStringList header() {
        QStringList header = QStringList() << "Entry" << "Key" << "Value" << "Comment" ;
        return header;
    }

private:
    GamsParameterItem mOptionItem;
    QList<QString> mHeader;
    QList<QVariant> mCheckState;

    OptionTokenizer* mOptionTokenizer;
    Option* mOption;

    void setRowCount(int rows);
    void updateCheckState();

};

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // GAMSPARAMETERMODEL_H
