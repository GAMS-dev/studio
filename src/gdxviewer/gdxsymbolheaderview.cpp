#include "gdxsymbolheaderview.h"
#include <QDebug>
#include <QPainter>
#include <QTableView>
#include <QMouseEvent>
#include <QFlags>
#include <QApplication>

#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolHeaderView::GdxSymbolHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    mFilterIconWidth = new int[GMS_MAX_INDEX_DIM + GMS_VAL_MAX];
    mFilterIconX = new int[GMS_MAX_INDEX_DIM + GMS_VAL_MAX];
    mFilterIconY = new int[GMS_MAX_INDEX_DIM + GMS_VAL_MAX];
}

GdxSymbolHeaderView::~GdxSymbolHeaderView()
{
    if(mFilterIconWidth)
        delete[] mFilterIconWidth;
    if(mFilterIconX)
        delete[] mFilterIconX;
    if(mFilterIconY)
        delete[] mFilterIconY;
}

void GdxSymbolHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    GdxSymbolHeaderView::QHeaderView::paintSection(painter, rect, logicalIndex);

    QTableView* tv = static_cast<QTableView*>(this->parent());
    GdxSymbol* symbol = static_cast<GdxSymbol*>(tv->model());

    if(logicalIndex < symbol->dim())
    {
        painter->restore();
        QString iconRes;
        if(symbol->filterActive()[logicalIndex])
            iconRes = iconFilterOn;
        else
            iconRes = iconFilterOff;

        QIcon icon(iconRes);
        int iconWidth = rect.height()*ICON_SCALE_FACTOR;
        int iconMargin = rect.height()*ICON_MARGIN_FACTOR;
        QPixmap pm = icon.pixmap(iconWidth, iconWidth);

        int posX = rect.bottomRight().x()-iconWidth-iconMargin;
        int posY = rect.bottomRight().y()-iconWidth-iconMargin;

        painter->drawImage(posX, posY, pm.toImage());

        mFilterIconWidth[logicalIndex] = iconWidth;
        mFilterIconX[logicalIndex] = posX;
        mFilterIconY[logicalIndex] = posY;

        painter->save();
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
    if(p.x() >= mFilterIconX[index] && p.x() <= mFilterIconX[index]+mFilterIconWidth[index] &&
       p.y() >= mFilterIconY[index] && p.y() <= mFilterIconY[index]+mFilterIconWidth[index])
    {
        return true;
    }
    return false;

}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
