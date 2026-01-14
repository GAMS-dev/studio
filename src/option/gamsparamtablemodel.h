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
#ifndef GAMSPARAMTABLEMODEL_H
#define GAMSPARAMTABLEMODEL_H

#include "option/optiontablemodel.h"

namespace gams {
namespace studio {
namespace option {

class GamsParamTableModel : public OptionTableModel
{
     Q_OBJECT
public:
    explicit GamsParamTableModel(const QString& callstr, const QString &normalizedCommandLineStr, OptionTokenizer* tokenizer, QObject *parent = nullptr);
    explicit GamsParamTableModel(const QString& callstr, const QList<OptionItem*> &itemList, OptionTokenizer* tokenizer, QObject *parent = nullptr);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

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


public slots:
    void on_groupDefinitionReloaded() override;
    QString getOptionTableEntry(int row) override;

    void on_updateOptionItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) override;
    void on_removeOptionItem() override;

    void toggleActiveOptionItem(int index);
    void on_ParameterTableModelChanged(const QString &text);

    void updateRecurrentStatus();

private:
    QList<OptionItem*> mOptionItem;
    QList<QString> mHeader;
    QMap<int, QVariant> mCheckState;

    bool mTokenizerUsed;

    void setRowCount(int rows);
    void itemizeOptionFromCommandLineStr(const QString &text);

    QList<OptionItem *> getCurrentListOfOptionItems();
    QString getParameterTableEntry(int row);
};

} // namepsace option
} // namespace studio
} // namespace gams

#endif // GAMSPARAMTABLEMODEL_H
