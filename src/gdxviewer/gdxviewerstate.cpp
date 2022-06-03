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

QByteArray GdxViewerState::symbolTableHeaderState() const
{
    return mSymbolTableHeaderState;
}

void GdxViewerState::setSymbolTableHeaderState(const QByteArray &symbolTableHeaderState)
{
    mSymbolTableHeaderState = symbolTableHeaderState;
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
