/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "searchlocator.h"
#include "search/searchdialog.h"
#include "search/result.h"
#include <QList>

namespace gams {
namespace studio {

SearchDialog* SearchLocator::mSd = nullptr;

void SearchLocator::provide(SearchDialog *sd)
{
    mSd = sd;
}
SearchResultList* SearchLocator::searchResults()
{
    if (mSd)
        return mSd->getCachedResults();
    else
        return nullptr;
}

SearchDialog* SearchLocator::searchDialog()
{
    return mSd;
}

}
}
