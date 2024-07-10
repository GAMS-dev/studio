/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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

#include "valuefilter.h"
#include "valuefilterwidget.h"

#include <QWidgetAction>
#include <QMenu>

namespace gams {
namespace studio {
namespace gdxviewer {

ValueFilter::ValueFilter(GdxSymbol *symbol, int valueColumn, QWidget *parent)
    :QWidgetAction(parent), mSymbol(symbol), mValueColumn(valueColumn)
{
    mMin = mSymbol->minDouble(valueColumn);
    mMax = mSymbol->maxDouble(valueColumn);
    mCurrentMin = mMin;
    mCurrentMax = mMax;
}

QWidget *ValueFilter::createWidget(QWidget *parent)
{
    mWidget = new ValueFilterWidget(this, parent);
    return mWidget;
}

void ValueFilter::updateFilter()
{
    if (mMin==mCurrentMin && mMax==mCurrentMax && !mExclude && mShowUndef && mShowNA && mShowPInf && mShowMInf && mShowEps && mShowAcronym)
        mSymbol->unregisterValueFilter(mValueColumn);
    else
        mSymbol->registerValueFilter(mValueColumn, this);
    mSymbol->filterRows();
}

void ValueFilter::reset()
{
    mSymbol->unregisterValueFilter(mValueColumn);
    mSymbol->filterRows();
}

double ValueFilter::min() const
{
    return mMin;
}

double ValueFilter::max() const
{
    return mMax;
}

double ValueFilter::currentMin() const
{
    return mCurrentMin;
}

double ValueFilter::currentMax() const
{
    return mCurrentMax;
}

bool ValueFilter::showUndef() const
{
    return mShowUndef;
}

void ValueFilter::setShowUndef(bool showUndef)
{
    mShowUndef = showUndef;
}

bool ValueFilter::showNA() const
{
    return mShowNA;
}

void ValueFilter::setShowNA(bool showNA)
{
    mShowNA = showNA;
}

bool ValueFilter::showPInf() const
{
    return mShowPInf;
}

void ValueFilter::setShowPInf(bool showPInf)
{
    mShowPInf = showPInf;
}

bool ValueFilter::showMInf() const
{
    return mShowMInf;
}

void ValueFilter::setShowMInf(bool showNInf)
{
    mShowMInf = showNInf;
}

bool ValueFilter::showEps() const
{
    return mShowEps;
}

void ValueFilter::setShowEps(bool showEps)
{
    mShowEps = showEps;
}

bool ValueFilter::showAcronym() const
{
    return mShowAcronym;
}

void ValueFilter::setShowAcronym(bool showAcronym)
{
    mShowAcronym = showAcronym;
}

bool ValueFilter::exclude() const
{
    return mExclude;
}

void ValueFilter::setExclude(bool invert)
{
    mExclude = invert;
}

void ValueFilter::setCurrentMin(double currentMin)
{
    mCurrentMin = currentMin;
}

void ValueFilter::setCurrentMax(double currentMax)
{
    mCurrentMax = currentMax;
}

void ValueFilter::setFocus()
{
    static_cast<ValueFilterWidget *>(mWidget)->setFocusOnOpen();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
