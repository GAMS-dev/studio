/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include "gamsoptiondefinitionmodel.h"
#include <QDataStream>

namespace gams {
namespace studio {
namespace option {

GamsOptionDefinitionModel::GamsOptionDefinitionModel(Option *data, int optionGroup, QObject *parent):
    OptionDefinitionModel (data, optionGroup, parent)
{
    QList<QVariant> rootData;
    rootData << "Parameter" << "Synonym" << "DefValue" << "Range"
             << "Type" << "Description" << "Debug Entry" ;
    rootItem = new OptionDefinitionItem(rootData);

    setupTreeItemModelData(mOption, rootItem);
}

QStringList GamsOptionDefinitionModel::mimeTypes() const
{
    QStringList types;
    types <<  optionMimeType(OptionDefinitionType::GamsOptionDefinition);
    return types;
}

QMimeData *GamsOptionDefinitionModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            if (index.column()>0) {
                continue;
            }

            QString text;
            OptionDefinitionItem *childItem = static_cast<OptionDefinitionItem*>(index.internalPointer());
            OptionDefinitionItem *parentItem = childItem->parentItem();

            if (parentItem == rootItem) {
                QModelIndex defValueIndex = index.sibling(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE);
                QModelIndex optionIdIndex = index.sibling(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER);
                text = QString("%1=%2=%3").arg(data(index, Qt::DisplayRole).toString(),
                                               data(defValueIndex, Qt::DisplayRole).toString(),
                                               data(optionIdIndex, Qt::DisplayRole).toString());
            } else {
                text = QString("%1=%2=%3").arg(parentItem->data(index.column()).toString(),
                                               data(index, Qt::DisplayRole).toString(),
                                               parentItem->data(OptionDefinitionModel::COLUMN_ENTRY_NUMBER).toString());
            }
            stream << text;
        }
    }

    mimeData->setData(optionMimeType(OptionDefinitionType::GamsOptionDefinition), encodedData);
    return mimeData;
}

void GamsOptionDefinitionModel::modifyOptionDefinitionItem(const OptionItem &optionItem)
{
    QModelIndexList indices = match(index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                             Qt::DisplayRole,
                                             QString::number(optionItem.optionId) , 1);
    beginResetModel();
    for(QModelIndex idx : qAsConst(indices)) {
        QModelIndex node = index(idx.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER);

        OptionDefinitionItem* nodeItem = static_cast<OptionDefinitionItem*>(node.internalPointer());
        OptionDefinitionItem *parentItem = nodeItem->parentItem();
        if (parentItem == rootItem) {
            OptionDefinition optdef = mOption->getOptionDefinition(nodeItem->data(OptionDefinitionModel::COLUMN_OPTION_NAME).toString());
            optdef.modified = !optionItem.disabled;
            setData(node, optdef.modified ? Qt::CheckState(Qt::Checked) : Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
        }
    }
    endResetModel();
}

void GamsOptionDefinitionModel::modifyOptionDefinition(const QList<OptionItem> &optionItems)
{
    QMap<int, int> modifiedOption;
    for(int i = 0; i<optionItems.size(); ++i) {
        if (optionItems.at(i).optionId != -1)
            modifiedOption[optionItems.at(i).optionId] = i;
    }
    QList<int> ids = modifiedOption.keys();
    beginResetModel();
    for(int i=0; i<rowCount(); ++i)  {
        QModelIndex node = index(i, OptionDefinitionModel::COLUMN_OPTION_NAME);

        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(node.internalPointer());
        OptionDefinitionItem *parentItem = item->parentItem();
        if (parentItem == rootItem) {
            OptionDefinition optdef = mOption->getOptionDefinition(item->data(OptionDefinitionModel::COLUMN_OPTION_NAME).toString());
            if (ids.contains(optdef.number))
                optdef.modified = true;
            else
                optdef.modified = false;
            setData(node, optdef.modified ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
    }
    endResetModel();
}


} // namespace option
} // namespace studio
} // namespace gams
