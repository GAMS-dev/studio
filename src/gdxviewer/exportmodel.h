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
#ifndef GAMS_STUDIO_GDXVIEWER_EXPORTMODEL_H
#define GAMS_STUDIO_GDXVIEWER_EXPORTMODEL_H

#include <QAbstractTableModel>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer;
class GdxSymbolTableModel;
class GdxSymbol;

class ExportModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ExportModel(GdxSymbolTableModel* mGdxSymbolTableModel, QObject *parent = nullptr);

    ~ExportModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QList<GdxSymbol*> selectedSymbols();

    void selectAll();
    void deselectAll();

private:
    GdxSymbolTableModel *mGdxSymbolTableModel = nullptr;
    QVector<bool> mChecked;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_EXPORTMODEL_H
