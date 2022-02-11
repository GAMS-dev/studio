#include "headerviewproxy.h"
#include <QPainter>
#include <QStyleOption>

namespace gams {
namespace studio {

HeaderViewProxy::HeaderViewProxy(QColor sepColor) : QProxyStyle(), mSepColor(sepColor)
{}

bool HeaderViewProxy::platformShouldDrawBorder() const
{
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

void HeaderViewProxy::drawControl(ControlElement oCtrElement, const QStyleOption *styleOption, QPainter *painter, const QWidget *widget) const
{
    QProxyStyle::drawControl(oCtrElement, styleOption, painter, widget);
    if (platformShouldDrawBorder() && oCtrElement == QStyle::CE_HeaderSection) {
        painter->save();
        QPen pen(painter->pen());
        pen.setColor(mSepColor);
        painter->setPen(pen);
        QRect rect = styleOption->rect;
        painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
        painter->restore();
    }
}

} // namespace studio
} // namespace gams
