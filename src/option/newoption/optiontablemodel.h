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
#ifndef OPTIONTABLEMODEL_H
#define OPTIONTABLEMODEL_H

#include <QAbstractTableModel>

#include "option/optiontokenizer.h"

namespace gams {
namespace studio {
namespace option {

struct OptionItem;
struct SolverOptionItem;
class ConfigItem;

namespace newoption {

typedef QList<SolverOptionItem*>  GamsParameterItem;     // QList<GamsParameter*>
//typedef QList<SolverOptionItem*>  SolverParameterItem;   // QList<SolverOption*>
typedef QList<SolverOptionItem*>  GamsParameterLineItem; // QList<LineOptin*>
typedef QList<ParamConfigItem*>   GucParameterItem;      // QList<GAMSParameter*>

class OptionTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    OptionTableModel(OptionTokenizer* tokenizer, QObject *parent = nullptr) :
        QAbstractTableModel(parent), mOptionTokenizer(tokenizer)
    { }

    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
        if (!index.isValid())
            return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
        else
            return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    }

    static const int COLUMN_ID    = 0;
    static const int COLUMN_KEY   = 1;
    static const int COLUMN_VALUE = 2;

    inline static int column_id()    { return COLUMN_ID;    }
    inline static int column_key()   { return COLUMN_KEY;   }
    inline static int column_value() { return COLUMN_VALUE; }

    inline QStringList headers()  { return mHeader; }

signals:
    void newTableRowDropped(const QModelIndex &index);
    void optionItemRemoved();

    void optionModelChanged(const QList<OptionItem*> &optionItem);
    void optionNameChanged(const QString &from, const QString &to);
    void optionValueChanged(const QModelIndex &index);

public slots:
    virtual void on_groupDefinitionReloaded() = 0;
    virtual QString getOptionTableEntry(int row) =0;

    virtual void on_updateOptionItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) = 0;
    virtual void on_removeOptionItem() = 0;

//    Qt::CheckState checkState(int index) const { return mError[index]; }
//    QList<int> recurrent(int index) const { return (index < mRecurrence.size() ? mRecurrence[index] : QList<int>()); }

protected:
    OptionTokenizer* mOptionTokenizer;
    QStringList mHeader;
    QMap<int, QVariant> mCheckState;

    QList<Qt::CheckState> mError;
    QList<QList<int>> mRecurrence;
};

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // OPTIONTABLEMODEL_H
