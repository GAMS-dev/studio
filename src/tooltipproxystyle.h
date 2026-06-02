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
#ifndef TOOLTIPPROXYSTYLE_H
#define TOOLTIPPROXYSTYLE_H

#include <QProxyStyle>
#include <QToolTip>

#include "theme.h"

namespace gams {
namespace studio {

class TooltipProxyStyle : public QProxyStyle
{
public:
    TooltipProxyStyle(const QString &styleName)
        : QProxyStyle(styleName) {}

    // overriding the widget-specific polish hook
    void polish(QWidget *widget) override {
        QProxyStyle::polish(widget);

        // identify the internal tooltip label by its class name
        if (widget && widget->metaObject()->className() == QStringLiteral("QTipLabel")) {
            QPalette palette = widget->palette();
            palette.setColor(QPalette::All, QPalette::ToolTipBase, Theme::color(Theme::Window_tooltipBase));
            palette.setColor(QPalette::All, QPalette::ToolTipText, Theme::color(Theme::Window_tooltipText));
            widget->setPalette(palette);
        }
    }
};

} // namespace studio
} // namespace gams

#endif // TOOLTIPPROXYSTYLE_H
