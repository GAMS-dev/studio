/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "resultitem.h"

namespace gams {
namespace studio {
namespace search {

ResultItem::ResultItem(ResultItem *parent)
    : mParent(parent)
    , mType(Root)
{

}

ResultItem::ResultItem(int index, const QString &filePath, ResultItem *parent)
    : mParent(parent)
    , mType(File)
{
    mResult.setFilePath(filePath);
    mResult.setLogicalIndex(index);
}

ResultItem::ResultItem(const Result &result, ResultItem *parent)
    : mParent(parent)
    , mResult(result)
    , mType(Entry)
{

}

ResultItem::~ResultItem()
{
    qDeleteAll(mChilds);
}

void ResultItem::append(ResultItem *item)
{
    mChilds.append(item);
}

ResultItem *ResultItem::child(int index)
{
    if (index < 0 || index >= mChilds.size())
        return nullptr;
    return mChilds.at(index);
}

const QList<ResultItem *> &ResultItem::childs() const
{
    return mChilds;
}

int ResultItem::firstLogicalIndex() const
{
    return mResult.logicalIndex();
}

int ResultItem::lastLogicalIndex() const
{
    return mResult.logicalIndex() + mChilds.size() - 1;
}

int ResultItem::realIndex() const
{
    return mRealIndex;
}

void ResultItem::setRealIndex(int index)
{
    mRealIndex = index;
}

int ResultItem::columnCount() const
{
    return 2;
}

int ResultItem::rowCount() const
{
    return mChilds.size();
}

int ResultItem::row() const
{
    if (mParent)
        return mParent->mChilds.indexOf(const_cast<ResultItem*>(this));
    return 0;
}

const Result &ResultItem::data() const
{
    return mResult;
}

QString ResultItem::filePathCount() const
{
    return mResult.filePath() + " (" + QString::number(mChilds.size()) + ")";
}

ResultItem *ResultItem::parent() const
{
    return mParent;
}

void ResultItem::setParent(ResultItem *item)
{
    mParent = item;
}

ResultItem::Type ResultItem::type() const
{
    return mType;
}

bool gams::studio::search::ResultItem::hasChilds() const
{
    return mChilds.size();
}

}
}
}
