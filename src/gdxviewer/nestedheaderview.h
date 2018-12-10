#ifndef NESTEDHEADERVIEW_H
#define NESTEDHEADERVIEW_H

#include "gdxsymbol.h"

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
    void init();

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    QSize sectionSizeFromContents(int logicalIndex) const;

private:
    void calcSectionSize();

    GdxSymbol* sym() const;
    int dim() const;
    QVector<int> mMaxSectionWidth;
    QPoint mMousePos = QPoint(-1,-1);
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // NESTEDHEADERVIEW_H
