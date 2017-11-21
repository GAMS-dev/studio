#ifndef COLUMNFILTER_H
#define COLUMNFILTER_H

#include <QWidgetAction>
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class ColumnFilter : public QWidgetAction
{
public:
    ColumnFilter(GdxSymbol* symbol, int column, QWidget *parent = 0);
    QWidget* createWidget(QWidget * parent) override;

private:
    GdxSymbol* mSymbol = nullptr;
    int mColumn;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // COLUMNFILTER_H
