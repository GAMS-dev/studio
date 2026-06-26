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
#include "labelfilterdelegate.h"
#include <QPainter>

namespace gams {
namespace studio {
namespace gdxviewer {

LabelFilterDelegate::LabelFilterDelegate(QObject *parent)
        : QStyledItemDelegate{parent}
    {}

void LabelFilterDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();
    if (index.data(Qt::CheckStateRole).toInt() != Qt::Checked) {
        painter->fillRect(opt.rect, opt.palette.color(QPalette::AlternateBase));
    } else {
        painter->fillRect(opt.rect, opt.palette.color(QPalette::Base));
    }
    painter->restore();

    opt.backgroundBrush = Qt::NoBrush;
    QStyledItemDelegate::paint(painter, opt, index);
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
