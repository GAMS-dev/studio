#include "tabwidget.h"

#include <QMouseEvent>
#include <QTabBar>

namespace gams {
namespace studio {

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
{

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

}
}
