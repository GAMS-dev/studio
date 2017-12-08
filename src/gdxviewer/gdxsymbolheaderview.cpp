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
        painter->save();
    }
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
