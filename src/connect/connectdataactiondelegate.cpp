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
    QStyledItemDelegate::initStyleOption(option, index);
    option->text = "";

    QIcon icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
    if (!icon.isNull()) {
        option->icon = icon;
        if ( index.data( Qt::DisplayRole ).toBool() ) {
            if (index.column()==(int)DataItemColumn::Delete)
                mDeleteActionPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
            else if (index.column()==(int)DataItemColumn::MoveDown)
                    mMoveDownActionPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
            else if (index.column()==(int)DataItemColumn::MoveUp)
                    mMoveUpActionPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
        }
    }
}

bool ConnectDataActionDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    qDebug() << "editorEvent " << index.row() << "," << index.column();
    if (event->type()==QEvent::MouseButtonRelease) {
        const QMouseEvent* const mouseevent = static_cast<const QMouseEvent*>( event );
        const QPoint p = mouseevent->pos();  // ->globalPos()
        if (index.data( Qt::DisplayRole ).toBool() ) {
            bool found = false;
            if (index.column()==(int)DataItemColumn::Delete) {
                QMap<QModelIndex, QRect>::iterator it;
                for (it= mDeleteActionPosition.begin();  it != mDeleteActionPosition.end(); ++it) {
                    QRect rect = mDeleteActionPosition[it.key()];
                    if (rect.contains(p)) {
                        emit requestDeleteItem(index);
                        found = true;
                        break;
                    }
                }
                if (found)
                    mDeleteActionPosition.erase(it);
            } else if (index.column()==(int)DataItemColumn::MoveDown) {
                      QMap<QModelIndex, QRect>::iterator it;
                      for (it= mMoveDownActionPosition.begin();  it != mMoveDownActionPosition.end(); ++it) {
                          QRect rect = mMoveDownActionPosition[it.key()];
                          if (it.key() == index && rect.contains(p)) {
                              emit requestMoveDownItem(index);
                              found = true;
                              break;
                          }
                      }
                      if (found)
                          mMoveDownActionPosition.erase(it);
            }   else if (index.column()==(int)DataItemColumn::MoveUp) {
                         QMap<QModelIndex, QRect>::iterator it;
                         for (it= mMoveUpActionPosition.begin();  it != mMoveUpActionPosition.end(); ++it) {
                             QRect rect = mMoveUpActionPosition[it.key()];
                             if (it.key() == index && rect.contains(p)) {
                                 emit requestMoveUpItem(index);
                                 found = true;
                                 break;
                             }
                         }
                         if (found)
                             mMoveUpActionPosition.erase(it);
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
