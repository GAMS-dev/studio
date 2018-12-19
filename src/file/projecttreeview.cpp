#include "projecttreeview.h"
#include "logger.h"
#include "projecttreemodel.h"
#include <QApplication>
#include <QMimeData>
#include <QMessageBox>

namespace gams {
namespace studio {

ProjectTreeView::ProjectTreeView(QWidget *parent) : QTreeView(parent)
{
    setDragDropMode(DragDrop);
}

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

void ProjectTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    mCurrentBeforeDrag = static_cast<ProjectTreeModel*>(model())->current();
    updateDrag(event);
}

void ProjectTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    updateDrag(event);
}

void ProjectTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    static_cast<ProjectTreeModel*>(model())->setCurrent(mCurrentBeforeDrag);
    mCurrentBeforeDrag = QModelIndex();
    QTreeView::dragLeaveEvent(event);
}

void ProjectTreeView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->accept();
        QStringList pathList;
        for (QUrl url: event->mimeData()->urls()) {
            pathList << url.toLocalFile();
        }
        emit dropFiles(indexAt(event->pos()), pathList);
    }
    static_cast<ProjectTreeModel*>(model())->setCurrent(mCurrentBeforeDrag);
    mCurrentBeforeDrag = QModelIndex();
    stopAutoScroll();
}

void ProjectTreeView::updateDrag(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        if (event->pos().y() > size().height()-50 || event->pos().y() < 50) {
            startAutoScroll();
        } else {
            stopAutoScroll();
        }
        ProjectTreeModel* m = static_cast<ProjectTreeModel*>(model());
        QModelIndex ind = indexAt(event->pos());
        if (ind.isValid()) {
            event->setDropAction(Qt::TargetMoveAction);
        } else {
            event->setDropAction(Qt::CopyAction);
        }
        m->setCurrent(ind);
        event->accept();
    } else {
        event->ignore();
    }
}

void ProjectTreeView::selectAll()
{
    expandAll();
    QTreeView::selectAll();
}

} // namespace studio
} // namespace gams
