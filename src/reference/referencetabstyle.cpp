/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "referencetabstyle.h"
#include <QPainter>

namespace gams {
namespace studio {
namespace reference {

void ReferenceTabStyle::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    opt.displayAlignment = Qt::AlignCenter;
    if (option.state & QStyle::State_MouseOver && option.state & QStyle::State_Enabled) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.brush(QPalette::AlternateBase));
        painter->drawRect(option.rect);
        painter->restore();
    }
    QStyledItemDelegate::paint(painter, opt, index);
    painter->save();
    painter->setPen(option.palette.color(QPalette::Mid));
    painter->drawLine(option.rect.left(), option.rect.bottom(), option.rect.right(), option.rect.bottom());
    painter->restore();
}

} // namespace reference
} // namespace studio
} // namespace gams
