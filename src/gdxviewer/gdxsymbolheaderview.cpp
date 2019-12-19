/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "gdxsymbolheaderview.h"
#include "gdxsymbol.h"
#include "scheme.h"

#include <QPainter>
#include <QTableView>
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolHeaderView::GdxSymbolHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    mFilterIconWidth.resize(GMS_MAX_INDEX_DIM);
    mFilterIconX.resize(GMS_MAX_INDEX_DIM);
    mFilterIconY.resize(GMS_MAX_INDEX_DIM);
}

GdxSymbolHeaderView::~GdxSymbolHeaderView()
{
}

void GdxSymbolHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    GdxSymbolHeaderView::QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    QTableView* tv = static_cast<QTableView*>(this->parent());
    GdxSymbol* symbol = static_cast<GdxSymbol*>(tv->model());

    if (logicalIndex < symbol->dim()) {
        QString iconRes;
        if (symbol->filterActive()[logicalIndex])
            iconRes = iconFilterOn;
        else
            iconRes = iconFilterOff;

        QIcon icon(Scheme::icon(iconRes));
        int iconWidth = rect.height()*ICON_SCALE_FACTOR;
        int iconMargin = rect.height()*ICON_MARGIN_FACTOR;
        QPixmap pm = icon.pixmap(iconWidth, iconWidth);

        int posX = rect.bottomRight().x()-iconWidth-iconMargin;
        int posY = rect.bottomRight().y()-iconWidth-iconMargin;

        painter->drawImage(posX, posY, pm.toImage());

        mFilterIconWidth[logicalIndex] = iconWidth;
        mFilterIconX[logicalIndex] = posX;
        mFilterIconY[logicalIndex] = posY;
    }
}

void GdxSymbolHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && pointFilterIconCollision(event->pos()))
        this->customContextMenuRequested(event->pos());
    else
        QHeaderView::mousePressEvent(event);
}

bool GdxSymbolHeaderView::pointFilterIconCollision(QPoint p)
{
    int index = logicalIndexAt(p);
    QTableView* tv = static_cast<QTableView*>(this->parent());
    GdxSymbol* symbol = static_cast<GdxSymbol*>(tv->model());

    if (index < symbol->dim()) {
        if(p.x() >= mFilterIconX[index] && p.x() <= mFilterIconX[index]+mFilterIconWidth[index] &&
           p.y() >= mFilterIconY[index] && p.y() <= mFilterIconY[index]+mFilterIconWidth[index])
            return true;
    }
    return false;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
