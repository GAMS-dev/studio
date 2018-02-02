/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef GDXSYMBOLHEADERVIEW_H
#define GDXSYMBOLHEADERVIEW_H

#include <QHeaderView>
#include <QString>
#include <QIcon>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    GdxSymbolHeaderView(Qt::Orientation orientation, QWidget *parent=nullptr);
    ~GdxSymbolHeaderView();

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent * event) override;

private:
    QString iconFilterOn = ":/img/filter";
    QString iconFilterOff = ":/img/filter-off";
    const double ICON_SCALE_FACTOR = 0.5;
    const double ICON_MARGIN_FACTOR = 0.1;

    int* mFilterIconWidth;
    int* mFilterIconX;
    int* mFilterIconY;

    bool pointFilterIconCollision(QPoint p);
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GDXSYMBOLHEADERVIEW_H
