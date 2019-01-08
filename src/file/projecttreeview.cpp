#include "projecttreeview.h"
#include "logger.h"
#include "projecttreemodel.h"
#include <QApplication>
#include <QMimeData>
#include <QMessageBox>

namespace gams {
namespace studio {

const QString cItemModelData("application/x-qabstractitemmodeldatalist");

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

void ProjectTreeView::startDrag(Qt::DropActions supportedActions)
{
    QTreeView::startDrag(supportedActions | Qt::MoveAction);
}

void ProjectTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    mSelectionBeforeDrag = selectionModel()->selection();
    updateDrag(event);
}

void ProjectTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    updateDrag(event);
}

void ProjectTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    selectionModel()->select(mSelectionBeforeDrag, QItemSelectionModel::ClearAndSelect);
    mSelectionBeforeDrag.clear();
    QTreeView::dragLeaveEvent(event);
}

void ProjectTreeView::dropEvent(QDropEvent *event)
{
    QStringList pathList;
    QList<NodeId> idList;
    if (event->mimeData()->formats().contains(cItemModelData)) {
        event->accept();
        QByteArray data = event->mimeData()->data(cItemModelData);
        QDataStream stream(&data, QIODevice::ReadOnly);
        while (!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;
            pathList << roleDataMap.value(Qt::UserRole).toString();
            int idNr = roleDataMap.value(Qt::UserRole+1).toInt();
            if (idNr > 0) idList << NodeId(idNr); // skips the root node
        }
        // [workaround] sometimes the dropAction isn't set correctly
        if (!event->keyboardModifiers().testFlag(Qt::ControlModifier))
            event->setDropAction(Qt::MoveAction);
        else
            event->setDropAction(Qt::CopyAction);
    }
    if (event->mimeData()->hasUrls()) {
        event->accept();
        for (QUrl url: event->mimeData()->urls()) {
            pathList << url.toLocalFile();
        }
        event->setDropAction(Qt::CopyAction);
    }
    QList<QModelIndex> newSelection;
    emit dropFiles(indexAt(event->pos()), pathList, idList, event->dropAction(), newSelection);
    if (newSelection.isEmpty()) {
        selectionModel()->select(mSelectionBeforeDrag, QItemSelectionModel::ClearAndSelect);
    } else {
        selectionModel()->clearSelection();
        for (QModelIndex idx: newSelection) {
            selectionModel()->select(idx, QItemSelectionModel::Select);
        }
    }
    mSelectionBeforeDrag.clear();
    stopAutoScroll();
}

void ProjectTreeView::updateDrag(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls() || event->mimeData()->formats().contains(cItemModelData)) {
        if (event->pos().y() > size().height()-50 || event->pos().y() < 50) {
            startAutoScroll();
        } else {
            stopAutoScroll();
        }
        ProjectTreeModel* treeModel = static_cast<ProjectTreeModel*>(model());
        QModelIndex ind = indexAt(event->pos());
        if (!event->keyboardModifiers().testFlag(Qt::ControlModifier)
                && event->mimeData()->formats().contains(cItemModelData)) {
            event->setDropAction(Qt::MoveAction);
        } else {
            event->setDropAction(Qt::CopyAction);
        }
        QModelIndex groupInd = treeModel->findGroup(ind);
        selectionModel()->select(groupInd, QItemSelectionModel::ClearAndSelect);
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
