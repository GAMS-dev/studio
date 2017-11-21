#ifndef GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
#define GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H

#include "ui_columnfilterframe.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class ColumnFilterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ColumnFilterFrame(QWidget *parent = 0);

private:
    Ui::ColumnFilterFrame ui;
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
