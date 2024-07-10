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

#include "tableviewdomainmodel.h"

namespace gams {
namespace studio {
namespace gdxviewer {

TableViewDomainModel::TableViewDomainModel(TableViewModel* tvModel, QObject *parent)
    : QAbstractTableModel(parent), mTvModel(tvModel)
{
    connect(mTvModel, &TableViewModel::modelReset, this, [this]() { beginResetModel(); endResetModel();});
}

QVariant TableViewDomainModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    if (role == Qt::DisplayRole) {
        if (section < mTvModel->dim())
            return mTvModel->domains().at(section) + " " + GdxSymbol::superScript[section+1];
        else {
            if (mTvModel->type() == GMS_DT_PAR)
                return QVariant("Value");
            else if (mTvModel->type() == GMS_DT_VAR || mTvModel->type() == GMS_DT_EQU) {
                switch(section-mTvModel->dim()) {
                case GMS_VAL_LEVEL: return "Level";
                case GMS_VAL_MARGINAL: return "Marginal";
                case GMS_VAL_LOWER: return "Lower";
                case GMS_VAL_UPPER: return "Upper";
                case GMS_VAL_SCALE: return "Scale";
                }

            }
        }
    } else if (role == Qt::ToolTipRole) {
        QString description("<html><head/><body>");
        description += "<p><span style=\" font-weight:600;\">Filter:</span> The filter menu can be opened via right click or by clicking on the filter icon.</p>";
        description += "</body></html>";
        return description;
    }
    return QVariant();
}

int TableViewDomainModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 0;
}

int TableViewDomainModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (mTvModel->type() == GMS_DT_PAR)
        return mTvModel->dim() + 1;
    if (mTvModel->type() == GMS_DT_EQU || mTvModel->type() == GMS_DT_VAR)
        return mTvModel->dim() + GMS_VAL_MAX;
    return mTvModel->dim();
}

QVariant TableViewDomainModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)
    if (!index.isValid())
        return QVariant();
    return QVariant();
}

TableViewModel *TableViewDomainModel::tvModel() const
{
    return mTvModel;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
