/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

#include <QStyleOptionTab>
#include <QPainter>

namespace gams {
namespace studio {

// Special remarks: If this is assigned to mainTabs-tabBar, the drawControl is accidently also called for other tabBars
// (Qt 15.2)        like tabBar and the SettingsDialog tabs. However this isn't true for the sizeFromContents. That
//                  is ONLY called if this Style is additionally applied to e.g. the logTabs-tabBar!
//
// For that reason this class is designed to support both tabBars, the mainTabs and the logTabs. The widget pointers
// to mainTabs and logTabs assures the correct behavior.

TabBarStyle::TabBarStyle(QTabWidget *mainTabs, QTabWidget *logTabs, QStyle *style)
    : QProxyStyle(style), mMainTabs(mainTabs), mLogTabs(logTabs)
{
    if (!mMainTabs || !mLogTabs)
        FATAL() << "MainTabs and LogTabs need to be defined";
    mMainTabs->tabBar()->setStyle(this);
    mLogTabs->tabBar()->setStyle(this);
}

QSize TabBarStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize res = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (widget == mMainTabs->tabBar()) {
        if (const QStyleOptionTabV4 *tab = qstyleoption_cast<const QStyleOptionTabV4 *>(option)) {
            if (QWidget *wid = mMainTabs->widget(tab->tabIndex)) {
                TabState state = getState(wid);
                if (state & tsChanged) {
                    QFont f = widget->font();
                    f.setBold(true);
                    int diff = QFontMetrics(f).horizontalAdvance(tab->text) - tab->fontMetrics.horizontalAdvance(tab->text);
                    res.setWidth(int(res.width() + diff));
                }
            }
        }
    }
    return res;
}

void dumpPalette(QPalette &pal)
{
    QList<int> codes { QPalette::WindowText, QPalette::Button, QPalette::Light, QPalette::Midlight, QPalette::Dark,
                QPalette::Mid, QPalette::Text, QPalette::BrightText, QPalette::ButtonText, QPalette::Base,
                QPalette::Window, QPalette::Shadow, QPalette::Highlight, QPalette::HighlightedText,
                QPalette::Link, QPalette::LinkVisited, QPalette::AlternateBase};
    QStringList names {"WindowText", "Button", "Light", "Midlight", "Dark", "Mid",
                "Text", "BrightText", "ButtonText", "Base", "Window", "Shadow",
                "Highlight", "HighlightedText",
                "Link", "LinkVisited",
                "AlternateBase"};
    for (int i = 0; i < codes.size(); ++i) {
        DEB() << names.at(i) << "  " << pal.color(QPalette::Normal, QPalette::ColorRole(codes.at(i))).name();
    }
}

int platformGetDyLifter(QTabWidget::TabPosition tabPos, bool isCurrent)
{
    int res = 0;
#ifndef __APPLE__
    if (!isCurrent)
        res = tabPos==QTabWidget::North ? 1 : tabPos==QTabWidget::South ? -1 : 0;
#endif
    return res;
}

QColor platformGetTextColor(bool isCurrent)
{
    bool dark = Theme::instance()->baseTheme(Theme::instance()->activeTheme()) == 1;
    QColor res = dark ? Qt::white : Qt::black;
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

void TabBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option,
                                            QPainter *painter, const QWidget *widget) const
{
    QTabWidget *tabWidget = widget == mMainTabs->tabBar() ? mMainTabs : widget == mLogTabs->tabBar() ? mLogTabs : nullptr;
    if (tabWidget && element == CE_TabBarTabLabel) {
        if (const QStyleOptionTabV4 *tab = qstyleoption_cast<const QStyleOptionTabV4 *>(option)) {
            QStyleOptionTabV4 opt(*tab);
            TabState state = tsNone;

            state = getState(tabWidget->widget(tab->tabIndex));
            if (state & tsChanged) {
                opt.text = "";
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            painter->save();
            if (state & tsChanged) {
                QFont f = painter->font();
                f.setBold(true);
                painter->setFont(f);
                painter->setPen(platformGetTextColor(opt.state.testFlag(State_Selected)));
                opt.rect = opt.rect.marginsRemoved(QMargins(12,0,12,0));
                if (int dy = platformGetDyLifter(tabWidget->tabPosition(), opt.state.testFlag(State_Selected))) {
                    opt.rect.moveTop(opt.rect.top() + dy);
                }
                if (opt.leftButtonSize.width() > 0) opt.rect.setLeft(opt.rect.left() + opt.leftButtonSize.width() + 4);
                if (opt.rightButtonSize.width() > 0) opt.rect.setRight(opt.rect.right() - opt.rightButtonSize.width() - 4);
                QProxyStyle::drawItemText(painter, opt.rect, Qt::AlignVCenter|Qt::AlignLeft, tab->palette, true, tab->text);
            }
            painter->restore();
            return;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

TabBarStyle::TabState TabBarStyle::getState(const QWidget *tabWidget) const
{
    if (!tabWidget) return tsNone;
    int res = tsNone;
//    bool changed = false;
//    bool marked = false;
//    bool grouped = false;
//    emit requestState(tabWidget, changed, marked, grouped);
    if (tabWidget->property("changed").toBool()) res = tsChanged;
    if (tabWidget->property("marked").toBool()) res = res + tsMarked;
//    if (tabWidget->property("grouped").toBool()) res = res + tsGrouped;
    return TabState(res);
}

bool TabBarStyle::isBold(int index) const
{
    return index == 2;
}

} // namespace studio
} // namespace gams

