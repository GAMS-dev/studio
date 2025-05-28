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
#ifndef FILEREFERENCEITEM_H
#define FILEREFERENCEITEM_H

#include <QVariant>

namespace gams {
namespace studio {
namespace reference {

typedef int FileReferenceId;

enum class FileReferenceItemColumn {
    Location         = 0,
    Type             = 1,
    GlobalLineNumber = 2,
    LocalLineNumber  = 3,
    Id               = 4,
    FirstEntry       = 5
};

class FileReferenceItem
{
public:
    FileReferenceItem(const QList<QVariant>& data, FileReferenceItem* parentItem = nullptr);
    ~FileReferenceItem();

    void appendChild(FileReferenceItem *child);

    FileReferenceItem* child(int row);
    int childNumber() const;

    FileReferenceItem* parent();
    int childCount() const;
    int columnCount() const;

    QVariant data(int column) const;
    int row() const;

    bool setData(int column, const QVariant &value);

    void setParent(FileReferenceItem* parent);
    void insertChild(int row, FileReferenceItem* item);

private:
    QList<QVariant>           mItemData;
    QList<FileReferenceItem*> mChildItems;
    FileReferenceItem*        mParentItem;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // FILEREFERENCEITEM_H
