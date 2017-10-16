#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include "ui_gdxviewer.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer : public QFrame
{
    Q_OBJECT

public:
    explicit GdxViewer(QWidget *parent = 0);

private:
    Ui::GdxViewer ui;
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
