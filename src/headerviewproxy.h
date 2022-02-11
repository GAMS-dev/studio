#ifndef GAMS_STUDIO_HEADERVIEWPROXY_H
#define GAMS_STUDIO_HEADERVIEWPROXY_H

#include <QProxyStyle>

namespace gams {
namespace studio {

class HeaderViewProxy : public QProxyStyle
{
    QColor mSepColor;
public:
    HeaderViewProxy(QColor sepColor);
    bool platformShouldDrawBorder() const;
    void drawControl(ControlElement oCtrElement, const QStyleOption * styleOption, QPainter * painter, const QWidget * widget = nullptr) const override;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_HEADERVIEWPROXY_H
