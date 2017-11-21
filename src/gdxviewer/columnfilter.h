#ifndef COLUMNFILTER_H
#define COLUMNFILTER_H

#include <QWidgetAction>

namespace gams {
namespace studio {
namespace gdxviewer {

class ColumnFilter : public QWidgetAction
{
public:
    ColumnFilter(QWidget *parent = 0);
    QWidget* createWidget(QWidget * parent) override;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // COLUMNFILTER_H
