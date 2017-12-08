#include "gdxsymbolheaderview.h"
#include <QDebug>
#include <QPainter>
#include <QTableView>

#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolHeaderView::GdxSymbolHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
}

void GdxSymbolHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    GdxSymbolHeaderView::QHeaderView::paintSection(painter, rect, logicalIndex);
/*
    QTableView* tv = static_cast<QTableView*>(this->parent());
    GdxSymbol* symbol = static_cast<GdxSymbol*>(tv->model());

    if(logicalIndex < symbol->dim())
    {
        painter->restore();
        QString iconRes;
        if(symbol->filterActive()[logicalIndex])
            iconRes = ":/img/filter";
        else
            iconRes = ":/img/filter";

        QIcon icon(iconRes);
        int iconWidth = rect.height()/2;
        QPixmap pm = icon->pixmap(iconWidth, iconWidth);

        painter->drawImage(rect.bottomRight().x()-iconWidth-2, rect.bottomRight().y()-iconWidth-2, pm.toImage());
        painter->save();
    }
    */
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
