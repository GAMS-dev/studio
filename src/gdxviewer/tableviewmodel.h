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
#ifndef GAMS_STUDIO_GDXVIEWER_TABLEVIEWMODEL_H
#define GAMS_STUDIO_GDXVIEWER_TABLEVIEWMODEL_H

#include <QAbstractTableModel>
#include "gdxsymbol.h"
#include "gdxsymboltablemodel.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class TableViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableViewModel(GdxSymbol* sym, GdxSymbolTableModel* gdxSymbolTable, QObject *parent = nullptr);
    ~TableViewModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVector<bool> defaultColumnTableView() const;

    QVector<int> tvDimOrder() const;

    int dim();

    int tvColDim() const;

    int type();

    void setTableView(int colDim = -1, const QVector<int> &tvDims = QVector<int>());

    void setTableViewNoArgs();

    bool isAllDefault(int valColIdx);

    bool needDummyRow() const;

    bool needDummyColumn() const;

    QVector<QList<QString> > labelsInRows() const;

    QStringList domains() const;

    GdxSymbol *sym() const;

public slots:
    void scrollHTriggered();
    void scrollVTriggered();
signals:
    void initFinished();

private:
    void calcDefaultColumnsTableView();

    void calcLabelsInRows();
    QVector<QList<QString>> mlabelsInRows;

    void initTableView(int nrColDim, QVector<int> dimOrder);

    GdxSymbol* mSym;
    GdxSymbolTableModel* mGdxSymbolTable;

    int mTvColDim;
    QVector<int> mTvDimOrder;
    QVector<QVector<uint>> mTvRowHeaders;
    QVector<QVector<uint>> mTvColHeaders;
    QHash<QVector<uint>, int> mTvKeysToValIdx;

    QVector<bool> mDefaultColumnTableView;

    bool mNeedDummyRow = false;
    bool mNeedDummyColumn = false;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_TABLEVIEWMODEL_H
