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
#include "common.h"
#include <QtDebug>
#include <QTime>

namespace gams {
namespace studio {
namespace search {

SearchResultModel::SearchResultModel(const QRegularExpression &regex, const QList<Result> &results)
    : mSearchRegex(regex), mResults(results)
{
    while (mResults.size() > MAX_SEARCH_RESULTS) {
        mResults.removeLast();
    }
}

QList<Result> SearchResultModel::results() const
{
    return mResults;
}

QRegularExpression SearchResultModel::searchRegex()
{
    return mSearchRegex;
}

int SearchResultModel::size()
{
    return mResults.size();
}

int SearchResultModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mResults.size();
}

int SearchResultModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 3;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole) {
        Result item = at(index.row());

        switch(index.column())
        {
        case 0: return item.filepath();
        case 1: return item.lineNr();
        case 2: return item.context();
        }
    }
    return QVariant();
}

QVariant SearchResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Filename");
            case 1:
                return QString("Line");
            case 2:
                return QString("Context");
            }
        }
    }
    return QVariant();
}

Result SearchResultModel::at(int index) const
{
    return mResults.at(index);
}

}
}
}

