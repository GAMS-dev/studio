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
#include "gdxsymbolheaderview.h"
#include "gdxsymbol.h"
#include "theme.h"
#include "tableviewdomainmodel.h"

#include <QPainter>
#include <QTableView>
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolHeaderView::GdxSymbolHeaderView(Qt::Orientation orientation, GdxSymbolHeaderView::HeaderType headerType, QWidget *parent)
    : QHeaderView(orientation, parent), mHeaderType(headerType)
{
    int maxColumns = GMS_MAX_INDEX_DIM+GMS_VAL_MAX;
    mFilterIconX.resize(maxColumns);
    mFilterIconY.resize(maxColumns);

    int h = GdxSymbolHeaderView::sectionSizeFromContents(0).height();
    mFilterIconWidth  = qRound(h * ICON_SCALE_FACTOR);
    mFilterIconMarginLeft = qRound(h*ICON_MARGIN_LEFT);
    mFilterIconMarginBottom = qRound(h*ICON_MARGIN_BOTTOM);

    this->setTextElideMode(Qt::ElideRight);
}

GdxSymbolHeaderView::~GdxSymbolHeaderView()
{
}

void GdxSymbolHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    TableViewModel* tvModel = nullptr;
    GdxSymbol* symbol = nullptr;
    int maxIndex = 0;
    int posX = 0;
    int posY = 0;

    if (mHeaderType == HeaderType::ListView) {
        QTableView* tv = static_cast<QTableView*>(this->parent());
        symbol = static_cast<GdxSymbol*>(tv->model());
        maxIndex = symbol->filterColumnCount();
    } else {
        TableViewDomainModel* tvDomainModel = static_cast<TableViewDomainModel*>(this->model());
        tvModel = tvDomainModel->tvModel();
        symbol = tvModel->sym();
        maxIndex = model()->columnCount();
    }

    // show filter icon
    if (logicalIndex < maxIndex) {
        QString iconRes;
        if (symbol->filterActive(logicalIndex))
            iconRes = iconFilterOn;
        else
            iconRes = iconFilterOff;
        QIcon icon(Theme::icon(iconRes));
        QPixmap pm = icon.pixmap(mFilterIconWidth, mFilterIconWidth);

        posX = rect.x() + mFilterIconMarginLeft;
        posY = rect.bottomRight().y()-mFilterIconWidth-mFilterIconMarginBottom;
        painter->drawImage(posX, posY, pm.toImage());

        if (mHeaderType == HeaderType::TableViewFilter) {
            QStyleOptionHeader opt;
            initStyleOption(&opt);
            opt.rect = rect;
            opt.section = logicalIndex;
            QPen pen(painter->pen());
            pen.setColor(palette().text().color());
            pen.setWidth(2);
            painter->setPen(pen);
            int idx = visualIndex(logicalIndex);
            if (idx == tvModel->dim() - tvModel->tvColDim()-1)
                painter->drawLine(opt.rect.right(), opt.rect.top(), opt.rect.right(), opt.rect.bottom());
            if (symbol->type() != GMS_DT_SET && idx == tvModel->dim()-1)
                painter->drawLine(opt.rect.right(), opt.rect.top(), opt.rect.right(), opt.rect.bottom());
        }
    }
    if (mHeaderType != HeaderType::TableViewFilter && Theme::instance()->baseTheme(Theme::instance()->activeTheme()) == 0) {
        QPen pen(painter->pen());
        pen.setColor(palette().midlight().color());
        painter->setPen(pen);
        painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    }

    mFilterIconX[logicalIndex] = posX;
    mFilterIconY[logicalIndex] = posY;
}

void GdxSymbolHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && pointFilterIconCollision(event->pos()))
        emit this->customContextMenuRequested(event->pos());
    else
        QHeaderView::mousePressEvent(event);
}

bool GdxSymbolHeaderView::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        int h = sectionSizeFromContents(0).height();
        mFilterIconWidth  = qRound(h * ICON_SCALE_FACTOR);
        mFilterIconMarginLeft = qRound(h*ICON_MARGIN_LEFT);
        mFilterIconMarginBottom = qRound(h*ICON_MARGIN_BOTTOM);
    }
    return QHeaderView::event(event);
}

bool GdxSymbolHeaderView::pointFilterIconCollision(QPoint p)
{
    int maxIndex = 0;
    if (mHeaderType == HeaderType::ListView)
        maxIndex = static_cast<GdxSymbol*>(this->model())->filterColumnCount();
    else
        maxIndex = model()->columnCount();
    int index = logicalIndexAt(p);
    if (index < maxIndex) {
        if(p.x() >= mFilterIconX[index] && p.x() <= mFilterIconX[index]+mFilterIconWidth &&
           p.y() >= mFilterIconY[index] && p.y() <= mFilterIconY[index]+mFilterIconWidth)
            return true;
    }
    return false;
}

QSize GdxSymbolHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize s = QHeaderView::sectionSizeFromContents(logicalIndex);
    int width = s.width();
#ifdef __APPLE__
    width += SECTION_WIDTH_FACTOR_MACOS*(mFilterIconWidth + mFilterIconMarginLeft);
#else
    width += SECTION_WIDTH_FACTOR      *(mFilterIconWidth + mFilterIconMarginLeft);
#endif
    s.setWidth(width);
    return s;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
