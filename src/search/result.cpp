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
#include "result.h"

namespace gams {
namespace studio {
namespace search {

Result::Result()
{

}

Result::Result(int lineNr, int colNr, int length, const QString &fileLoc,
               const NodeId& parent, const QString& context)
    : mLineNr(lineNr)
    , mColNr(colNr)
    , mLength(length)
    , mFilePath(fileLoc)
    , mParent(parent)
{
    int left = qMax(0, colNr - MAX_CONTEXT_LENGTH/2 + length/2);

    mContext = context.mid(left, MAX_CONTEXT_LENGTH);

    int startMatch = colNr - left;
    int endMatch = colNr - left + length;

    mContext = mContext.insert(endMatch, matchHighlightEnd);
    mContext = mContext.insert(startMatch, matchHighlightStart);

    int lengthBeforeTrim = context.length();
    mContext = mContext.trimmed();

    if (left + MAX_CONTEXT_LENGTH < lengthBeforeTrim) mContext.append("...");
    if (left > 0) mContext.prepend("...");
}

int Result::lineNr() const
{
    return mLineNr;
}

int Result::colNr() const
{
    return mColNr;
}

QString Result::filePath() const
{
    return mFilePath;
}

void Result::setFilePath(const QString &fp)
{
    mFilePath = fp;
}

QString Result::context() const
{
    return mContext;
}

int Result::length() const
{
    return mLength;
}

NodeId Result::parentGroup() const
{
    return mParent;
}

void Result::setParentGroup(const NodeId &parent)
{
    mParent = parent;
}

int Result::logicalIndex() const
{
    return mLogicalIndex;
}

void Result::setLogicalIndex(int index)
{
    mLogicalIndex = index;
}

}
}
}
