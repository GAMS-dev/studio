/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "result.h"

namespace gams {
namespace studio {
namespace search {

Result::Result(int lineNr, int colNr, int length, QString fileLoc, QString context) :
    mLineNr(lineNr), mColNr(colNr), mLength(length), mFilepath(fileLoc), mContext(context)
{ }

int Result::lineNr() const
{
    return mLineNr;
}

int Result::colNr() const
{
    return mColNr;
}

QString Result::filepath() const
{
    return mFilepath;
}

QString Result::context() const
{
    return mContext;
}

int Result::length() const
{
    return mLength;
}

}
}
}
