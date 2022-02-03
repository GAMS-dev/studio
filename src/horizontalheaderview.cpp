#include "horizontalheaderview.h"
#include "theme.h"

#include <QPainter>

namespace gams {
namespace studio {

HorizontalHeaderView::HorizontalHeaderView(QWidget *parent) : QHeaderView(Qt::Horizontal, parent)
{}

HorizontalHeaderView::~HorizontalHeaderView()
{}

void HorizontalHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (Theme::instance()->baseTheme(Theme::instance()->activeTheme()) == 0) {
        paintSectionBorder(painter, rect, logicalIndex);
    }
}

void HorizontalHeaderView::paintSectionBorder(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    Q_UNUSED(logicalIndex)
    QPen pen(painter->pen());
    pen.setColor(palette().midlight().color());
    painter->setPen(pen);
    painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
}

} // namespace studio
} // namespace gams
