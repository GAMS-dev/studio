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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H

#include <QAbstractItemModel>
#include "gdxcc.h"

class QMutex;

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbol;

class GdxSymbolTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GdxSymbolTableModel(gdxHandle_t gdx, QMutex* gdxMutex, const QString &encoding, QObject *parent = nullptr);
    ~GdxSymbolTableModel() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QList<GdxSymbol *> gdxSymbols() const;
    GdxSymbol* getSymbolByName(const QString &name) const;
    QString uel2Label(int uel);
    std::vector<int> labelCompIdx();
    int symbolCount() const;
    QString getElementText(int textNr);
    int getUelCount() const;
    int label2Uel(const QString &label);

    QStringDecoder &decoder();

private:
    QStringList mHeaderText;
    QString typeAsString(int type, int subType) const;
    void createSortIndex();
    gdxHandle_t mGdx = nullptr;
    int mUelCount;
    int mSymbolCount;
    void loadUel2Label();
    void loadStringPool();
    void loadGDXSymbols();
    void reportIoError(int errNr, const QString &message);

    QList<GdxSymbol*> mGdxSymbols;
    QMap<QString, GdxSymbol*> mGdxSymbolByName;
    QStringList mUel2Label;
    QStringList mStrPool;

    std::vector<int> mLabelCompIdx;
    bool mIsSortIndexCreated = false;

    QMutex* mGdxMutex = nullptr;
    QStringDecoder mDecoder;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
