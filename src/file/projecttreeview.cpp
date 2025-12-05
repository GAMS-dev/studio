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
#include "projecttreeview.h"
#include "projecttreemodel.h"
#include "projectproxymodel.h"
#include "filemeta.h"
#include <QApplication>
#include <QMimeData>

namespace gams {
namespace studio {

const QString ProjectTreeView::ItemModelDataType = "application/x-qabstractitemmodeldatalist";

ProjectTreeView::ProjectTreeView(QWidget *parent) : QTreeView(parent)
{
    setDragDropMode(DragDrop);
    connect(this, &ProjectTreeView::expanded, this, [this](const QModelIndex &index) {
        NodeId nodeId = index.data(ProjectTreeModel::NodeIdRole).toInt();
        mExpandedNodes.insert(nodeId);
        emit expandedChanged(nodeId, true);
    });
    connect(this, &ProjectTreeView::collapsed, this, [this](const QModelIndex &index) {
        NodeId nodeId = index.data(ProjectTreeModel::NodeIdRole).toInt();
        mExpandedNodes.remove(nodeId);
        emit expandedChanged(nodeId, false);
    });
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

void ProjectTreeView::fixFocus(bool delay)
{
    if (delay || mDelayUpdate) {
        mDelayUpdate = delay;
        return;
    }
    QModelIndex mi = static_cast<ProjectProxyModel*>(model())->current();
    if (mi.isValid() && currentIndex() != mi)
        setCurrentIndex(mi);
}

void gams::studio::ProjectTreeView::restoreExpansion(const QModelIndex &index)
{
    NodeId nodeId = index.data(ProjectTreeModel::NodeIdRole).toInt();
    if (mExpandedNodes.contains(nodeId))
        expand(index);
    QModelIndex child = model()->index(0, 0, index);
    int i = 0;
    while (child.isValid()) {
        restoreExpansion(child);
        child = child.sibling(++i, 0);
    }
}

void ProjectTreeView::startDrag(Qt::DropActions supportedActions)
{
    QList<NodeId> nodeIdList;
    for (QModelIndex index : selectionModel()->selection().indexes())
        nodeIdList << index.data(ProjectTreeModel::NodeIdRole).toInt();
    emit getHasRunBlocker(nodeIdList, mHasRunBlocker);
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
    if (event->mimeData()->formats().contains(ItemModelDataType)) {
        event->accept();
        QByteArray data = event->mimeData()->data(ItemModelDataType);
        QDataStream stream(&data, QFile::ReadOnly);
        while (!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;
            pathList << roleDataMap.value(ProjectTreeModel::LocationRole).toString();
            int idNr = roleDataMap.value(ProjectTreeModel::NodeIdRole).toInt();
            if (idNr > 0) idList << NodeId(idNr); // skips the root node
        }
        // [workaround] sometimes the dropAction isn't set correctly
        if (!event->modifiers().testFlag(Qt::ControlModifier) && !mHasRunBlocker)
            event->setDropAction(Qt::MoveAction);
        else
            event->setDropAction(Qt::CopyAction);
    }
    if (event->mimeData()->hasUrls()) {
        event->accept();
        pathList << FileMeta::pathList(event->mimeData()->urls());
        event->setDropAction(Qt::CopyAction);
    }
    QModelIndex index = indexAt(event->position().toPoint());
    if (!index.isValid() && rootIndex().parent().isValid()) // a project is in focus
        index = rootIndex();
    QList<QModelIndex> newSelection;
    emit dropFiles(index, pathList, idList, event->dropAction(), newSelection);
    if (newSelection.isEmpty()) {
        selectionModel()->select(mSelectionBeforeDrag, QItemSelectionModel::ClearAndSelect);
    } else {
        selectionModel()->clearSelection();
        for (QModelIndex idx: std::as_const(newSelection)) {
            selectionModel()->select(idx, QItemSelectionModel::Select);
        }
    }
    mSelectionBeforeDrag.clear();
    mHasRunBlocker = false;
    stopAutoScroll();
}

void ProjectTreeView::updateDrag(QDragMoveEvent *event)
{
    bool isIntern = event->mimeData()->formats().contains(ItemModelDataType);
    if (isIntern) {
        for (QModelIndex index : mSelectionBeforeDrag.indexes()) {
            if (index.isValid() && index.data(ProjectTreeModel::IsGamsSys).toBool()) {
                event->ignore();
                return;
            }
        }
    }
    if ((event->mimeData()->hasUrls() || isIntern) && !event->buttons().testFlag(Qt::RightButton)) {
        if (event->position().y() > size().height()-50 || event->position().y() < 50) {
            startAutoScroll();
        } else {
            stopAutoScroll();
        }
        if (!event->modifiers().testFlag(Qt::ControlModifier) && !mHasRunBlocker && isIntern) {
            event->setDropAction(Qt::MoveAction);
        } else if (isIntern || FileMeta::hasExistingFile(event->mimeData()->urls())
                            || FileMeta::hasExistingFolder(event->mimeData()->urls())) {
            event->setDropAction(Qt::CopyAction);
        } else {
            event->ignore();
            return;
        }
        bool locked;
        QModelIndex ind = indexAt(event->position().toPoint());
        QModelIndex groupInd = static_cast<ProjectProxyModel*>(model())->findProject(ind, &locked);
        if (groupInd.isValid())
            selectionModel()->select(groupInd, QItemSelectionModel::ClearAndSelect);
        (locked) ? event->ignore() : event->accept();
    } else {
        event->ignore();
    }
}

void ProjectTreeView::keyPressEvent(QKeyEvent *event)
{
    if (currentIndex().isValid() && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        event->accept();
        emit activated(currentIndex());
        return;
    }
    QTreeView::keyPressEvent(event);
}

void ProjectTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    QModelIndex ind = indexAt(event->pos());
    if (event->button() == Qt::LeftButton && model()->data(ind, ProjectTreeModel::IsProjectRole).toBool()) {
        QRect rect = visualRect(ind);
        if (rect.isValid() && event->pos().x() > rect.right() - rect.height()) {
            emit openProjectEdit(ind);
        }
    }
    QTreeView::mouseReleaseEvent(event);
}

void ProjectTreeView::selectAll()
{
    QModelIndex currMi = currentIndex();
    if (!currMi.isValid())
        static_cast<ProjectProxyModel*>(model())->current();
    if (!model()->data(currMi, ProjectTreeModel::IsProjectRole).toBool())
        expandAll();
    QTreeView::selectAll();
}

} // namespace studio
} // namespace gams
