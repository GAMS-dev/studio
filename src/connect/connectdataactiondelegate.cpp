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
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

#include "connectdataactiondelegate.h"

namespace gams {
namespace studio {
namespace connect {

ConnectDataActionDelegate::ConnectDataActionDelegate(DataItemColumn column, QObject *parent)
    : QStyledItemDelegate{parent},
      mColumn(column)
{

}

void ConnectDataActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setBackgroundMode(Qt::TransparentMode);
    QBrush bg = QBrush(qvariant_cast<QColor>(index.data(Qt::BackgroundRole)));
    painter->fillRect(option.rect, bg);

    QIcon icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
    if (!icon.isNull()) {
       painter->translate(option.rect.left(), option.rect.top());
//       QRect clip(0, 0, option.rect.width(), option.rect.height());

       qDebug() << "paint... " << (int)mColumn <<"::"<< index.row() << "," << index.column();
//       icon.paint(painter, clip, Qt::AlignVCenter|Qt::AlignLeft);
       icon.paint(painter, QRect(0, 0, 16, 16), Qt::AlignVCenter|Qt::AlignLeft);
    }
    painter->restore();
}

bool ConnectDataActionDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    qDebug() << "editorEvent";
    if (event->type()==QEvent::MouseButtonRelease) {
        qDebug() << "mouse release";
        if (model->data(index, Qt::DisplayRole ).toBool())
            qDebug() << "true(" << index.row() << "," << index.column() << ") " << (int)mColumn;
        else
            qDebug() << "false(" << index.row() << "," << index.column() << ")" << (int)mColumn;
        return (index.data( Qt::DisplayRole ).toBool());
    }
    return QStyledItemDelegate::editorEvent(event,model, option, index);
}

} // namespace connect
} // namespace studio
} // namespace gams
