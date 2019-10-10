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
#include "definitionitemdelegate.h"
#include <QPainter>
#include <QStylePainter>
#include <QApplication>

namespace gams {
namespace studio {
namespace option {

DefinitionItemDelegate::DefinitionItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void DefinitionItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    QStyleOptionViewItem opt(option);
    initStyleOption( &opt, index);

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    QRect checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
    if (opt.checkState == Qt::Checked)  {
        painter->fillRect(checkRect,QBrush(QColor(0, 128, 0, 128)));
    } else  {
        QPainterPath path;
        path.addRect(checkRect);
        QPen pen(QBrush(Qt::white, Qt::SolidPattern), 1);
        painter->setPen(pen);
        painter->fillPath(path, QBrush(Qt::white, Qt::SolidPattern));
    }
}

} // namespace option
} // namespace studio
} // namespace gams
