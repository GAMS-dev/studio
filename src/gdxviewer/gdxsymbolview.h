#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEW_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEW_H

#include <QFrame>
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

namespace Ui {
class GdxSymbolView;
}

class GdxSymbolView : public QFrame
{
    Q_OBJECT

public:
    explicit GdxSymbolView(QWidget *parent = 0);
    explicit GdxSymbolView(GdxSymbol *sym, QWidget *parent = 0);
    ~GdxSymbolView();

private:
    Ui::GdxSymbolView *ui;
    GdxSymbol *mSym = nullptr;
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEW_H
