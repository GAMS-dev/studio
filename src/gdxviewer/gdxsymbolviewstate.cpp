/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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

QVector<QStringList> GdxSymbolViewState::uncheckedLabels() const
{
    return mUncheckedLabels;
}

void GdxSymbolViewState::setUncheckedLabels(const QVector<QStringList> &uncheckedLabels)
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

QVector<int> GdxSymbolViewState::getTableViewColumnWidths() const
{
    return mTableViewColumnWidths;
}

void GdxSymbolViewState::setTableViewColumnWidths(const QVector<int> &tableViewColumnWidths)
{
    mTableViewColumnWidths = tableViewColumnWidths;
}

bool GdxSymbolViewState::autoResizeLV() const
{
    return mAutoResizeLV;
}

void GdxSymbolViewState::setAutoResizeLV(bool newAutoResizeLV)
{
    mAutoResizeLV = newAutoResizeLV;
}

bool GdxSymbolViewState::autoResizeTV() const
{
    return mAutoResizeTV;
}

void GdxSymbolViewState::setAutoResizeTV(bool newAutoResizeTV)
{
    mAutoResizeTV = newAutoResizeTV;
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
