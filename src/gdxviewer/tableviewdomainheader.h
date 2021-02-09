#ifndef GAMS_STUDIO_GDXVIEWER_TABLEVIEWDOMAINHEADER_H
#define GAMS_STUDIO_GDXVIEWER_TABLEVIEWDOMAINHEADER_H

#include <QHeaderView>

namespace gams {
namespace studio {
namespace gdxviewer {

class TableViewDomainHeader : public QHeaderView
{
    Q_OBJECT

public:
    TableViewDomainHeader(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~TableViewDomainHeader() override;
    QSize sectionSizeFromContents(int logicalIndex) const override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent * event) override;

private:
    bool pointFilterIconCollision(QPoint p);

private:
    QString iconFilterOn = ":/img/filter";
    QString iconFilterOff = ":/img/filter-off";
    const double ICON_SCALE_FACTOR = 0.5;
    const double ICON_MARGIN_FACTOR = 0.1;
    const double SECTION_WIDTH_FACTOR = 1.5;

    int mFilterIconWidth;
    int mFilterIconMargin;
    mutable std::vector<int> mFilterIconX;
    mutable std::vector<int> mFilterIconY;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_TABLEVIEWDOMAINHEADER_H
