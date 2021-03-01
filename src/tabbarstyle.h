#ifndef TABBARSTYLE_H
#define TABBARSTYLE_H

#include <QProxyStyle>
#include <QTabWidget>

namespace gams {
namespace studio {


class TabBarStyle : public QProxyStyle
{
    enum TabState { tsNone, tsChanged, tsMarked, tsChangedMarked,
                    tsGrouped, tsGroupedChanged, tsGroupedMarked, tsGroupedChangedMarked, };
    Q_OBJECT
public:
    TabBarStyle(QTabWidget *mainTabs, QTabWidget *logTabs, QStyle *style = nullptr);
    ~TabBarStyle() override {}

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

private:
    TabState getState(const QWidget *tabWidget) const;
    bool isBold(int index) const;

private:
    QTabWidget *mMainTabs;
    QTabWidget *mLogTabs;
};

} // namespace studio
} // namespace gams

#endif // TABBARSTYLE_H
