#include "gdxsymbolheaderview.h"
#include <QDebug>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolHeaderView::GdxSymbolHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    qDebug() << "constructor GdxSymbolHeaderView";
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
