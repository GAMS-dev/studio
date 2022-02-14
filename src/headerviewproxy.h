#ifndef GAMS_STUDIO_HEADERVIEWPROXY_H
#define GAMS_STUDIO_HEADERVIEWPROXY_H

#include <QProxyStyle>

namespace gams {
namespace studio {

class HeaderViewProxy : public QProxyStyle
{
    QColor mSepColor;
    static HeaderViewProxy *mInstance;
    HeaderViewProxy();
public:
    static HeaderViewProxy *instance();
    static void deleteInstance();
    static bool platformShouldDrawBorder();

    void drawControl(ControlElement oCtrElement, const QStyleOption * styleOption, QPainter * painter, const QWidget * widget = nullptr) const override;
    void setSepColor(const QColor &newSepColor);
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_HEADERVIEWPROXY_H
