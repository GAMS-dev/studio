#include "columnfilter.h"
#include "columnfilterframe.h"


namespace gams {
namespace studio {
namespace gdxviewer {

ColumnFilter::ColumnFilter(QWidget *parent)
    :QWidgetAction(parent)
{

}

QWidget *ColumnFilter::createWidget(QWidget *parent)
{
    return new ColumnFilterFrame(parent);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
