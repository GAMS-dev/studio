/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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
    TabBarStyle(QTabWidget *mainTabs, QTabWidget *logTabs, const QString &style = nullptr);
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
