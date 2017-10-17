#include "gdxviewer.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewer::GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent) :
    QFrame(parent)
{
    ui.setupUi(this);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
