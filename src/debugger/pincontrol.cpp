#include "pincontrol.h"

namespace gams {
namespace studio {
namespace debugger {

PinControl::PinControl()
{}

void PinControl::setPinView(const PExProjectNode *project, const QString &file, Qt::Orientation orientation)
{
    mData.insert(project, PinData(file, orientation));
}

QString PinControl::pinFile(const PExProjectNode *project) const
{
    if (mData.contains(project))
        return mData.value(project).first;
    mData.value(nullptr).first;
}

void PinControl::clear()
{
    PinData defaultData = mData.value(nullptr);
    mData.clear();
    mData.insert(nullptr, defaultData);
}

} // namespace debugger
} // namespace studio
} // namespace gams
