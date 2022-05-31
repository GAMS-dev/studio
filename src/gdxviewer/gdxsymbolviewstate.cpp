#include "gdxsymbolviewstate.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolViewState::GdxSymbolViewState()
{

}

bool GdxSymbolViewState::squeezeTrailingZeroes() const
{
    return mSqueezeTrailingZeroes;
}

void GdxSymbolViewState::setSqueezeTrailingZeroes(bool squeezeTrailingZeroes)
{
    mSqueezeTrailingZeroes = squeezeTrailingZeroes;
}

int GdxSymbolViewState::dim() const
{
    return mDim;
}

void GdxSymbolViewState::setDim(int dim)
{
    mDim = dim;
}

int GdxSymbolViewState::type() const
{
    return mType;
}

void GdxSymbolViewState::setType(int type)
{
    mType = type;
}

bool GdxSymbolViewState::tableView() const
{
    return mTableView;
}

void GdxSymbolViewState::setTableView(bool tableView)
{
    mTableView = tableView;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
