#include "projecttreeview.h"
#include "logger.h"
#include "projecttreemodel.h"

namespace gams {
namespace studio {

ProjectTreeView::ProjectTreeView(QWidget *parent) : QTreeView(parent)
{ }

void ProjectTreeView::focusOutEvent(QFocusEvent *event)
{
    QModelIndex mi = static_cast<ProjectTreeModel*>(model())->current();
    if (currentIndex() != mi)
        setCurrentIndex(mi);
    QTreeView::focusOutEvent(event);
}

} // namespace studio
} // namespace gams
