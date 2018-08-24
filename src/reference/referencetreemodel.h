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
#ifndef REFERENCETREEMODEL_H
#define REFERENCETREEMODEL_H

#include <QAbstractItemModel>

#include "symbolreferenceitem.h"
#include "referenceitemmodel.h"

namespace gams {
namespace studio {
namespace reference {

class ReferenceTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ReferenceTreeModel(Reference* ref, QObject *parent = nullptr);
    ~ReferenceTreeModel();

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    bool removeRows(int row, int count, const QModelIndex &parent) override;

    void resetModel();

signals:
    void referenceSelectionChanged(ReferenceItem item);

public slots:
    void updateSelectedSymbol(SymbolId symbolid);

private:
    void insertSymbolReference(QList<ReferenceItemModel*>& parents, const QList<ReferenceItem*>& referenceItemList, const QString& referenceType);

    Reference* mReference;
    SymbolId mCurrentSymbolID;
    ReferenceItemModel* mRootItem;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // REFERENCETREEMODEL_H
