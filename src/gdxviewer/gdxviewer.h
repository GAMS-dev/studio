#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include "ui_gdxviewer.h"
#include "gdxcc.h"
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer : public QFrame
{
    Q_OBJECT

public:
    explicit GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent = 0);

private:
    Ui::GdxViewer ui;
    gdxHandle_t mGdx = nullptr;
    void reportIoError(int errNr, QString message);
    QList<GDXSymbol> mGdxSymbols;
    QList<GDXSymbol> loadGDXSymbol();
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
