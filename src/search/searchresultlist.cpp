/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "searchresultlist.h"
#include <QtDebug>

namespace gams {
namespace studio {
namespace search {

SearchResultList::SearchResultList(QRegularExpression regex) : mSearchRegex(regex)
{
}

SearchResultList::~SearchResultList()
{
}

QList<Result> SearchResultList::resultsAsList() const
{
    QList<Result> result;
    for (QList<Result> rl : mResultHash.values())
        result << rl;

    return result;
}

void SearchResultList::addResult(int lineNr, int colNr, int length, QString fileLoc, QString context)
{
    Result r = Result(lineNr, colNr, length, fileLoc, context);
    mSize++;
    mResultHash[fileLoc].append(r);
}

QList<Result> SearchResultList::filteredResultList(QString fileLocation)
{
    return mResultHash.value(fileLocation);
}

void SearchResultList::setSearchRegex(QRegularExpression searchRegex)
{
    mSearchRegex = searchRegex;
}

QRegularExpression SearchResultList::searchRegex()
{
    return mSearchRegex;
}

int SearchResultList::size()
{
    return mSize;
}

int SearchResultList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mSize;
}

int SearchResultList::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 3;
}

QVariant SearchResultList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole) {
        int row = index.row();

        Result item = at(row);

        switch(index.column())
        {
        case 0: return item.filepath();
        case 1: return item.lineNr();
        case 2: return item.context();
        }
    }
    return QVariant();
}

QVariant SearchResultList::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Filename");
            case 1:
                return QString("LineNr");
            case 2:
                return QString("Context");
            }
        }
    }
    return QVariant();
}

Result SearchResultList::at(int index) const
{
    int start = 0;
    QList<QString> keys = mResultHash.keys();
    QString key;

    for (int i = 0; i < keys.size(); i++) {

        // is in this list
        if (index < (start + mResultHash.value(keys.at(i)).size()))
            return mResultHash.value(keys.at(i)).at(index - start); // calc index
        else
            start += mResultHash.value(keys.at(i)).size(); // go to next list
    }
    qDebug() << "ERROR: SearchResultList::at out of bounds" << index;
    return Result(0, 0, 0, "", ""); // this should never happen
}

QMultiHash<QString, QList<Result>> SearchResultList::resultHash() const
{
    return mResultHash;
}

}
}
}

