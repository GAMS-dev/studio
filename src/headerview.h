#ifndef GAMS_STUDIO_HEADERVIEW_H
#define GAMS_STUDIO_HEADERVIEW_H

#include <QHeaderView>

namespace gams {
namespace studio {

class HeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit HeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    virtual ~HeaderView() override;
    void setAutoLineWindows(bool active);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void paintSectionBorder(QPainter *painter, const QRect &rect) const;

private:
    bool platformShouldDrawBorder() const;
    bool mAutoLine;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_HEADERVIEW_H
