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
#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include <QAbstractItemModel>
#include <QRegularExpression>

#include "result.h"

namespace gams {
namespace studio {
namespace search {

class ResultItem;

class SearchResultModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    SearchResultModel(const QRegularExpression &regex, const QList<Result> &results, QObject *parent = nullptr);

    ~SearchResultModel();

    QRegularExpression searchRegex();

    int resultCount();

    QString resultCountString();

    ResultItem* item(int logicalIndex);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QRegularExpression mSearchRegex;
    QList<Result> mResults;
    QHash<int, ResultItem*> mItems;
    ResultItem* mRootItem;
    bool mMaxPlusResutls = false;
};

}
}
}
#endif // SEARCHRESULTLIST_H
