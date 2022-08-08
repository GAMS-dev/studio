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
#include <QToolTip>

#include "connectdatakeydelegate.h"
#include "connectdatamodel.h"

namespace gams {
namespace studio {
namespace connect {

ConnectDataKeyDelegate::ConnectDataKeyDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
    mIconWidth = 16;
    mIconHeight = 16;
}

void ConnectDataKeyDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->text = index.data(Qt::DisplayRole).toString();
    if (index.parent().isValid()) {
        QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CHECK_STATE );
        if (checkstate_index.data( Qt::DisplayRole ).toInt()==2) {
            option->icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
            option->decorationPosition = QStyleOptionViewItem::Right;
            qDebug() << "2 icon rec(" << option->rect.topLeft().x() << "," << option->rect.topLeft().y() << ") " << option->rect.width()
                     << "       icon width(" << option->icon.pixmap(option->icon.actualSize(QSize(mIconWidth, mIconHeight))).width() << ")"
                     << "       index(" << index.row() << "," << index.column() << ") "      << index.data(Qt::DisplayRole).toString();
            mSchemaHelpPosition[index.data(Qt::DisplayRole).toString()] =
                        QRect(option->rect.bottomRight().x()-mIconWidth, option->rect.bottomRight().y()-mIconHeight, mIconWidth , mIconHeight);
        } else if (checkstate_index.data( Qt::DisplayRole ).toInt()==4) {
            option->icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
            QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CHECK_STATE );
            if (checkstate_index.data( Qt::DisplayRole ).toInt()==4) {
                qDebug() << "4 icon rec(" << option->rect.x() << "," << option->rect.y() << ") " << option->rect.width()
                         << "       icon width(" << option->icon.pixmap(option->icon.actualSize(QSize(mIconWidth, mIconHeight))).width() << ")"
                         << "       index(" << index.row() << "," << index.column() << ") ";
                mSchemaAppendPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
            }
        }
    } else {
        option->icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CHECK_STATE );
        if (checkstate_index.data( Qt::DisplayRole ).toInt()==4) {
            qDebug() << "4 icon rec(" << option->rect.x() << "," << option->rect.y() << ") " << option->rect.width()
                     << "       icon width(" << option->icon.pixmap(option->icon.actualSize(QSize(mIconWidth, mIconHeight))).width() << ")"
                     << "       index(" << index.row() << "," << index.column() << ") ";
            mSchemaAppendPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
        }
    }
}

bool ConnectDataKeyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type()==QEvent::MouseButtonRelease) {
        const QMouseEvent* const mouseevent = static_cast<const QMouseEvent*>( event );
        const QPoint p = mouseevent->pos();  // ->globalPos()
        qDebug() << "position(" << p.x() << "," << p.y() << ") ";
        bool found = false;
        foreach( QString key, mSchemaHelpPosition.keys() ) {
            QRect rect = mSchemaHelpPosition[key];
            qDebug() << "      check(" << rect.x() << "," << rect.y() << ") ";
            if (rect.contains(p)) {
                qDebug() << "Found Schema! " << key;
                emit requestSchemaHelp(key);
                found = true;
                break;
            }
        }
        if (!found) {
            foreach( QModelIndex idx, mSchemaAppendPosition.keys() ) {
                QRect rect = mSchemaAppendPosition[idx];
                qDebug() << "      check(" << rect.x() << "," << rect.y() << "), index=(" << index.row() << "," <<index.column()<<")"
                         << "      equal?"<< (idx == index? "yes" : "no");
                if (idx == index && rect.contains(p)) {
                    qDebug() << "Found idx! " << idx.row() << "," << idx.column();
                    found = true;
                    break;
                }
            }
        }
        return (index.data( Qt::DisplayRole ).toBool());
    }
    return QStyledItemDelegate::editorEvent(event,model, option, index);

}

} // namespace connect
} // namespace studio
} // namespace gams
