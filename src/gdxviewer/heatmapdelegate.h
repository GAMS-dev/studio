/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#ifndef HEATMAPDELEGATE_H
#define HEATMAPDELEGATE_H

#include <QStyledItemDelegate>

namespace gams {
namespace studio {
namespace gdxviewer {

class HeatmapDelegate : public QStyledItemDelegate
{
    QWidget *mParent = nullptr;
    QAbstractItemModel *mModel = nullptr;
    double mMin = .0;
    double mDiv = .0;

    Q_OBJECT
public:
    explicit HeatmapDelegate(QWidget *parent, QAbstractItemModel *model);
    void setBounds(double min, double max);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // HEATMAPDELEGATE_H
