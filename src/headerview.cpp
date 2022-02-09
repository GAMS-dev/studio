#include "headerview.h"
#include "theme.h"

#include <QPainter>

namespace gams {
namespace studio {

HeaderView::HeaderView(Qt::Orientation orientation, QWidget *parent) : QHeaderView(orientation, parent)
{
    setAutoLineWindows(true);
}

HeaderView::~HeaderView()
{}

void HeaderView::setAutoLineWindows(bool active) {
    mAutoLine = platformShouldDrawBorder() && active;
}

void HeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (mAutoLine && Theme::instance()->baseTheme(Theme::instance()->activeTheme()) == 0) {
        paintSectionBorder(painter, rect);
    }
}

void HeaderView::paintSectionBorder(QPainter *painter, const QRect &rect) const
{
    if (!platformShouldDrawBorder() || orientation() == Qt::Vertical) return;
    painter->save();
    QPen pen(painter->pen());
    pen.setColor(palette().midlight().color());
    painter->setPen(pen);
    painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    painter->restore();
}

bool HeaderView::platformShouldDrawBorder() const
{
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

} // namespace studio
} // namespace gams
