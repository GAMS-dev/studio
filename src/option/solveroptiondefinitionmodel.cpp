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
#include "solveroptiondefinitionmodel.h"
#include "settings.h"
#include <QDataStream>
#include <QFile>

namespace gams {
namespace studio {
namespace option {

SolverOptionDefinitionModel::SolverOptionDefinitionModel(Option *data, int optionGroup, QObject *parent):
    OptionDefinitionModel (data, optionGroup, parent)
{
    QList<QVariant> rootData;
    rootData << "Option" << "Synonym" << "DefValue" << "Range"
             << "Type" << "Description" << "Debug Entry" ;
    rootItem = new OptionDefinitionItem(rootData);

    setupTreeItemModelData(mOption, rootItem);
}

QStringList SolverOptionDefinitionModel::mimeTypes() const
{
    QStringList types;
    types << optionMimeType(OptionDefinitionType::SolverOptionDefinition);
    return types;
}

QMimeData *SolverOptionDefinitionModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData();
    if (isViewCompact)
        return mimeData;

    QByteArray encodedData;
    QDataStream stream(&encodedData, QFile::WriteOnly);

    Settings* settings = Settings::settings();
    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            if (index.column()>0) {
                continue;
            }
            OptionDefinitionItem *childItem = static_cast<OptionDefinitionItem*>(index.internalPointer());
            OptionDefinitionItem *parentItem = childItem->parentItem();

            const QString lineComment = mOption->isEOLCharDefined() ? QString(mOption->getEOLChars().at(0)) : QString("*");
            const QModelIndex descriptionIndex = index.sibling(index.row(), OptionDefinitionModel::COLUMN_DESCIPTION);
            if (parentItem == rootItem) {
                if (settings && settings->toBool(skSoAddCommentAbove)) {
                    stream << QString("%1 %2").arg(lineComment, data(descriptionIndex, Qt::DisplayRole).toString());
                }
                const QModelIndex defValueIndex = index.sibling(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE);
                const QModelIndex optionIdIndex = index.sibling(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER);
                stream << QString("%1=%2=%3=%4").arg(data(index, Qt::DisplayRole).toString(),
                                                     data(defValueIndex, Qt::DisplayRole).toString(),
                                                     data(descriptionIndex, Qt::DisplayRole).toString(),
                                                     data(optionIdIndex, Qt::DisplayRole).toString());
            } else {
                if (settings && settings->toBool(skSoAddCommentAbove)) {
                    stream << QString("%1 %2").arg(lineComment, parentItem->data(OptionDefinitionModel::COLUMN_DESCIPTION).toString());
                    stream << QString("%1 %2 - %3").arg(lineComment,
                                                        data(index, Qt::DisplayRole).toString(),
                                                        data(descriptionIndex, Qt::DisplayRole).toString());
                }
                stream << QString("%1=%2=%3=%4").arg(parentItem->data(OptionDefinitionModel::COLUMN_OPTION_NAME).toString(),
                                                     data(index, Qt::DisplayRole).toString(),
                                                     data(descriptionIndex, Qt::DisplayRole).toString(),
                                                     parentItem->data(OptionDefinitionModel::COLUMN_ENTRY_NUMBER).toString());
            }
        }
    }

    mimeData->setData(optionMimeType(OptionDefinitionType::SolverOptionDefinition), encodedData);
    return mimeData;
}

void SolverOptionDefinitionModel::modifyOptionDefinitionItem(const SolverOptionItem* optionItem)
{
    QModelIndexList indices = match(index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                             Qt::DisplayRole,
                                             QString::number(optionItem->optionId) , 1);
    beginResetModel();
    for(const QModelIndex &idx : std::as_const(indices)) {
        QModelIndex node = index(idx.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER);

        OptionDefinitionItem* nodeItem = static_cast<OptionDefinitionItem*>(node.internalPointer());
        OptionDefinitionItem *parentItem = nodeItem->parentItem();
        if (parentItem == rootItem) {
            OptionDefinition optdef = mOption->getOptionDefinition(nodeItem->data(OptionDefinitionModel::COLUMN_OPTION_NAME).toString());
            optdef.modified = !optionItem->disabled;
            setData(node, optdef.modified ? Qt::CheckState(Qt::Checked) : Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
        }
    }
    endResetModel();
}

void SolverOptionDefinitionModel::modifyOptionDefinition(const QList<SolverOptionItem *> &optionItems)
{
    QMap<int, int> modifiedOption;
    for(int i = 0; i<optionItems.size(); ++i) {
        if (optionItems.at(i)->optionId != -1)
            modifiedOption[optionItems.at(i)->optionId] = i;
    }

    const QList<int> ids = modifiedOption.keys();
    beginResetModel();
    for(int i=0; i<rowCount(); ++i)  {
        const QModelIndex node = index(i, OptionDefinitionModel::COLUMN_ENTRY_NUMBER);

        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(node.internalPointer());
        OptionDefinitionItem *parentItem = item->parentItem();
        if (parentItem == rootItem) {
            OptionDefinition optdef = mOption->getOptionDefinition(item->data(OptionDefinitionModel::COLUMN_OPTION_NAME).toString());
            if (ids.contains(optdef.number))
                optdef.modified = true;
            else
                optdef.modified = false;
            setData(node, optdef.modified ? Qt::CheckState(Qt::Checked) : Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
        }
    }
    endResetModel();
}

void SolverOptionDefinitionModel::on_compactViewChanged(bool compact)
{
    isViewCompact = compact;
}

} // namespace option
} // namespace studio
} // namespace gams
