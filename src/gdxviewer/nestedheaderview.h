#ifndef NESTEDHEADERVIEW_H
#define NESTEDHEADERVIEW_H

#include "tableviewmodel.h"

#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

class NestedHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    NestedHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~NestedHeaderView() override;
    void setModel(QAbstractItemModel *model) override;

    void setDdEnabled(bool value);

public slots:
    void reset() override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sectionSizeFromContents(int logicalIndex) const override;

private:
    int pointToDimension(QPoint p);
    int pointToDropDimension(QPoint p);
    void bindScrollMechanism();
    void decideAcceptDragEvent(QDragMoveEvent* event);
    int toGlobalDim(int localDim, int orientation);

    TableViewModel* sym() const;
    int dim() const;
    QPoint mMousePos = QPoint(-1,-1);
    QPoint mDragStartPosition;

    int dimIdxStart = -1;
    int dimIdxEnd = -1;

    int dragOrientationStart = -1;
    int dragOrientationEnd = -1;

    QVector<int> sectionWidth;
    bool ddEnabled = true;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // NESTEDHEADERVIEW_H
