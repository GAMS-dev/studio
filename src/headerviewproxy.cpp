/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "headerviewproxy.h"
#include "theme.h"
#include <QPainter>
#include <QStyleOption>
#include <QApplication>

namespace gams {
namespace studio {

HeaderViewProxy *HeaderViewProxy::mInstance = nullptr;

void HeaderViewProxy::setSepColor(const QColor &newSepColor)
{
    mSepColor = newSepColor;
}

HeaderViewProxy::HeaderViewProxy() : QProxyStyle()
{}

HeaderViewProxy *HeaderViewProxy::instance()
{
    if (!mInstance)
        mInstance = new HeaderViewProxy();
    return mInstance;
}

void HeaderViewProxy::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

bool HeaderViewProxy::platformShouldDrawBorder()
{
#ifdef _WIN64
    return true;
#else
    return false;
#endif
}

void HeaderViewProxy::drawControl(ControlElement oCtrElement, const QStyleOption *styleOption, QPainter *painter, const QWidget *widget) const
{
    if (Theme::instance()->baseTheme(Theme::instance()->activeTheme()) == 1) {
        QApplication::style()->drawControl(oCtrElement, styleOption, painter, widget);
        return;
    }
    QApplication::style()->drawControl(oCtrElement, styleOption, painter, widget);
    if (platformShouldDrawBorder() && oCtrElement == QStyle::CE_HeaderSection) {
        painter->save();
        QPen pen(painter->pen());
        pen.setColor(mSepColor);
        painter->setPen(pen);
        QRect rect = styleOption->rect;
        painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
        painter->restore();
    }
}

} // namespace studio
} // namespace gams
