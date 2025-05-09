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
#include "searchresultmodel.h"
#include "resultitem.h"
#include "common.h"

#include <QtDebug>
#include <QTime>

namespace gams {
namespace studio {
namespace search {

SearchResultModel::SearchResultModel(const QRegularExpression &regex,
                                     const QList<Result> &results,
                                     QObject *parent)
    : QAbstractItemModel(parent)
    , mSearchRegex(regex)
    , mResults(results)
    , mRootItem(new ResultItem)
    , mMaxPlusResutls(results.size() > MAX_SEARCH_RESULTS-1)
{
    if (mResults.size() > MAX_SEARCH_RESULTS) {
        mResults.erase(mResults.begin()+MAX_SEARCH_RESULTS, mResults.end());
        mResults.squeeze();
    }
    int fileIndex = 0;
    int itemIndex = 0;
    ResultItem* file = nullptr;
    for (auto iter = mResults.constBegin(); iter!=mResults.constEnd(); ++iter) {
        if (!file || file->data().filePath() != iter->filePath()) {
            file = new ResultItem(itemIndex, iter->filePath(), mRootItem);
            file->setRealIndex(fileIndex++);
            mRootItem->append(file);
        }
        auto r = *iter;
        r.setLogicalIndex(itemIndex++);
        auto item = new ResultItem(r, file);
        item->setRealIndex(file->childs().size());
        file->append(item);
        mItems[item->firstLogicalIndex()] = item;
    }
}

SearchResultModel::~SearchResultModel()
{
    delete mRootItem;
}

QRegularExpression SearchResultModel::searchRegex()
{
    return mSearchRegex;
}

int SearchResultModel::resultCount()
{
    return mResults.size();
}

QString SearchResultModel::resultCountString()
{
    return mMaxPlusResutls ? QString::number(MAX_SEARCH_RESULTS) + "+" : QString::number(resultCount());
}

ResultItem *SearchResultModel::item(int logicalIndex)
{
    return mItems.contains(logicalIndex) ? mItems[logicalIndex] : nullptr;
}

QModelIndex SearchResultModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    ResultItem *item;
    if (!parent.isValid())
        item = mRootItem;
    else
        item = static_cast<ResultItem*>(parent.internalPointer());
    auto child = item->child(row);
    if (child)
        return createIndex(row, column, child);
    return QModelIndex();
}

int SearchResultModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    ResultItem *item;
    if (!parent.isValid())
        item = mRootItem;
    else
        item = static_cast<ResultItem*>(parent.internalPointer());
    return item->rowCount();
}

int SearchResultModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ResultItem*>(parent.internalPointer())->columnCount();
    return mRootItem->columnCount();
}

QModelIndex SearchResultModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    auto child = static_cast<ResultItem*>(index.internalPointer());
    auto item = child->parent();
    if (item == mRootItem)
        return QModelIndex();
    return createIndex(item->row(), 0, item);
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    auto item = static_cast<ResultItem*>(index.internalPointer());
    if (role != Qt::DisplayRole)
        return QVariant();
    if (item->type() == ResultItem::Entry) {
        switch(index.column())
        {
        case 0: return item->data().context();
        case 1: return item->data().lineNr();
        }
    }
    return index.column() == 0 ? item->filePathCount() : QVariant();
}

QVariant SearchResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Results");
            case 1:
                return QString("Line");
            }
        }
    }
    return QVariant();
}

}
}
}
