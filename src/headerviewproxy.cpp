#include "headerviewproxy.h"
#include <QPainter>
#include <QStyleOption>

namespace gams {
namespace studio {

HeaderViewProxy *HeaderViewProxy::mInstance = nullptr;

void HeaderViewProxy::setSepColor(const QColor &newSepColor)
{
    mSepColor = newSepColor;
}

HeaderViewProxy::HeaderViewProxy() : QProxyStyle()
{}

HeaderViewProxy *HeaderViewProxy::instance()
{
    if (!mInstance)
        mInstance = new HeaderViewProxy();
    return mInstance;
}

void HeaderViewProxy::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

bool HeaderViewProxy::platformShouldDrawBorder()
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
