#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include "ui_gdxviewer.h"
#include "gdxcc.h"
#include "gdxsymbol.h"
#include "gdxsymboltable.h"
#include <memory>
#include <QMutex>
#include <QVector>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer : public QWidget
{
    Q_OBJECT

public:
    GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent = 0);
    ~GdxViewer();
    void updateSelectedSymbol(QItemSelection selected, QItemSelection deselected);
    GdxSymbol* selectedSymbol();

private:
    Ui::GdxViewer ui;
    void reportIoError(int errNr, QString message);

    GdxSymbolTable* mGdxSymbolTable;

    gdxHandle_t mGdx;
    QMutex* mGdxMutex;

    void loadSymbol(GdxSymbol* selectedSymbol);

    QVector<GdxSymbolView*> mSymbolViews;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
