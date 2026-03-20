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
#include "licensebutton.h"

#include <QPainter>

namespace gams {
namespace studio {
namespace support {

LicenseButton::LicenseButton(QWidget *parent)
    : QPushButton(parent)
{}

void LicenseButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF rect = this->rect().adjusted(1, 1, -1, -1);
    qreal cornerRadius = 4.0;

    painter.setBrush(palette().color(QPalette::AlternateBase));
    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(rect, cornerRadius, cornerRadius);
}

} // namespace support
} // namespace studio
} // namespace gams
