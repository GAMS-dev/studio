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
#ifndef FILEUSEDTREEMODEL_H
#define FILEUSEDTREEMODEL_H

#include <QAbstractItemModel>

#include "filereferenceitem.h"
#include "reference.h"

namespace gams {
namespace studio {
namespace reference {

class FileUsedTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit FileUsedTreeModel(QObject *parent = nullptr);
    ~FileUsedTreeModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex indexForTreeItem(FileReferenceItem* item);

    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    FileReferenceItem *getItem(const QModelIndex &index) const;

    void resetModel();
    void initModel(Reference* ref);
    bool isModelLoaded();

protected:
    void setupTreeItemModelData();

    static QList<QVariant> header() {
        QList<QVariant> header;
        header << QVariant("Location")    << QVariant("Type")
               << QVariant("Global Line") << QVariant("Reference Line")
               << QVariant("ID")          << QVariant("First");
        return header;
    }
    Reference*         mReference;
    FileReferenceItem* mRootItem;
};
} // namespace reference
} // namespace studio
} // namespace gams

#endif // FILEUSEDTREEMODEL_H
