#ifndef GDXSYMBOLHEADERVIEW_H
#define GDXSYMBOLHEADERVIEW_H

#include <QHeaderView>
#include <QIcon>


namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    GdxSymbolHeaderView(Qt::Orientation orientation, QWidget *parent=nullptr);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GDXSYMBOLHEADERVIEW_H
