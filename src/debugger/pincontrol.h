#ifndef GAMS_STUDIO_DEBUGGER_PINCONTROL_H
#define GAMS_STUDIO_DEBUGGER_PINCONTROL_H

#include "file/pexgroupnode.h"
#include <QHash>

namespace gams {
namespace studio {
namespace debugger {

typedef QPair<QString, Qt::Orientation> PinData;

class PinControl
{
    QHash<PExProjectNode*, PinData> mData;
public:
    PinControl();
    void setPinView(const PExProjectNode *project, const QString &file, Qt::Orientation orientation);
    QString pinFile(const PExProjectNode *project) const;
    void clear();
};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_PINCONTROL_H
