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
#ifndef LXITREEITEM_H
#define LXITREEITEM_H

#include <QList>
#include <QModelIndex>


namespace gams {
namespace studio {
namespace lxiviewer {

class LxiTreeItem
{
public:
    explicit LxiTreeItem(const QString &index = QString(), int lineNr = -1, const QString &text = QString(), LxiTreeItem *parentItem = nullptr);
    ~LxiTreeItem();

    void appendChild(LxiTreeItem *child);

    LxiTreeItem* child(int row);
    int childCount() const;
    int row() const;
    LxiTreeItem* parentItem();

    QString index() const;

    QString text() const;

    int lineNr() const;

    QModelIndex modelIndex() const;
    void setModelIndex(QModelIndex value);

private:
    QList<LxiTreeItem*> mChildItems;
    QString mIndex;
    int mLineNr;
    QString mText;
    LxiTreeItem* mParentItem = nullptr;
    QModelIndex mModelIndex;
};

} // namespace lxiviewer
} // namespace studio
} // namespace gams

#endif // LXITREEITEM_H
