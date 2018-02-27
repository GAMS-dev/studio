#ifndef ADDOPTIONHEADERVIEW_H
#define ADDOPTIONHEADERVIEW_H

#include <QHeaderView>

namespace gams {
namespace studio {

class AddOptionHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    AddOptionHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~AddOptionHeaderView();

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

private:
    const QString iconStr = ":/img/plus";
    const double ICON_SCALE_FACTOR = 0.6;
    const double ICON_MARGIN_FACTOR = 0.5;

    mutable int mIconWidth;
    mutable int mIconX;
    mutable int mIconY;
    mutable int mLogicalIndex;

    bool isAddOptionCoordinate(QPoint p);
};

} // namespace studio
} // namespace gams

#endif // ADDOPTIONHEADERVIEW_H
