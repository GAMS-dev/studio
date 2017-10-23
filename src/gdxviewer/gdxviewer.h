#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include "ui_gdxviewer.h"
#include "gdxcc.h"
#include "gdxsymbol.h"
#include "gdxsymboltable.h"
#include <memory>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer : public QFrame
{
    Q_OBJECT

public:
    explicit GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent = 0);
    ~GdxViewer();
    void updateSelectedSymbol();

private:
    Ui::GdxViewer ui;
    void reportIoError(int errNr, QString message);

    GdxSymbolTable* mGdxSymbolTable;

    gdxHandle_t mGdx;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
