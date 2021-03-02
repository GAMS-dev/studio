#ifndef TABBARSTYLE_H
#define TABBARSTYLE_H

#include <QProxyStyle>
#include <QTabWidget>

namespace gams {
namespace studio {


class TabBarStyle : public QProxyStyle
{
    enum TabState { tsNormal, tsBold, tsColor1, tsColor1Bold,
                    tsColor2, tsColor2Bold, tsColor12, tsColor12Bold, };
    Q_OBJECT
public:
    TabBarStyle(QTabWidget *mainTabs, QTabWidget *logTabs, QStyle *style = nullptr);
    ~TabBarStyle() override {}

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

private:
    TabState getState(const QWidget *tabWidget, bool selected) const;
    int platformGetDyLifter(QTabWidget::TabPosition tabPos, bool isCurrent) const;
    QColor platformGetTextColor(TabState state, bool isCurrent) const;

private:
    QTabWidget *mMainTabs;
    QTabWidget *mLogTabs;
};

} // namespace studio
} // namespace gams

#endif // TABBARSTYLE_H
