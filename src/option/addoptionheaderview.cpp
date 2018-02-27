#include <QPainter>
#include <QTableView>
#include <QMouseEvent>
#include <QtWidgets>

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

bool AddOptionHeaderView::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        if (isAddOptionCoordinate(helpEvent->pos())) {
            QToolTip::showText(helpEvent->globalPos(), "add new option");
        } else {
            QToolTip::hideText();
            event->ignore();
        }

        return true;
    }
    return QWidget::event(event);
}

void AddOptionHeaderView::mousePressEvent(QMouseEvent* event)
{
    if (Qt::LeftButton == event->button() && isAddOptionCoordinate(event->pos())) {
        QTableView* tableView = static_cast<QTableView*>(this->parent());
        tableView->model()->insertRows(tableView->model()->rowCount(), 1, QModelIndex());
    }

   QHeaderView::mousePressEvent(event);
}

bool AddOptionHeaderView::isAddOptionCoordinate(QPoint p)
{
    int index = logicalIndexAt(p);
    if (index != mLogicalIndex)
        return false;

    return (p.x() >= mIconX && p.x() <= mIconX+mIconWidth && p.y() >= mIconY && p.y() <= mIconY+mIconWidth);
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

} // namespace studio
} // namespace gams
