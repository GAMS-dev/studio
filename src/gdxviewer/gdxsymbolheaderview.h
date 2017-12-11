#ifndef GDXSYMBOLHEADERVIEW_H
#define GDXSYMBOLHEADERVIEW_H

#include <QHeaderView>
#include <QString>
#include <QIcon>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    GdxSymbolHeaderView(Qt::Orientation orientation, QWidget *parent=nullptr);
    ~GdxSymbolHeaderView();

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent * event) override;

private:
    QString iconFilterOn = ":/img/filter";
    QString iconFilterOff = ":/img/filter-off";
    const double ICON_SCALE_FACTOR = 0.5;
    const double ICON_MARGIN_FACTOR = 0.1;

    int* mFilterIconWidth;
    int* mFilterIconX;
    int* mFilterIconY;

    bool pointFilterIconCollision(QPoint p);
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GDXSYMBOLHEADERVIEW_H
