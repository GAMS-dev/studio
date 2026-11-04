/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "exception.h"
#include "theme.h"
#include "viewhelper.h"

#include <QStyleOptionTab>
#include <QPainter>

namespace {
constexpr int cTextMargin = 12;
constexpr int cSpacing    = 4;
}

namespace gams {
namespace studio {

QRegularExpression TabBarStyle::RexSystemLog = QRegularExpression(QString("^%1 \\(\\d+\\)$").arg(ViewStrings::SystemLog));

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

// QSize TabBarStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
// {
//     QSize res = QProxyStyle::sizeFromContents(type, option, size, widget);
//     QTabWidget *tabWidget = widget == mMainTabs->tabBar() ? mMainTabs : widget == mLogTabs->tabBar() ? mLogTabs : nullptr;
//     if (tabWidget) {
//         if (widget == mMainTabs->tabBar()) {
//             if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
//                 if (QWidget *wid = tabWidget->widget(tab->tabIndex)) {
//                     QStyleOptionTab opt(*tab);
//                     QSize newSize = size;
//                     opt.text = platformGetText(opt.text, wid);
//                     if (opt.text != tab->text) {
//                         const QFont &f = widget->font();
//                         int diff = QFontMetrics(f).horizontalAdvance(opt.text) - tab->fontMetrics.horizontalAdvance(tab->text);
//                         newSize.setWidth(newSize.width() + diff);
//                         res = QProxyStyle::sizeFromContents(type, &opt, newSize, widget);
//                     }
//                 }
//             }
//         } else if (widget == mLogTabs->tabBar()) {
//             if (type == CT_TabBarTab) {
//                 if (const QStyleOptionTab *tabOption = qstyleoption_cast<const QStyleOptionTab *>(option)) {
//                     const QTabBar *tabBar = qobject_cast<const QTabBar *>(widget);
//                     if (tabBar) {
//                         int tabIndex = tabBar->tabAt(tabOption->rect.center());
//                        if (tabIndex == 0) {
//                             QString text = tabBar->tabText(tabIndex);
//                             if (RexSystemLog.match(text).hasMatch()) {
//                                 // To fix the truncated text of the tab because the bounded box of the bold font
//                                 // has been calculated beforehand during drawControl(),
//                                 // suggesting the amount of pixels needed to fit the bold layout.
//                                 QFont font = qApp->font();
//                                 QFontMetrics fontMetrics(font);
//                                 font.setBold(true);
//                                 QFontMetrics boldMetrics(font);
//                                 int textWidthNormal = fontMetrics.horizontalAdvance(tabOption->text);
//                                 int textWidthBold = boldMetrics.horizontalAdvance(tabOption->text);
//                                 int extraWidth = textWidthBold - textWidthNormal;
//                                 res.setWidth(res.width() + extraWidth);
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
//     return res;
// }


QSize TabBarStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option,
                                    const QSize &size, const QWidget *widget) const
{
    QSize res = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type != CT_TabBarTab)
        return res;

    QTabWidget *tabWidget = widget == mMainTabs->tabBar() ? mMainTabs
                            : widget == mLogTabs->tabBar()  ? mLogTabs : nullptr;
    const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(option);
    if (!tabWidget || !tab)
        return res;
    if (tabWidget->tabPosition() == QTabWidget::West || tabWidget->tabPosition() == QTabWidget::East)
        return res;

    QFont font = widget->font();
    const QString text = platformGetText(tab->text, tabWidget->widget(tab->tabIndex));
    if (tabWidget == mLogTabs && RexSystemLog.match(text).hasMatch())
        font.setBold(true);
    const QFontMetrics fm(font);

    int width = cTextMargin + fm.horizontalAdvance(text) + cTextMargin;
    if (tab->leftButtonSize.width() > 0)
        width += tab->leftButtonSize.width() + cSpacing;
    if (tab->rightButtonSize.width() > 0)
        width += tab->rightButtonSize.width() + cSpacing;
    if (!tab->icon.isNull())
        width += tab->iconSize.width() + cSpacing;

    res.setWidth(width);
    return res;
}

void TabBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option,
                                            QPainter *painter, const QWidget *widget) const
{
    QTabWidget *tabWidget = widget == mMainTabs->tabBar() ? mMainTabs : widget == mLogTabs->tabBar() ? mLogTabs : nullptr;
    if (tabWidget) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            // tab element
            if (element == CE_TabBarTabShape) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(tab->palette.color(QPalette::Mid));
                painter->setBrush(tab->palette.color(tab->state.testFlag(State_Selected) ? Theme::isDark() ? QPalette::AlternateBase : QPalette::Base
                                                                                         : Theme::isDark() ? QPalette::Window : QPalette::AlternateBase ));
                QRect rect = tab->rect;
                if (tab->state.testFlag(State_Selected))
                    rect.setBottom(rect.bottom() + 1);
                painter->drawRoundedRect(rect, 2.0, 2.0);
                painter->restore();
                return;
            }
            // tab text
            if (element == CE_TabBarTabLabel) {
                const bool selected = tab->state.testFlag(State_Selected);
                const TabState state = getState(tabWidget->widget(tab->tabIndex), selected);

                QRect textRect = tab->rect.marginsRemoved(QMargins(12, 0, 12, 0));
                if (int dy = platformGetDyLifter(tabWidget->tabPosition(), selected))
                    textRect.translate(0, dy);
                if (tab->leftButtonSize.width() > 0)
                    textRect.setLeft(textRect.left() + tab->leftButtonSize.width() + 4);
                if (tab->rightButtonSize.width() > 0)
                    textRect.setRight(textRect.right() - tab->rightButtonSize.width() - 4);

                painter->save();
                if (!tab->icon.isNull()) {
                    const bool closeLeft = styleHint(SH_TabBar_CloseButtonPosition, nullptr, widget)
                    == QTabBar::LeftSide;
                    QRect iconRect(QPoint(), tab->iconSize);
                    iconRect.moveCenter(textRect.center());
                    if (closeLeft) {
                        iconRect.moveRight(textRect.right());
                        textRect.setRight(iconRect.left() - 4);
                    } else {
                        iconRect.moveLeft(textRect.left());
                        textRect.setLeft(iconRect.right() + 1 + 4);
                    }
                    const QIcon::Mode mode = tab->state.testFlag(State_Enabled) ? QIcon::Normal : QIcon::Disabled;
                    tab->icon.paint(painter, iconRect, Qt::AlignCenter, mode, selected ? QIcon::On : QIcon::Off);
                }

                // Text
                QString text = platformGetText(tab->text, tabWidget->widget(tab->tabIndex));
                if (widget == mLogTabs->tabBar() && RexSystemLog.match(text).hasMatch()) {
                    QFont font = painter->font();
                    font.setBold(true);
                    painter->setFont(font);
                }
                if (const auto *bar = qobject_cast<const QTabBar *>(widget))
                    text = painter->fontMetrics().elidedText(text, bar->elideMode(), textRect.width());

                painter->setPen(platformGetTextColor(state, selected));
                QProxyStyle::drawItemText(painter, textRect, Qt::AlignVCenter | Qt::AlignLeft,
                                          tab->palette, true, text);
                painter->restore();
                return;
            }
        }
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

void TabBarStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                                const QWidget *widget) const
{
    if (element == PE_FrameTabBarBase) {
        painter->fillRect(option->rect, option->palette.color( Theme::isDark() ? QPalette::Window
                                                                               : QPalette::AlternateBase ));
        painter->setPen(option->palette.color(QPalette::Mid));
        QTabWidget *tabWidget = widget == mMainTabs->tabBar() ? mMainTabs : widget == mLogTabs->tabBar() ? mLogTabs
                                                                                                         : nullptr;
        bool tabsAtBottom = tabWidget && tabWidget->tabPosition() == QTabWidget::South;
        QPoint p1 = tabsAtBottom ? option->rect.topLeft()  : option->rect.bottomLeft();
        QPoint p2 = tabsAtBottom ? option->rect.topRight() : option->rect.bottomRight();
        painter->drawLine(p1, p2);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int TabBarStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (widget == mMainTabs->tabBar() || widget == mLogTabs->tabBar()) {
        if (hint == SH_TabBar_PreferNoArrows)
            return false;
        if (hint == SH_TabBar_ElideMode)
            return Qt::ElideNone;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}


} // namespace studio
} // namespace gams
