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
#include "gdxsymbol.h"
#include "tableviewmodel.h"
#include "gclgms.h"
#include "logger.h"
#include <QTableView>
#include <QPainter>

namespace gams {
namespace studio {
namespace gdxviewer {

HeatmapDelegate::HeatmapDelegate(QTableView *tableView)
    : QStyledItemDelegate{tableView}, mTableView(tableView)
{}

void HeatmapDelegate::setTableView(QTableView *tableView)
{
    mTableView = tableView;
}

void HeatmapDelegate::setSymbolModel(GdxSymbol *sym)
{
    mSymbol = sym;
}

void HeatmapDelegate::setTableModel(TableViewModel *tvModel)
{
    mTvModel = tvModel;
}

bool HeatmapDelegate::active() const
{
    return mActive;
}

void HeatmapDelegate::setActive(bool active)
{
    mActive = active;
}

bool HeatmapDelegate::useFilterBounds() const
{
    return mUseFilterBounds;
}

void HeatmapDelegate::setUseFilterBounds(bool useFilter)
{
    mUseFilterBounds = useFilter;
}

bool equalDouble(double x, double y)
{
    static double epsilon = std::numeric_limits<double>::epsilon();
    if (qAbs(x - y) <= epsilon * 10)
        return true;
    return qAbs(x - y) <= epsilon * qMax(qAbs(x), qAbs(y));
}

bool HeatmapDelegate::symbolCanShowHeatmap()
{
    if (!mSymbol) return false;
    for (int i = 0; i < mSymbol->numBoundSize(); ++i) {
        if (!equalDouble(mSymbol->minDouble(i), mSymbol->maxDouble(i)))
            return true;
    }
    return false;
}

void HeatmapDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!mActive) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QString text = opt.text;
    opt.text = "";
    QStyledItemDelegate::paint(painter, opt, index);

    bool ok;
    double val = index.model()->data(index, Qt::UserRole).toDouble(&ok);
    if (val == GMS_SV_UNDEF || val == GMS_SV_NA) ok = false;
    if (val == GMS_SV_EPS) val = 0;
    int mod = mSymbol->numBoundSize();
    if (ok && mod) {
        int col = index.model() == mSymbol ? index.column() - mSymbol->dim() : index.column();
        int numBoundIndex = mod == 1 ? 0 : col % mod;
        if (!mSymbol->allEqual(numBoundIndex)) {
            auto valFilter = mSymbol->valueFilter(numBoundIndex);
            double min = mUseFilterBounds && valFilter.active ? valFilter.min : mSymbol->minDouble(numBoundIndex);
            double max = mUseFilterBounds && valFilter.active ? valFilter.max : mSymbol->maxDouble(numBoundIndex);
            if (val == GMS_SV_PINF) val = max;
            if (val == GMS_SV_MINF) val = min;
            qreal alpha = equalDouble(min, max) ? .5 : qBound(.0, (val - min) / (max - min), 1.);
            QColor color = Theme::heatmapColor(Theme::Window_base, alpha);

            painter->save();
            QRect smallerRect = opt.rect.adjusted(1, 1, -1, -1);
            painter->fillRect(smallerRect, color);
            painter->restore();
        }
    }

    opt.text = text;
    opt.backgroundBrush = Qt::NoBrush;
    QStyledItemDelegate::paint(painter, opt, index);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
