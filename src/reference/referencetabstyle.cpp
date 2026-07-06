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
#include <QStyleOptionTab>
#include <QPainter>
#include "referencetabstyle.h"

namespace gams {
namespace studio {
namespace reference {

ReferenceTabStyle::ReferenceTabStyle(const QString &style)
    : QProxyStyle(style)
{ }

QSize ReferenceTabStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab)
        s.transpose();
    return s;
}

void ReferenceTabStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
        if (element == CE_TabBarTabShape && tab->state & QStyle::State_Selected) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(tab->palette.highlight());
            painter->drawRoundedRect(tab->rect, 4.0, 4.0);
            painter->restore();
            return;
        }
        if (element == CE_TabBarTabLabel) {
            if (tab->state & QStyle::State_Selected) {
                painter->save();
                painter->setPen(tab->palette.highlightedText().color());
                painter->drawText(tab->rect, Qt::AlignCenter, tab->text);
                painter->restore();
                return;
            }
            QStyleOptionTab opt(*tab);
            opt.shape = QTabBar::RoundedNorth;
            QProxyStyle::drawControl(element, &opt, painter, widget);
            return;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

} // namespace reference
} // namespace studio
} // namespace gams
