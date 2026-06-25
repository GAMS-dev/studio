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
#include "heatmapdelegate.h"
#include "theme.h"
#include <QPainter>

namespace gams {
namespace studio {
namespace gdxviewer {

HeatmapDelegate::HeatmapDelegate(QWidget *parent, QAbstractItemModel *model)
    : QStyledItemDelegate{parent}, mParent(parent), mModel(model)
{}

void HeatmapDelegate::setBounds(double min, double max)
{
    mMin = min;
    mDiv = max - min;
}

void HeatmapDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QString text = opt.text;
    opt.text = "";
    QStyledItemDelegate::paint(painter, opt, index);

    bool ok;
    double val = mModel->data(index).toDouble(&ok);
    if (ok) {
        painter->save();
        QRect smallerRect = opt.rect.adjusted(1, 1, -1, -1);
        QColor color = Theme::profileColor(Theme::Window_base, qBound(.0, (val - mMin) / mDiv, 1.));
        painter->fillRect(smallerRect, color);
        painter->restore();
    }

    opt.text = text;
    opt.backgroundBrush = Qt::NoBrush;
    QStyledItemDelegate::paint(painter, opt, index);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
