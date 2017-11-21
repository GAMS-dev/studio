#ifndef GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
#define GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H

#include "ui_columnfilterframe.h"
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class ColumnFilterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ColumnFilterFrame(GdxSymbol* symbol, int column, QWidget *parent = 0);

private:
    Ui::ColumnFilterFrame ui;
    GdxSymbol* mSymbol;
    int mColumn;

public slots:
    void apply();
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
