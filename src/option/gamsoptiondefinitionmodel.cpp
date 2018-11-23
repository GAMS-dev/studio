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
#include "gamsoptiondefinitionmodel.h"

namespace gams {
namespace studio {
namespace option {

GamsOptionDefinitionModel::GamsOptionDefinitionModel(Option *data, int optionGroup, QObject *parent):
    OptionDefinitionModel (data, optionGroup, parent)
{

}

QStringList GamsOptionDefinitionModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.gams-pf.text";
    return types;
}

QMimeData *GamsOptionDefinitionModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            if (index.column()>0) {
                continue;
            }

            QString text;
            OptionDefinitionItem *childItem = static_cast<OptionDefinitionItem*>(index.internalPointer());
            OptionDefinitionItem *parentItem = childItem->parentItem();

            if (parentItem == rootItem) {
                QModelIndex defValueIndex = index.sibling(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE);
                text = QString("%1=%2").arg(data(index, Qt::DisplayRole).toString()).arg(data(defValueIndex, Qt::DisplayRole).toString());
            } else {
                text = QString("%1=%2").arg(parentItem->data(index.column()).toString()).arg(data(index, Qt::DisplayRole).toString());
            }
            stream << text;
        }
    }

    mimeData->setData("application/vnd.gams-pf.text", encodedData);
    return mimeData;
}

void GamsOptionDefinitionModel::modifyOptionDefinition(const QList<OptionItem> &optionItems)
{
    QStringList optionNameList;
    for(OptionItem item : optionItems) {
        optionNameList << item.key;
    }
    for(int i=0; i<rowCount(); ++i)  {
        QModelIndex node = index(i, OptionDefinitionModel::COLUMN_OPTION_NAME);

        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(node.internalPointer());
        OptionDefinitionItem *parentItem = item->parentItem();
        if (parentItem == rootItem) {
            OptionDefinition optdef = mOption->getOptionDefinition(item->data(OptionDefinitionModel::COLUMN_OPTION_NAME).toString());
            if (optionNameList.contains(optdef.name, Qt::CaseInsensitive) || optionNameList.contains(optdef.synonym, Qt::CaseInsensitive))
                optdef.modified = true;
            else
                optdef.modified = false;
            setData(node, optdef.modified ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

    }
}


} // namespace option
} // namespace studio
} // namespace gams
