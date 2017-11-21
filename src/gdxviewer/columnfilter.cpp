#include "columnfilter.h"
#include "columnfilterframe.h"


namespace gams {
namespace studio {
namespace gdxviewer {

ColumnFilter::ColumnFilter(GdxSymbol *symbol, int column, QWidget *parent)
    :QWidgetAction(parent), mSymbol(symbol), mColumn(column)
{

}

QWidget *ColumnFilter::createWidget(QWidget *parent)
{
    return new ColumnFilterFrame(mSymbol, mColumn, parent);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
