#ifndef GAMS_STUDIO_ABSTRACTVIEW_H
#define GAMS_STUDIO_ABSTRACTVIEW_H

#include <QWidget>
#include <QHeaderView>

#include "common.h"

namespace gams {
namespace studio {

class AbstractView: public QWidget
{
    Q_OBJECT
public:
    AbstractView(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~AbstractView() override;

signals:
    void zoomRequest(int delta);

protected slots:
    void columnsRegister(QHeaderView *header);

protected:
    void columnsResetAll();
    void columnsUpdateScale();
    qreal columnsCurrentScale() const;
    void wheelEvent(QWheelEvent *e) override;
    bool event(QEvent *event) override;


private slots:
    void columnsUnregister(QObject *object);

private:
    typedef QList<qreal> ColumnWidths;
    typedef QHash<QHeaderView*, ColumnWidths> HeadersData;
    HeadersData mHeaders;
    qreal mBaseScale = -1.0;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ABSTRACTVIEW_H
