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

class GdxSymbolView : public QWidget
{
    Q_OBJECT

public:
    explicit GdxSymbolView(QWidget *parent = 0);
    ~GdxSymbolView();

    GdxSymbol *sym() const;
    void setSym(GdxSymbol *sym);

private:
    Ui::GdxSymbolView *ui;
    GdxSymbol *mSym = nullptr;
    QByteArray mInitialHeaderState;

public slots:
    void enableControls();
    void refreshView();
    void toggleSqueezeDefaults(bool checked);
    void resetSortFilter();
    void showColumnFilter(QPoint p);
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEW_H
