#ifndef GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
#define GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H

#include "ui_columnfilterframe.h"
#include "gdxsymbol.h"
#include "filteruelmodel.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class ColumnFilterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ColumnFilterFrame(GdxSymbol* symbol, int column, QWidget *parent = 0);
    ~ColumnFilterFrame();

private:
    Ui::ColumnFilterFrame ui;
    GdxSymbol* mSymbol;
    int mColumn;
    FilterUelModel* mModel;

private slots:
    void apply();
    void selectAll();
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
