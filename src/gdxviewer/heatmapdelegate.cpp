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

void HeatmapDelegate::debugSymbol()
{
    DEB() << "Type: " << mSymbol->type();
    if (!mSymbol->numBoundSize()) {
        DEB() << "  - not initialized";
        return;
    }
    if (mSymbol->type() == GMS_DT_PAR)
        DEB() << "Min/Max: " << mSymbol->minDouble() << " .. " << mSymbol->maxDouble();
    else if (mSymbol->type() == GMS_DT_EQU || mSymbol->type() == GMS_DT_VAR) {
        for (int i = 0; i < GMS_VAL_MAX; ++i)
            DEB() << "Min/Max: [" << i << "] " << mSymbol->minDouble(i) << " .. " << mSymbol->maxDouble(i);
    }
}

void HeatmapDelegate::setTableModel(TableViewModel *tvModel)
{
    mTvModel = tvModel;
}

bool HeatmapDelegate::active() const
{
    return mActive;
}

void HeatmapDelegate::setActive(bool newActive)
{
    mActive = newActive;
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
    double val = index.model()->data(index).toDouble(&ok);
    if (ok) {
        int mod = mSymbol->numBoundSize();
        int col = index.model() == mSymbol ? index.column() - mSymbol->dim() : index.column();
        int numBoundIndex = mod == 1 ? 0 : col % mod;
        double min = mSymbol->minDouble(numBoundIndex);
        double div = mSymbol->maxDouble(numBoundIndex) - min;
        painter->save();
        QRect smallerRect = opt.rect.adjusted(1, 1, -1, -1);
        QColor color = Theme::profileColor(Theme::Window_base, qBound(.0, (val - min) / div, 1.));
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
