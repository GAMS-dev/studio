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
#ifndef CLEANUPFILTERMODEL_H
#define CLEANUPFILTERMODEL_H

#include <QAbstractItemModel>
#include <QAbstractTableModel>

namespace gams {
namespace studio {

class CleanupFilterItem
{
public:
    explicit CleanupFilterItem(Qt::CheckState checked = Qt::Unchecked,
                               const QString &filter = QString(),
                               const QString &description = QString(),
                               CleanupFilterItem *parent = nullptr);

    ~CleanupFilterItem();

    QString filter() const;

    void setFilter(const QString &filter);

    QString description() const;

    void setDescription(const QString &description);

    Qt::CheckState checked();

    void setChecked(Qt::CheckState state);

    void append(CleanupFilterItem *child);

    void remove(int index);

    void removeAllItems();

    CleanupFilterItem* item(int index);

    QList<CleanupFilterItem*> items() const;

    void setItems(const QList<CleanupFilterItem*>& items);

    bool hasItems() const;

    int entries() const;

    int row() const;

    CleanupFilterItem* parent() const;

    void setParent(CleanupFilterItem *parent);

private:
    CleanupFilterItem *mParent = nullptr;

    QList<CleanupFilterItem*> mItems;

    QString mFilter;
    QString mDescription;
    Qt::CheckState mChecked;
};

struct CleanupWorkspaceItem
{
    QString Workspace;
    Qt::CheckState CheckState = Qt::Unchecked;
};

class CleanupFilterModel
    : public QAbstractItemModel
{
    Q_OBJECT

public:
    CleanupFilterModel(QObject *parent = nullptr);

    ~CleanupFilterModel();

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    CleanupFilterItem* data() const;

    void setData(const QList<CleanupFilterItem*> &entries);

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    void setSelection(Qt::CheckState checkState);

    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    QStringList activeFilters() const;

private:
    CleanupFilterItem* mRootItem;
};

class CleanupWorkspaceModel
    : public QAbstractTableModel
{
    Q_OBJECT

public:
    CleanupWorkspaceModel(QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    const QList<CleanupWorkspaceItem>& workspaces() const;

    void setWorkspaces(const QList<CleanupWorkspaceItem> &workspaces);

    void setSelection(Qt::CheckState checkState);

    QStringList activeWorkspaces() const;

private:
    QList<CleanupWorkspaceItem> mWorkspaces;
};

}
}

#endif // CLEANUPFILTERMODEL_H
