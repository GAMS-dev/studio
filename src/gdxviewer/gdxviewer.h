#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include "ui_gdxviewer.h"
#include "gdxcc.h"
#include "gdxsymbol.h"
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
    QStringList mUel2Label;

    void updateSelectedSymbol();

private:
    Ui::GdxViewer ui;
    gdxHandle_t mGdx = nullptr;
    void reportIoError(int errNr, QString message);
    QList<std::shared_ptr<GDXSymbol>> mGdxSymbols;
    QList<std::shared_ptr<GDXSymbol>> loadGDXSymbol();
    int mUelCount;
    QStringList loadUel2Label();
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
