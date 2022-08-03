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

#include "connectdatakeydelegate.h"
#include "connectdatamodel.h"

namespace gams {
namespace studio {
namespace connect {

ConnectDataKeyDelegate::ConnectDataKeyDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void ConnectDataKeyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    painter->setBackgroundMode(Qt::TransparentMode);
    QBrush bg = QBrush(qvariant_cast<QColor>(index.data(Qt::BackgroundRole)));
    painter->fillRect(option.rect, bg);

//    int textLeftOffset = 10, iconSize=30, iconTopOffset=20, textTopOffset=0;
    QRect textRect = option.rect;
    QRect iconRect = option.rect;
    if (index.parent().isValid()) {

        QColor fg = QColor(qvariant_cast<QColor>(index.data(Qt::ForegroundRole)));
        painter->setPen(fg);
//        painter->drawText(QPoint(option.rect.x()+5, option.rect.y()+option.rect.height()-5), index.data(Qt::DisplayRole).toString());
        painter->drawText(textRect.left(),option.rect.y()+option.rect.height()-5, index.data(Qt::DisplayRole).toString());

        QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CHECK_STATE );
        if (checkstate_index.data( Qt::DisplayRole ).toInt()==2) {
           QIcon icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
           if (!icon.isNull()) {
              painter->translate(iconRect.right()-20, iconRect.top()+5);

              icon.paint(painter, QRect(0, 0, 16, 16), Qt::AlignVCenter|Qt::AlignRight);
              qDebug() << "2 keypaint... "
                       << "topleft(" << painter->viewport().topLeft().x() << "," << painter->viewport().topLeft().y() << ") "
                       << "bottomright(" << painter->viewport().bottomRight().x() << "," << painter->viewport().bottomRight().y()<< ")";
              qDebug() << "              option.rect(" << iconRect.x() << "," << iconRect.y() << ")";
           }
        } else if (checkstate_index.data( Qt::DisplayRole ).toInt()==4) {
                  QIcon icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
                  if (!icon.isNull()) {
                      painter->translate(option.rect.left()+5, option.rect.top());

                      icon.paint(painter, QRect(0, 0, 16, 16), Qt::AlignVCenter|Qt::AlignLeft);
                      qDebug() << "4 keypaint... "
                               << "topleft(" << painter->viewport().topLeft().x() << "," << painter->viewport().topLeft().y() << ") "
                               << "bottomright(" << painter->viewport().bottomRight().x() << "," << painter->viewport().bottomRight().y();
                      qDebug() << "              option.rect(" << iconRect.x() << "," << iconRect.y() << ")";

                  }
        }
    } else {
        QIcon icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        if (!icon.isNull()) {
           painter->translate(option.rect.left()+5, option.rect.top());

           qDebug() << "0 keypaint... ";
           icon.paint(painter, QRect(0, 0, 16, 16), Qt::AlignVCenter|Qt::AlignLeft);
        }

        QColor fg = QColor(qvariant_cast<QColor>(index.data(Qt::ForegroundRole)));
        painter->setPen(fg);
        painter->drawText(QPoint(option.rect.x(), option.rect.y()+option.rect.height()-10), index.data(Qt::DisplayRole).toString());
    }
    painter->restore();
}


bool ConnectDataKeyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type()==QEvent::MouseButtonRelease) {
        qDebug() << "key mouse release";
        const QMouseEvent* const mouseevent = static_cast<const QMouseEvent*>( event );
        const QPoint p = mouseevent->pos();  // ->globalPos()
        qDebug() << "position(" << p.x() << "," << p.y() << ") ";
        return (index.data( Qt::DisplayRole ).toBool());
    }
    return QStyledItemDelegate::editorEvent(event,model, option, index);

}

} // namespace connect
} // namespace studio
} // namespace gams
