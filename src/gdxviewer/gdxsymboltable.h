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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H

#include <QAbstractItemModel>
#include <QMutex>
#include "gdxsymbol.h"
#include <memory>
#include "gdxcc.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbol;

class GdxSymbolTable : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GdxSymbolTable(gdxHandle_t gdx, QMutex* gdxMutex, QObject *parent = nullptr);
    ~GdxSymbolTable() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QList<GdxSymbol *> gdxSymbols() const;
    QString uel2Label(int uel);
    std::vector<int> labelCompIdx();
    int symbolCount() const;
    QString getElementText(int textNr);

private:
    QStringList mHeaderText;
    QString typeAsString(int type) const;
    void createSortIndex();
    gdxHandle_t mGdx = nullptr;
    int mUelCount;
    int mSymbolCount;
    void loadUel2Label();
    void loadStringPool();
    void loadGDXSymbols();
    void reportIoError(int errNr, QString message);

    QList<GdxSymbol*> mGdxSymbols;
    QStringList mUel2Label;
    QStringList mStrPool;

    std::vector<int> mLabelCompIdx;
    bool mIsSortIndexCreated = false;

    QMutex* mGdxMutex = nullptr;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
