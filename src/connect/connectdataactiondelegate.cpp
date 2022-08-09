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

ConnectDataActionDelegate::ConnectDataActionDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
    mIconWidth = 16;
    mIconHeight = 16;
}

ConnectDataActionDelegate::~ConnectDataActionDelegate()
{
    mDeleteActionPosition.clear();
    mMoveUpActionPosition.clear();
    mMoveDownActionPosition.clear();
}

void ConnectDataActionDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    mDeleteActionPosition.clear();
    mMoveUpActionPosition.clear();
    mMoveDownActionPosition.clear();

    QStyledItemDelegate::initStyleOption(option, index);
    option->text = "";

    if (index.parent().isValid()) {
        QIcon icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        if (!icon.isNull()) {
            option->icon = icon;
            if (index.data( Qt::DisplayRole ).toBool()) {
                if (index.column()==(int)DataItemColumn::DELETE)
                    mDeleteActionPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
                else if (index.column()==(int)DataItemColumn::MOVE_DOWN)
                        mMoveDownActionPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
                else if (index.column()==(int)DataItemColumn::MOVE_UP)
                        mMoveUpActionPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
            }
        }
    } else {

    }
}

bool ConnectDataActionDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    qDebug() << "editorEvent " << index.column();
    if (event->type()==QEvent::MouseButtonRelease) {
        const QMouseEvent* const mouseevent = static_cast<const QMouseEvent*>( event );
        const QPoint p = mouseevent->pos();  // ->globalPos()
        if (index.data( Qt::DisplayRole ).toBool()) {
            bool found = false;
            if (index.column()==(int)DataItemColumn::DELETE) {
                foreach( QModelIndex idx, mDeleteActionPosition.keys() ) {
                    QRect rect = mDeleteActionPosition[idx];
                    qDebug() << "      check(" << rect.x() << "," << rect.y() << ") ";
                    if (idx == index && rect.contains(p)) {
                        qDebug() << "Found delete action! " << idx;
                        found = true;
                        break;
                    }
                }
            } else if (index.column()==(int)DataItemColumn::MOVE_DOWN) {
                      foreach( QModelIndex idx, mMoveDownActionPosition.keys() ) {
                          QRect rect = mMoveDownActionPosition[idx];
                          qDebug() << "      check(" << rect.x() << "," << rect.y() << ") ";
                          if (idx == index && rect.contains(p)) {
                              qDebug() << "Found MoveDown action! " << idx;
                              found = true;
                              break;
                          }
                      }
            }   else if (index.column()==(int)DataItemColumn::MOVE_UP) {
                         foreach( QModelIndex idx, mMoveUpActionPosition.keys() ) {
                             QRect rect = mMoveUpActionPosition[idx];
                             qDebug() << "      check(" << rect.x() << "," << rect.y() << ") ";
                             if (idx == index && rect.contains(p)) {
                                 qDebug() << "Found MoveUp action! " << idx;
                                 found = true;
                                 break;
                             }
                         }
            }
            return found;
        }
        return false;
    }
    return QStyledItemDelegate::editorEvent(event,model, option, index);
}

} // namespace connect
} // namespace studio
} // namespace gams
