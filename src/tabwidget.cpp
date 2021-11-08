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
            int delta = (we->source() == Qt::MouseEventNotSynthesized) ? we->angleDelta().y() : we->angleDelta().x() / 10;
            if (delta > 0) {
                if (!bLeft->isHidden()) bLeft->clicked();
            } else {
                if (!bRight->isHidden()) bRight->clicked();
            }
            return true;
        }
    }
    return false;
}

}
}
