#include <QPainter>
#include <QTableView>
#include <QMouseEvent>

#include "addoptionheaderview.h"

namespace gams {
namespace studio {


AddOptionHeaderView::AddOptionHeaderView(Qt::Orientation orientation, QWidget *parent) :
        QHeaderView(orientation, parent)
{

}

AddOptionHeaderView::~AddOptionHeaderView()
{

}

void AddOptionHeaderView::paintSection(QPainter* painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == 0) {
         QIcon icon(iconStr);
         int iconWidth = rect.height()*ICON_SCALE_FACTOR;
         int iconMargin = (rect.height() - iconWidth)*ICON_MARGIN_FACTOR;
         QPixmap pm = icon.pixmap(iconWidth, iconWidth);

         int posX = rect.topLeft().x() + iconMargin;
         int posY = rect.topLeft().y() + iconMargin;

         painter->drawImage(posX, posY, pm.toImage());

         mIconWidth = iconWidth;
         mIconX = posX;
         mIconY= posY;
         mLogicalIndex = logicalIndex;
    }
}

void AddOptionHeaderView::mousePressEvent(QMouseEvent* event)
{
    if (Qt::LeftButton == event->button() && addOptionIconCollision(event->pos())) {
        QTableView* tableView = static_cast<QTableView*>(this->parent());
        tableView->model()->insertRows(tableView->model()->rowCount(), 1, QModelIndex());
    }

   QHeaderView::mousePressEvent(event);
}

bool AddOptionHeaderView::addOptionIconCollision(QPoint p)
{
    int index = logicalIndexAt(p);
    QTableView* tv = static_cast<QTableView*>(this->parent());

    if (index != mLogicalIndex)
        return false;

    return (p.x() >= mIconX && p.x() <= mIconX+mIconWidth && p.y() >= mIconY && p.y() <= mIconY+mIconWidth);
}

} // namespace studio
} // namespace gams
