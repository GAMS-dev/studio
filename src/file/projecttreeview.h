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

signals:
    void dropFiles(QModelIndex idx, QStringList files);
    void closeNode(NodeId nodeId);
protected:
    void focusOutEvent(QFocusEvent *event) override;
    void fixFocus();
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void updateDrag(QDragMoveEvent *event);

public slots:
    void selectAll() override;

private:
    QItemSelection mSelectionBeforeDrag;
};

} // namespace studio
} // namespace gams

#endif // PROJECTTREEVIEW_H
