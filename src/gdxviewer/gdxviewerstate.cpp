#include "gdxviewerstate.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewerState::GdxViewerState()
{

}

GdxViewerState::~GdxViewerState()
{
    for (QString name : mSymbolViewState.keys()) {
        deleteSymbolViewState(name);
    }
}

QString GdxViewerState::symbolTableFilter() const
{
    return mSymbolTableFilter;
}

void GdxViewerState::setSymbolTableFilter(const QString &symbolTableFilter)
{
    mSymbolTableFilter = symbolTableFilter;
}

bool GdxViewerState::allColumnsChecked() const
{
    return mAllColumnsChecked;
}

void GdxViewerState::setAllColumnsChecked(bool allColumnsChecked)
{
    mAllColumnsChecked = allColumnsChecked;
}

GdxSymbolViewState* GdxViewerState::symbolViewState(QString name) const
{
    if (mSymbolViewState.contains(name))
        return mSymbolViewState[name];
    else
        return NULL;
}

GdxSymbolViewState* GdxViewerState::addSymbolViewState(QString name)
{
    mSymbolViewState[name] = new GdxSymbolViewState();
    return mSymbolViewState[name];
}

void GdxViewerState::deleteSymbolViewState(QString name)
{
    delete mSymbolViewState[name];
    mSymbolViewState.remove(name);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
