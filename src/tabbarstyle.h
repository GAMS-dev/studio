#ifndef TABBARSTYLE_H
#define TABBARSTYLE_H

#include <QProxyStyle>

namespace gams {
namespace studio {

class TabBarStyle : public QProxyStyle
{
    Q_OBJECT
public:
    TabBarStyle(QStyle *style = nullptr);
    ~TabBarStyle() override {}

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

signals:
    void getState(const QWidget *widget, int index, bool &current, bool &changed, bool &grouped);

private:
    bool isBold(int index) const;
};

} // namespace studio
} // namespace gams

#endif // TABBARSTYLE_H
