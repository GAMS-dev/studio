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
#ifndef RESULTITEM_H
#define RESULTITEM_H

#include "result.h"

#include <QString>
#include <QList>

namespace gams {
namespace studio {
namespace search {

class ResultItem
{
public:
    enum Type {
        Entry,
        File,
        Root
    };

    ResultItem(ResultItem *parent = nullptr);

    ResultItem(int index, const QString &filePath, ResultItem *parent = nullptr);

    ResultItem(const Result &result, ResultItem *parent = nullptr);

    ~ResultItem();

    void append(ResultItem *item);

    ResultItem* child(int index);

    const QList<ResultItem*>& childs() const;

    bool hasChilds() const;

    int firstLogicalIndex() const;

    int lastLogicalIndex() const;

    int realIndex() const;

    void setRealIndex(int index);

    int columnCount() const;

    int rowCount() const;

    int row() const;

    const Result& data() const;

    QString filePathCount() const;

    ResultItem* parent() const;

    void setParent(ResultItem* item);

    Type type() const;

private:
    ResultItem* mParent = nullptr;
    QList<ResultItem*> mChilds;
    Result mResult;
    int mRealIndex = 0;
    Type mType;
};

}
}
}

#endif // RESULTITEM_H
