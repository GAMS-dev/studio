/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "tabbarstyle.h"
#include "logger.h"
#include "exception.h"
#include "theme.h"
#include "viewhelper.h"

#include <QStyleOptionTab>
#include <QPainter>

namespace gams {
namespace studio {

// Special remarks: If this is assigned to mainTabs-tabBar, the drawControl is accidently also called for other tabBars
// (Qt 15.2)        like logTabs and the SettingsDialog tabs. However this isn't true for the sizeFromContents. That
//                  is ONLY called if this Style is additionally applied to e.g. the logTabs-tabBar!
//
// For that reason this class is designed to support both tabBars, the mainTabs and the logTabs. The widget pointers
// to mainTabs and logTabs assures the correct behavior.

TabBarStyle::TabBarStyle(QTabWidget *mainTabs, QTabWidget *logTabs, const QString &style)
    : QProxyStyle(style), mMainTabs(mainTabs), mLogTabs(logTabs)
{
    if (!mMainTabs || !mLogTabs)
        FATAL() << "MainTabs and LogTabs need to be defined";
    mMainTabs->tabBar()->setStyle(this);
    mLogTabs->tabBar()->setStyle(this);
}

int TabBarStyle::platformGetDyLifter(QTabWidget::TabPosition tabPos, bool isCurrent) const
{
    int res = 0;
#ifndef __APPLE__
    if (!isCurrent)
        res = tabPos==QTabWidget::North ? 1 : tabPos==QTabWidget::South ? -1 : 0;
#else
    Q_UNUSED(tabPos)
    Q_UNUSED(isCurrent)
#endif
    return res;
}

QColor TabBarStyle::platformGetTextColor(TabState state, bool isCurrent) const
{
    bool dark = Theme::instance()->isDark();
    QColor res = dark ? Qt::white : Qt::black;
    if (state & tsColorAll) return dark ? res.darker(160) : QColor(50,50,50); // text slightly grayed out
#ifdef __APPLE__
    if (!isCurrent) {
        res = dark ? res.darker(160) : QColor(50,50,50);
    }
#else
    if (!isCurrent) {
        res = dark ? res.darker(125) : QColor(60,60,60);
    }
#endif
    return res;
}

QString TabBarStyle::platformGetText(const QString &text, const QWidget *tabWidget) const
{
    if (tabWidget && tabWidget->parentWidget() && tabWidget->parentWidget()->parentWidget() == mMainTabs && ViewHelper::modified(tabWidget))
        return text+"*";
    return text;
}

TabBarStyle::TabState TabBarStyle::getState(const QWidget *tabWidget, bool selected) const
{
    int res = tsNormal;
    if (tabWidget && tabWidget->parentWidget()) {
        if (!selected && tabWidget->parentWidget()->parentWidget() == mMainTabs)
            res = tsColorAll;
    }
    return TabState(res);
}

QSize TabBarStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize res = QProxyStyle::sizeFromContents(type, option, size, widget);
    QTabWidget *tabWidget = widget == mMainTabs->tabBar() ? mMainTabs : widget == mLogTabs->tabBar() ? mLogTabs : nullptr;
    if (tabWidget && widget == mMainTabs->tabBar()) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            if (QWidget *wid = tabWidget->widget(tab->tabIndex)) {
                QStyleOptionTab opt(*tab);
                QSize newSize = size;
                opt.text = platformGetText(opt.text, wid);
                if (opt.text != tab->text) {
                    const QFont &f = widget->font();
                    int diff = QFontMetrics(f).horizontalAdvance(opt.text) - tab->fontMetrics.horizontalAdvance(tab->text);
                    newSize.setWidth(newSize.width() + diff);
                    res = QProxyStyle::sizeFromContents(type, &opt, newSize, widget);
                }
            }
        }
    }
    return res;
}

void TabBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option,
                                            QPainter *painter, const QWidget *widget) const
{
    QTabWidget *tabWidget = widget == mMainTabs->tabBar() ? mMainTabs : widget == mLogTabs->tabBar() ? mLogTabs : nullptr;
    if (tabWidget) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {

//            if (element == CE_TabBarTabShape) { // change the color of the background
//                QStyleOptionTabV4 opt(*tab);
//                QProxyStyle::drawControl(element, &opt, painter, widget);
//                painter->save();
//                painter->setBrush(Qt::darkGreen);
//                painter->setPen(Qt::NoPen);
//                painter->drawRect(opt.rect.marginsRemoved(QMargins(1,1,1,1)));
//                painter->restore();
//                return;
//            }

            if (element == CE_TabBarTabLabel) {
                QStyleOptionTab opt(*tab);
                TabState state = tsNormal;

                state = getState(tabWidget->widget(tab->tabIndex), opt.state.testFlag(State_Selected));
                opt.text = "";

                QProxyStyle::drawControl(element, &opt, painter, widget);

                painter->save();
                painter->setPen(platformGetTextColor(state, opt.state.testFlag(State_Selected)));
                opt.rect = opt.rect.marginsRemoved(QMargins(12,0,12,0));
                opt.text = platformGetText(tab->text, tabWidget->widget(tab->tabIndex));
                if (int dy = platformGetDyLifter(tabWidget->tabPosition(), opt.state.testFlag(State_Selected))) {
                    opt.rect.moveTop(opt.rect.top() + dy);
                }
                if (opt.leftButtonSize.width() > 0) opt.rect.setLeft(opt.rect.left() + opt.leftButtonSize.width() + 4);
                if (opt.rightButtonSize.width() > 0) opt.rect.setRight(opt.rect.right() - opt.rightButtonSize.width() - 4);
                if (!opt.icon.isNull()) {
                    opt.rect.setLeft(opt.rect.left() + opt.iconSize.width() + 4);
                }
                QProxyStyle::drawItemText(painter, opt.rect, Qt::AlignVCenter|Qt::AlignLeft, tab->palette, true, opt.text);
                painter->restore();
                return;
            }
        }
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}


} // namespace studio
} // namespace gams

