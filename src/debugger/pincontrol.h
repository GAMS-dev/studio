#ifndef GAMS_STUDIO_DEBUGGER_PINCONTROL_H
#define GAMS_STUDIO_DEBUGGER_PINCONTROL_H

#include "file/pexgroupnode.h"
#include <QHash>

namespace gams {
namespace studio {

namespace pin {
class PinViewWidget;
}

namespace debugger {

struct PinData;

class PinControl: public QObject
{
    Q_OBJECT
public:
    PinControl(QObject *parent = nullptr);
    void init(gams::studio::pin::PinViewWidget *pinView);
    void closedPinView();
    void setPinView(PExProjectNode *project, QWidget *pinChild, FileMeta *fileMeta);
    bool hasPinChild(PExProjectNode *project) const;
    void projectSwitched(PExProjectNode *project);
    void resetToInitialView();

private slots:
    void removeView(QObject *editWidget);

private:
    PinData *pinChild(PExProjectNode *project) const;
    bool hasPinView();
    void debugData(QString text);

private:
    pin::PinViewWidget *mPinView = nullptr;
    PExProjectNode *mLastProject = nullptr;
    QWidget *mCurrentWidget = nullptr;
    QHash<PExProjectNode*, PinData*> mData;
};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_PINCONTROL_H
