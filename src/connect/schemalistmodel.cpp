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
#include <QMimeData>
#include <QFile>

#include "schemalistmodel.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace connect {

SchemaListModel::SchemaListModel(const QStringList& schema, QObject *parent)
    : QStandardItemModel{parent},
      mSchema(schema)
{
    for(int row=0; row<mSchema.size(); row++) {
        QStandardItem *sitem = new QStandardItem();
        sitem->setData( mSchema.at(row), Qt::DisplayRole );
        sitem->setEditable(false);
        sitem->setSelectable(true);
        sitem->setTextAlignment(Qt::AlignLeft);
        sitem->setForeground(Theme::color(Theme::Normal_Green));
        sitem->setToolTip(
             QString("<html><head/><body><p>%1 <span style=' font-weight:600;'>%2</span> %3.</p> <p>%4 <span style=' font-weight:600;'>%2</span> %5 from %3.</p> <p>%6 <span style=' font-weight:600;'>%2</span> %5 from %3.</p></body></html>")
                 .arg("Select or click to show", mSchema.at(row), "definition", "Double click to append", "data", "Drag and drop to insert")
        );
        setItem(row, 0, sitem);
    }
}

Qt::ItemFlags SchemaListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::NoItemFlags;
    else
        return Qt::ItemIsDragEnabled | defaultFlags;
}

QStringList SchemaListModel::mimeTypes() const
{
    QStringList types;
    types <<  "application/vnd.gams-connect.text";
    return types;
}

QMimeData *SchemaListModel::mimeData(const QModelIndexList &indexes) const
{
    emit schemaItemChanged(item(indexes.last().row())->text());

    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QFile::WriteOnly);

    for (const QModelIndex &index : indexes) {
        QStandardItem* sitem = item(index.row());
        QString text = QString("schema=%1").arg(sitem->data(Qt::DisplayRole).toString());
        stream << text;
        break;
    }
    mimeData->setData( "application/vnd.gams-connect.text", encodedData);
    return mimeData;
}

void SchemaListModel::setToolTip(const QModelIndex &index)
{
    QStandardItem* it = nullptr;
    for(int row=0; row<mSchema.size(); row++) {
        QStandardItem *sitem = item(index.row());
        if (index.row()==row)
            it=sitem;
        sitem->setToolTip("");
    }
    if (it)
       it->setToolTip(
                   QString("<html><head/><body><p>%1 <span style=' font-weight:600;'>%2</span> %3.</p> <p>%4 <span style=' font-weight:600;'>%2</span> %5 from %3.</p> <p>%6 <span style=' font-weight:600;'>%2</span> %5 from %3.</p></body></html>")
                       .arg("Select or click to show", it->data(Qt::DisplayRole).toString(), "definition", "Double click to append", "data", "Drag and drop to insert")
          );

}

} // namespace connect
} // namespace studio
} // namespace gams

