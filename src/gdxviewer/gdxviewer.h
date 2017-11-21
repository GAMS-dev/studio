#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include "ui_gdxviewer.h"
#include "gdxcc.h"
#include "gdxsymbol.h"
#include "gdxsymboltable.h"
#include <memory>
#include <QMutex>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer : public QFrame
{
    Q_OBJECT

public:
    explicit GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent = 0);
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

signals:
    void loadFinished();


public slots:
    void toggleSqueezeDefaults(bool checked);
    void refreshView();
    void resetSorting();
    void showColumnFilter(QPoint p);
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
