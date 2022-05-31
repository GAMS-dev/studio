#ifndef GAMS_STUDIO_ABSTRACTVIEW_H
#define GAMS_STUDIO_ABSTRACTVIEW_H

#include <QWidget>
#include <QHeaderView>

namespace gams {
namespace studio {

class AbstractView: public QWidget
{
    Q_OBJECT
public:
    AbstractView(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~AbstractView() override;

protected:
    void columnsRegister(QHeaderView *header);
    void columnsResetAll();
    QList<qreal> columnWidths(QHeaderView *header, qreal scale = 0.0);
    void setScale(qreal scale);

private slots:
    void columnsUnregister(QObject *object);

private:
    void storeColumnWidths();

private:
    typedef QList<qreal> ColumnWidths;
    typedef QHash<QHeaderView*, ColumnWidths> HeadersData;
    HeadersData mHeaders;
    qreal mBaseScale = 1.0;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ABSTRACTVIEW_H
