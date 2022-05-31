#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H

#include <QStringList>


namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolViewState
{
public:
    GdxSymbolViewState();

    bool squeezeTrailingZeroes() const;
    void setSqueezeTrailingZeroes(bool squeezeTrailingZeroes);

    int dim() const;
    void setDim(int dim);

    int type() const;
    void setType(int type);

    bool tableView() const;
    void setTableView(bool tableView);

private:
    bool mSqueezeTrailingZeroes;
    int mDim;
    int mType;
    bool mTableView;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H
