#ifndef GAMS_STUDIO_HORIZONTALHEADERVIEW_H
#define GAMS_STUDIO_HORIZONTALHEADERVIEW_H

#include <QHeaderView>

namespace gams {
namespace studio {

class HorizontalHeaderView : public QHeaderView
{
public:
    explicit HorizontalHeaderView(QWidget *parent = nullptr);
    virtual ~HorizontalHeaderView() override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void paintSectionBorder(QPainter *painter, const QRect &rect, int logicalIndex) const;

};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_HORIZONTALHEADERVIEW_H
