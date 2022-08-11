#ifndef GAMS_STUDIO_GDXVIEWER_QUICKSELECTLISTVIEW_H
#define GAMS_STUDIO_GDXVIEWER_QUICKSELECTLISTVIEW_H

#include <QListView>
#include <QObject>

namespace gams {
namespace studio {
namespace gdxviewer {

class QuickSelectListView : public QListView
{
    Q_OBJECT

public:
    QuickSelectListView(QWidget *parent = nullptr);

signals:
    void quickSelect();

protected:
    void mouseReleaseEvent(QMouseEvent * event) override;

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_QUICKSELECTLISTVIEW_H
