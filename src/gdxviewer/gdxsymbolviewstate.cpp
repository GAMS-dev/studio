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

QVector<QStringList> GdxSymbolViewState::uncheckedLabels() const
{
    return mUncheckedLabels;
}

void GdxSymbolViewState::setUncheckedLabels(const QVector<QStringList> &uncheckedLabels)
{
    mUncheckedLabels = uncheckedLabels;
}

QVector<double> GdxSymbolViewState::currentMin() const
{
    return mCurrentMin;
}

void GdxSymbolViewState::setCurrentMin(const QVector<double> &currentMin)
{
    mCurrentMin = currentMin;
}

QVector<double> GdxSymbolViewState::currentMax() const
{
    return mCurrentMax;
}

void GdxSymbolViewState::setCurrentMax(const QVector<double> &currentMax)
{
    mCurrentMax = currentMax;
}

QVector<bool> GdxSymbolViewState::valFilterActive() const
{
    return mValFilterActive;
}

void GdxSymbolViewState::setValFilterActive(const QVector<bool> &valFilterActive)
{
    mValFilterActive = valFilterActive;
}

QVector<bool> GdxSymbolViewState::exclude() const
{
    return mExclude;
}

void GdxSymbolViewState::setExclude(const QVector<bool> &exclude)
{
    mExclude = exclude;
}

QVector<bool> GdxSymbolViewState::showUndef() const
{
    return mShowUndef;
}

void GdxSymbolViewState::setShowUndef(const QVector<bool> &showUndef)
{
    mShowUndef = showUndef;
}

QVector<bool> GdxSymbolViewState::showNA() const
{
    return mShowNA;
}

void GdxSymbolViewState::setShowNA(const QVector<bool> &showNA)
{
    mShowNA = showNA;
}

QVector<bool> GdxSymbolViewState::showPInf() const
{
    return mShowPInf;
}

void GdxSymbolViewState::setShowPInf(const QVector<bool> &showPInf)
{
    mShowPInf = showPInf;
}

QVector<bool> GdxSymbolViewState::showMInf() const
{
    return mShowMInf;
}

void GdxSymbolViewState::setShowMInf(const QVector<bool> &showMInf)
{
    mShowMInf = showMInf;
}

QVector<bool> GdxSymbolViewState::showEps() const
{
    return mShowEps;
}

void GdxSymbolViewState::setShowEps(const QVector<bool> &showEps)
{
    mShowEps = showEps;
}

QVector<bool> GdxSymbolViewState::showAcronym() const
{
    return mShowAcronym;
}

void GdxSymbolViewState::setShowAcronym(const QVector<bool> &showAcronym)
{
    mShowAcronym = showAcronym;
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
