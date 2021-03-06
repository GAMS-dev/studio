#ifndef TABBARSTYLE_H
#define TABBARSTYLE_H

#include <QProxyStyle>
#include <QTabWidget>

namespace gams {
namespace studio {


class TabBarStyle : public QProxyStyle
{
    enum TabState { tsNormal=0, tsColorAll=1, tsColorMark=2, };
    Q_OBJECT
public:
    TabBarStyle(QTabWidget *mainTabs, QTabWidget *logTabs, QString style = nullptr);
    ~TabBarStyle() override {}

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

private:
    TabState getState(const QWidget *tabWidget, bool selected) const;
    QString platformGetText(const QString &text, const QWidget *tabWidget) const;
    int platformGetDyLifter(QTabWidget::TabPosition tabPos, bool isCurrent) const;
    QColor platformGetTextColor(TabState state, bool isCurrent) const;

private:
    QTabWidget *mMainTabs;
    QTabWidget *mLogTabs;
};

} // namespace studio
} // namespace gams

#endif // TABBARSTYLE_H
