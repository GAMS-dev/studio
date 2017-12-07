#ifndef GDXSYMBOLHEADERVIEW_H
#define GDXSYMBOLHEADERVIEW_H

#include <QHeaderView>


namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    GdxSymbolHeaderView(Qt::Orientation orientation, QWidget *parent=nullptr);
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GDXSYMBOLHEADERVIEW_H
