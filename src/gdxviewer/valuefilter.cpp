/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
    return new ValueFilterWidget(this, parent);
}

void ValueFilter::setFilter(double min, double max)
{
    if (min == mMin && max == mMax) {
        std::vector<bool> filterActive = mSymbol->filterActive();
        filterActive[mValueColumn+mSymbol->dim()] = false;
        mSymbol->setFilterActive(filterActive);
        return;
    }

    mCurrentMin = min;
    mCurrentMax = max;
    std::vector<bool> filterActive = mSymbol->filterActive();
    filterActive[mValueColumn+mSymbol->dim()] = true;
    mSymbol->registerValueFilter(mValueColumn, this);
    mSymbol->setFilterActive(filterActive);
    mSymbol->filterRows();
    mSymbol->setFilterHasChanged(true);
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

} // namespace gdxviewer
} // namespace studio
} // namespace gams
