/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "theme.h"

#include <QPainter>
#include <QStylePainter>
#include <QApplication>
#include <QTreeView>
#include <QPainterPath>

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
        painter->fillRect(checkRect, Theme::color(Theme::Normal_Blue));
    } else  {
        QPainterPath path;
        path.addRect(checkRect);
        QPen pen;
        painter->setPen(pen);
        painter->fillPath(path, QBrush(index.data(Qt::BackgroundRole).value<QColor>(), Qt::SolidPattern));
    }

    int level = 0;
    for(QModelIndex i = index; (i = i.parent()).isValid(); level++);

    if (index.column()==0 && level > 0) {
        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt);
        painter->fillRect(textRect, index.data(Qt::BackgroundRole).value<QColor>());

        auto view = qobject_cast<const QTreeView*>(opt.widget);
        int indent = level * (view ? view->indentation() : 10);

       opt.rect.adjust(indent, 0, 0, 0);
       style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

       opt.rect = option.rect;
       opt.rect.setWidth(indent);

       opt.text.clear();
       opt.viewItemPosition = QStyleOptionViewItem::Middle;
       style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    }
}

} // namespace option
} // namespace studio
} // namespace gams
