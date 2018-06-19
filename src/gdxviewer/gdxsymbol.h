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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H

#include <QAbstractTableModel>
#include <QMutex>
#include "gdxsymboltable.h"
#include <memory>
#include "gdxcc.h"
#include <QString>
#include <QSet>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolTable;


class GdxSymbol : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, int nr,
                       GdxSymbolTable* gdxSymbolTable, QObject *parent = nullptr);
    ~GdxSymbol() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int nr() const;
    QString name() const;
    int dim() const;
    int type() const;
    int recordCount() const;
    QString explText() const;
    bool isLoaded() const;
    void loadData();
    void stopLoadingData();
    bool isAllDefault(int valColIdx);
    int subType() const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void filterRows();
    int sortColumn() const;
    Qt::SortOrder sortOrder() const;
    void resetSortFilter();
    GdxSymbolTable *gdxSymbolTable() const;
    std::vector<std::vector<int> *> uelsInColumn() const;
    std::vector<bool *> showUelInColumn() const;
    void setShowUelInColumn(const std::vector<bool *> &showUelInColumn);
    std::vector<bool> filterActive() const;
    void setFilterActive(const std::vector<bool> &filterActive);

signals:
    void loadFinished();

private:
    gdxHandle_t mGdx = nullptr;
    int mNr;
    QMutex* mGdxMutex = nullptr;
    int mDim;
    int mType;
    int mSubType;
    int mRecordCount;
    QString mExplText;
    QString mName;

    std::vector<int> mMinUel;
    std::vector<int> mMaxUel;

    GdxSymbolTable* mGdxSymbolTable = nullptr;

    bool mIsLoaded = false;
    int mLoadedRecCount = 0;
    int mFilterRecCount = 0;

    bool stopLoading = false;

    std::vector<int> mKeys;
    std::vector<double> mValues;

    QStringList mDomains;

    bool mDefaultColumn[GMS_VAL_MAX] {false};

    void calcDefaultColumns();
    void calcUelsInColumn();
    void loadMetaData();
    void loadDomains();


    double specVal2SortVal(double val);
    std::vector<double> mSpecValSortVal;

    std::vector<std::vector<int>*> mUelsInColumn;
    std::vector<bool*> mShowUelInColumn;
    std::vector<bool> mFilterActive;

    std::vector<int> mRecSortIdx;
    std::vector<int> mRecFilterIdx;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
