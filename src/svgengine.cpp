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
#include "svgengine.h"
#include "theme.h"
#include "logger.h"
#include <QPainter>
#include <QStyleOption>
#include <QGuiApplication>
#include <QApplication>

namespace gams {
namespace studio {

SvgEngine::SvgEngine(const QString &name, int alpha)
    : QIconEngine(), mName(name), mNameD(name), mAlpha(qBound(0, alpha, 100))
{
    mController = Theme::instance();
}

SvgEngine::SvgEngine(const QString &name, const QString &disabledName, int alpha)
    : QIconEngine(), mName(name), mNameD(disabledName), mAlpha(qBound(0, alpha, 100))
{
    mController = Theme::instance();
}

SvgEngine::SvgEngine(const SvgEngine &other) : QIconEngine()
{
    mController = other.mController;
    mForceSquare = other.mForceSquare;
    mName = other.mName;
    mNameD = other.mNameD;
    mNormalMode = other.mNormalMode;
}

SvgEngine::~SvgEngine()
{
    if (mController)
        mController->unbind(this);
}

QString SvgEngine::iconName()
{
    //QIconEngine::IconNameHook
    return mName;
}

void SvgEngine::replaceNormalMode(QIcon::Mode mode)
{
    mNormalMode = mode;
}

void SvgEngine::forceSquare(bool force)
{
    mForceSquare = force;
}

void SvgEngine::unbind()
{
    mController = nullptr;
}

void SvgEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state)
    if (mode == QIcon::Normal) mode = mNormalMode;
    const QString &name = (mode == QIcon::Disabled ? mNameD : mName);
    QByteArray &data = mController->data(name, mode, mAlpha);
    QSvgRenderer renderer(data);
    QRect pRect = rect;
    if (mForceSquare) pRect.setWidth(pRect.height());
    renderer.render(painter, pRect);
}

QIconEngine *SvgEngine::clone() const
{
    return new SvgEngine(*this);
}

QPixmap SvgEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    paint(&painter, QRect(0, 0, size.width(), size.height()), mode, state);
    painter.end();
    QPixmap res = QPixmap::fromImage(img, Qt::NoFormatConversion);
//    if (mode == QIcon::Disabled) {
//        QStyleOption opt(0);
//        opt.palette = QGuiApplication::palette();
//        return QApplication::style()->generatedIconPixmap(mode, res, &opt);
//    }
    return  res;
}

} // namespace studio
} // namespace gams
