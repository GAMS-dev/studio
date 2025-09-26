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

    QByteArray encodedData;
    QDataStream stream(&encodedData, QFile::WriteOnly);

    Settings* settings = Settings::settings();
    QString name;
    QString value;
    const QString commentChar = mOption->isEOLCharDefined() ? QString(mOption->getEOLChars().at(0)) : QString("*");
    QString EOLComment;
    QString aboveComment;
    QString entry;
    for (const QModelIndex &index : indexes) {
        if (!index.isValid())
            continue;
        switch (index.column()) {
        case OptionDefinitionModel::COLUMN_OPTION_NAME:
            name = index.parent().isValid() ? index.parent().data().toString() : index.data().toString();
            break;
        case OptionDefinitionModel::COLUMN_DEF_VALUE:
            value = (index.parent().isValid() ? index.siblingAtColumn(OptionDefinitionModel::COLUMN_OPTION_NAME).data().toString()
                                              : index.data().toString());
            break;
        case OptionDefinitionModel::COLUMN_DESCIPTION:
            aboveComment = (index.parent().isValid() ? index.parent().siblingAtColumn(OptionDefinitionModel::COLUMN_DESCIPTION).data().toString()
                                                     : index.data().toString());
            EOLComment = index.data().toString();
            break;
        case OptionDefinitionModel::COLUMN_ENTRY_NUMBER:
            entry = (index.parent().isValid() ? index.parent().siblingAtColumn(OptionDefinitionModel::COLUMN_ENTRY_NUMBER).data().toString()
                                              : index.data().toString());
            break;
        default: break;
        }
    }
    if (settings && settings->toBool(skSoAddCommentAbove))
        stream << QString("%1 %2").arg(commentChar, aboveComment);
    if (indexes.size()>0) {
        if (indexes.first().parent().isValid() &&
            settings && settings->toBool(skSoAddCommentAbove) && !settings->toBool(skSoAddEOLComment))
            stream << QString("%1 %2 - %3").arg(commentChar, value, EOLComment);
        if (entry.isEmpty())
            entry = indexes.first().siblingAtColumn(OptionDefinitionModel::COLUMN_ENTRY_NUMBER).data().toString();
    }
    stream << QString("%1=%2=%3=%4").arg(name, value, EOLComment, entry);

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
