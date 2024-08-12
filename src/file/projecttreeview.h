/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include "common.h"
#include <QTreeView>

namespace gams {
namespace studio {

class ProjectTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit ProjectTreeView(QWidget *parent = nullptr);
    static const QString ItemModelDataType;

signals:
    void dropFiles(QModelIndex idx, QStringList files, QList<gams::studio::NodeId> knownIds, Qt::DropAction,
                   QList<QModelIndex> &newSelection);
    void openProjectEdit(QModelIndex idx);
    void getHasRunBlocker(const QList<NodeId> ids, bool &runBlocked);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void fixFocus();
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void updateDrag(QDragMoveEvent *event);
    void keyPressEvent(QKeyEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
    void selectAll() override;

private:
    QItemSelection mSelectionBeforeDrag;
    bool mHasRunBlocker = false;


};

} // namespace studio
} // namespace gams

#endif // PROJECTTREEVIEW_H
