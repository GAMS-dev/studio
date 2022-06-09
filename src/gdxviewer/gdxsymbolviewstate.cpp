#include "gdxsymbolviewstate.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolViewState::GdxSymbolViewState()
{

}

bool GdxSymbolViewState::sqTrailingZeroes() const
{
    return mSqTrailingZeroes;
}

void GdxSymbolViewState::setSqTrailingZeroes(bool sqTrailingZeroes)
{
    mSqTrailingZeroes = sqTrailingZeroes;
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

bool GdxSymbolViewState::tableViewActive() const
{
    return mTableViewActive;
}

void GdxSymbolViewState::setTableViewActive(bool tableViewActive)
{
    mTableViewActive = tableViewActive;
}

QVector<QSet<QString>> GdxSymbolViewState::uncheckedLabels() const
{
    return mUncheckedLabels;
}

void GdxSymbolViewState::setUncheckedLabels(const QVector<QSet<QString>> &uncheckedLabels)
{
    mUncheckedLabels = uncheckedLabels;
}

int GdxSymbolViewState::numericalPrecision() const
{
    return mNumericalPrecision;
}

void GdxSymbolViewState::setNumericalPrecision(int numericalPrecision)
{
    mNumericalPrecision = numericalPrecision;
}

bool GdxSymbolViewState::restoreSqZeroes() const
{
    return mRestoreSqZeroes;
}

void GdxSymbolViewState::setRestoreSqZeroes(bool restoreSqZeroes)
{
    mRestoreSqZeroes = restoreSqZeroes;
}

int GdxSymbolViewState::valFormatIndex() const
{
    return mValFormatIndex;
}

void GdxSymbolViewState::setValFormatIndex(int valFormatIndex)
{
    mValFormatIndex = valFormatIndex;
}

bool GdxSymbolViewState::sqDefaults() const
{
    return mSqDefaults;
}

void GdxSymbolViewState::setSqDefaults(bool sqDefaults)
{
    mSqDefaults = sqDefaults;
}

QByteArray GdxSymbolViewState::listViewHeaderState() const
{
    return mListViewHeaderState;
}

void GdxSymbolViewState::setListViewHeaderState(const QByteArray &listViewHeaderState)
{
    mListViewHeaderState = listViewHeaderState;
}

QByteArray GdxSymbolViewState::tableViewFilterHeaderState() const
{
    return mTableViewFilterHeaderState;
}

void GdxSymbolViewState::setTableViewFilterHeaderState(const QByteArray &tableViewFilterHeaderState)
{
    mTableViewFilterHeaderState = tableViewFilterHeaderState;
}

bool GdxSymbolViewState::tableViewLoaded() const
{
    return mTableViewLoaded;
}

void GdxSymbolViewState::setTableViewLoaded(bool tableViewLoaded)
{
    mTableViewLoaded = tableViewLoaded;
}

int GdxSymbolViewState::tvColDim() const
{
    return mTvColDim;
}

void GdxSymbolViewState::setTvColDim(int tvColDim)
{
    mTvColDim = tvColDim;
}

QVector<int> GdxSymbolViewState::tvDimOrder() const
{
    return mTvDimOrder;
}

void GdxSymbolViewState::setTvDimOrder(const QVector<int> &tvDimOrder)
{
    mTvDimOrder = tvDimOrder;
}

QVector<ValueFilterState> GdxSymbolViewState::valueFilterState() const
{
    return mValueFilterState;
}

void GdxSymbolViewState::setValueFilterState(const QVector<ValueFilterState> &valueFilterState)
{
    mValueFilterState = valueFilterState;
}

QVector<bool> GdxSymbolViewState::getShowAttributes() const
{
    return showAttributes;
}

void GdxSymbolViewState::setShowAttributes(const QVector<bool> &value)
{
    showAttributes = value;
}

QByteArray GdxSymbolViewState::getTableViewHeaderState() const
{
    return mTableViewHeaderState;
}

void GdxSymbolViewState::setTableViewHeaderState(const QByteArray &tableViewHeaderState)
{
    mTableViewHeaderState = tableViewHeaderState;
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
