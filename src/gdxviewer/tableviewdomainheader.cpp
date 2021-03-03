/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

#include "tableviewdomainheader.h"
#include "gdxsymbol.h"
#include "tableviewmodel.h"
#include "tableviewdomainmodel.h"
#include "theme.h"

#include <QPainter>
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

TableViewDomainHeader::TableViewDomainHeader(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    int maxColumns = GMS_MAX_INDEX_DIM+GMS_VAL_MAX;
    mFilterIconX.resize(maxColumns);
    mFilterIconY.resize(maxColumns);

    int h = sectionSizeFromContents(0).height();
    mFilterIconWidth  = h*ICON_SCALE_FACTOR;
    mFilterIconMargin = h*ICON_MARGIN_FACTOR;
}

TableViewDomainHeader::~TableViewDomainHeader()
{

}

void TableViewDomainHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    TableViewDomainHeader::QHeaderView::paintSection(painter, rect, logicalIndex);
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    opt.rect = rect;
    opt.section = logicalIndex;
    painter->restore();

    TableViewDomainModel* tvDomainModel = static_cast<TableViewDomainModel*>(this->model());
    TableViewModel* tvModel = tvDomainModel->tvModel();
    GdxSymbol* symbol = tvModel->sym();

    // show filter icon
    if (logicalIndex < model()->columnCount()) {
        int domIndex = logicalIndex;
        if (logicalIndex < symbol->dim()) {
            domIndex = tvModel->tvDimOrder().at(domIndex);
        }
        QString iconRes;
        if (symbol->filterActive(domIndex))
            iconRes = iconFilterOn;
        else
            iconRes = iconFilterOff;
        QIcon icon(Theme::icon(iconRes));
        QPixmap pm = icon.pixmap(mFilterIconWidth, mFilterIconWidth);

        int posX = rect.x() + mFilterIconMargin;
        int posY = rect.bottomRight().y()-mFilterIconWidth-mFilterIconMargin;

        painter->drawImage(posX, posY, pm.toImage());

        if (logicalIndex == tvModel->dim() - tvModel->tvColDim()-1)
            painter->drawLine(opt.rect.right(), opt.rect.top(), opt.rect.right(), opt.rect.bottom());
        if (symbol->type() == GMS_DT_PAR && logicalIndex == tvModel->dim()-1)
            painter->drawLine(opt.rect.right(), opt.rect.top(), opt.rect.right(), opt.rect.bottom());

        mFilterIconX[logicalIndex] = posX;
        mFilterIconY[logicalIndex] = posY;
    }
}

void TableViewDomainHeader::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && pointFilterIconCollision(event->pos()))
        this->customContextMenuRequested(event->pos());
    else
        QHeaderView::mousePressEvent(event);
}

bool TableViewDomainHeader::pointFilterIconCollision(QPoint p)
{
    int index = logicalIndexAt(p);

    TableViewDomainModel* tvDomainModel = static_cast<TableViewDomainModel*>(this->model());
    TableViewModel* tvModel = tvDomainModel->tvModel();
    GdxSymbol* symbol = tvModel->sym();

    if (index < model()->columnCount()) {
        if(p.x() >= mFilterIconX[index] && p.x() <= mFilterIconX[index]+mFilterIconWidth &&
           p.y() >= mFilterIconY[index] && p.y() <= mFilterIconY[index]+mFilterIconWidth)
            return true;
    }
    return false;
}

QSize TableViewDomainHeader::sectionSizeFromContents(int logicalIndex) const
{
    QSize s = QHeaderView::sectionSizeFromContents(logicalIndex);
#ifdef __APPLE__
    // do not modify the width on MAC OS since there is already a sufficient width to display the filter icon
#else
    s.setWidth(s.width() + SECTION_WIDTH_FACTOR*(mFilterIconWidth + mFilterIconMargin));
#endif
    return s;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
