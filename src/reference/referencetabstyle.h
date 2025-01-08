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
#ifndef REFERENCETABSTYLE_H
#define REFERENCETABSTYLE_H

#include <QProxyStyle>

namespace gams {
namespace studio {
namespace reference {

class ReferenceTabStyle : public QProxyStyle
{
public:
    ReferenceTabStyle(const QString &style);

    virtual QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const override;

    virtual void drawControl(ControlElement element, const QStyleOption *option,
                             QPainter *painter, const QWidget *widget) const override;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // REFERENCETABSTYLE_H
