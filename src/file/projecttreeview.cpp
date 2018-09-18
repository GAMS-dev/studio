#include "projecttreeview.h"
#include "logger.h"
#include "projecttreemodel.h"
#include <QApplication>

namespace gams {
namespace studio {

ProjectTreeView::ProjectTreeView(QWidget *parent) : QTreeView(parent)
{ }

void ProjectTreeView::focusOutEvent(QFocusEvent *event)
{
    QWidget *wid = qApp->focusWidget();
    while (wid) {
        if (wid == this) break;
        wid = wid->parentWidget();
    }
    if (wid != this) fixFocus();
    QTreeView::focusOutEvent(event);
}

void ProjectTreeView::fixFocus()
{
    QModelIndex mi = static_cast<ProjectTreeModel*>(model())->current();
    if (mi.isValid() && currentIndex() != mi)
        setCurrentIndex(mi);
}

} // namespace studio
} // namespace gams
