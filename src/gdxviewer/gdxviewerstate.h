#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWERSTATE_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWERSTATE_H

#include "gdxsymbolviewstate.h"

#include <QMap>
#include <QObject>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewerState
{
public:
    GdxViewerState();
    ~GdxViewerState();

    QString symbolTableFilter() const;
    void setSymbolTableFilter(const QString &symbolTableFilter);

    bool allColumnsChecked() const;
    void setAllColumnsChecked(bool allColumnsChecked);

    GdxSymbolViewState* symbolViewState(QString name) const;
    GdxSymbolViewState* addSymbolViewState(QString name);
    void deleteSymbolViewState(QString name);

private:
    QString mSymbolTableFilter;
    bool mAllColumnsChecked;
    QMap<QString, GdxSymbolViewState*> mSymbolViewState;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWERSTATE_H
