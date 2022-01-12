/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
 */
#ifndef GAMS_STUDIO_SVGENGINE_H
#define GAMS_STUDIO_SVGENGINE_H

#include <QIconEngine>
#include <QSvgRenderer>
#include <QIconEnginePlugin>

namespace gams {
namespace studio {

class Theme;

class SvgEngine : public QIconEngine
{
public:
    SvgEngine(const QString &name, int alpha = 100);
    SvgEngine(const QString &name, const QString &disabledName, int alpha = 100);
    SvgEngine(const SvgEngine &other);
    ~SvgEngine() override;
    QString iconName() const override;
    void replaceNormalMode(QIcon::Mode mode);
    void forceSquare(bool force);
    void unbind();
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine * clone() const override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
private:
    Theme *mController = nullptr;
    bool mForceSquare = true;
    QString mName;
    QString mNameD;
    int mAlpha = 100;
    QIcon::Mode mNormalMode = QIcon::Normal;

};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_SVGENGINE_H
