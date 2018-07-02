/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_LXIVIEWER_LXITREEMODEL_H
#define GAMS_STUDIO_LXIVIEWER_LXITREEMODEL_H

#include <QAbstractItemModel>
#include <QVector>

namespace gams {
namespace studio {
namespace lxiviewer {

class LxiTreeItem;

class LxiTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit LxiTreeModel(LxiTreeItem *root, QVector<int> lineNrs, QVector<LxiTreeItem*> treeItems, QObject *parent = nullptr);
    ~LxiTreeModel() override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVector<int> lineNrs() const;

    QVector<LxiTreeItem *> treeItems() const;

private:
    LxiTreeItem* mRootItem;
    QVector<int> mLineNrs;
    QVector<LxiTreeItem*> mTreeItems;

};

} // namespace lxiviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_LXIVIEWER_LXITREEMODEL_H
