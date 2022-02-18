/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
 */
#include "tabwidget.h"

#include <QMouseEvent>
#include <QTabBar>

namespace gams {
namespace studio {

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setUsesScrollButtons(true);
    for (QObject *obj : tabBar()->children()) {
        QToolButton *tb = qobject_cast<QToolButton*>(obj);
        if (tb) (bLeft ? bRight : bLeft) = tb;
    }
    tabBar()->installEventFilter(this);
}

void TabWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        int index = tabBar()->tabAt(tabBar()->mapFromGlobal(event->globalPos()));
        emit closeTab(index);
        event->accept();
        return;
    }

    QTabWidget::mouseReleaseEvent(event);
}

bool TabWidget::eventFilter(QObject *sender, QEvent *event)
{
    Q_UNUSED(sender)
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent*>(event);
        if (!we->modifiers().testFlag(Qt::ControlModifier) && bLeft && bRight) {
            int delta = 0;
            if (we->source() == Qt::MouseEventNotSynthesized)
                 delta =  we->angleDelta().y();
            else {
                mWheelSum += we->pixelDelta().x();
                delta = mWheelSum / 20;
                mWheelSum = mWheelSum % 20;
            }

            if (delta > 0) {
                if (!bLeft->isHidden()) bLeft->clicked();
            } else if (delta < 0) {
                if (!bRight->isHidden()) bRight->clicked();
            }
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && me->modifiers().testFlag(Qt::ControlModifier)) {
            int index = tabBar()->tabAt(tabBar()->mapFromGlobal(me->globalPos()));
            Qt::Orientation orient = (me->modifiers().testFlag(Qt::ShiftModifier) ? Qt::Vertical : Qt::Horizontal);
            emit openSplitView(index, orient);
            return true;
        }
    }
    return false;
}

}
}
