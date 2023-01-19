/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_HEADERVIEWPROXY_H
#define GAMS_STUDIO_HEADERVIEWPROXY_H

#include <QProxyStyle>

namespace gams {
namespace studio {

class HeaderViewProxy : public QProxyStyle
{
    QColor mSepColor;
    static HeaderViewProxy *mInstance;
    HeaderViewProxy();
public:
    static HeaderViewProxy *instance();
    static void deleteInstance();
    static bool platformShouldDrawBorder();

    void drawControl(ControlElement oCtrElement, const QStyleOption * styleOption, QPainter * painter, const QWidget * widget = nullptr) const override;
    void setSepColor(const QColor &newSepColor);
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_HEADERVIEWPROXY_H
