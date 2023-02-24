/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include "logger.h"
#include "projecttreemodel.h"
#include "filemeta.h"
#include <QApplication>
#include <QMimeData>

namespace gams {
namespace studio {

const QString ProjectTreeView::ItemModelDataType = "application/x-qabstractitemmodeldatalist";

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
    if (event->mimeData()->formats().contains(ItemModelDataType)) {
        event->accept();
        QByteArray data = event->mimeData()->data(ItemModelDataType);
        QDataStream stream(&data, QIODevice::ReadOnly);
        while (!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;
            pathList << roleDataMap.value(ProjectTreeModel::LocationRole).toString();
            int idNr = roleDataMap.value(ProjectTreeModel::NodeIdRole).toInt();
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
        pathList << FileMeta::pathList(event->mimeData()->urls());
        event->setDropAction(Qt::CopyAction);
    }
    QList<QModelIndex> newSelection;
    emit dropFiles(indexAt(event->pos()), pathList, idList, event->dropAction(), newSelection);
    if (newSelection.isEmpty()) {
        selectionModel()->select(mSelectionBeforeDrag, QItemSelectionModel::ClearAndSelect);
    } else {
        selectionModel()->clearSelection();
        for (QModelIndex idx: qAsConst(newSelection)) {
            selectionModel()->select(idx, QItemSelectionModel::Select);
        }
    }
    mSelectionBeforeDrag.clear();
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
    if ((event->mimeData()->hasUrls() || isIntern) && !event->mouseButtons().testFlag(Qt::RightButton)) {
        if (event->pos().y() > size().height()-50 || event->pos().y() < 50) {
            startAutoScroll();
        } else {
            stopAutoScroll();
        }
        ProjectTreeModel* treeModel = static_cast<ProjectTreeModel*>(model());
        QModelIndex ind = indexAt(event->pos());
        if (!event->keyboardModifiers().testFlag(Qt::ControlModifier) && isIntern) {
            event->setDropAction(Qt::MoveAction);
        } else if (isIntern || FileMeta::hasExistingFile(event->mimeData()->urls())
                            || FileMeta::hasExistingFolder(event->mimeData()->urls())) {
            event->setDropAction(Qt::CopyAction);
        } else {
            event->ignore();
            return;
        }
        bool locked;
        QModelIndex groupInd = treeModel->findProject(ind, &locked);
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
        static_cast<ProjectTreeModel*>(model())->current();
    if (!model()->data(currMi, ProjectTreeModel::IsProjectRole).toBool())
        expandAll();
    QTreeView::selectAll();
}

} // namespace studio
} // namespace gams
