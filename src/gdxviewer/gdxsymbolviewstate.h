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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H

#include "valuefilter.h"

#include <QStringList>
#include <QMap>
#include <QObject>

namespace gams {
namespace studio {
namespace gdxviewer {



class GdxSymbolViewState
{
public:
    GdxSymbolViewState();

    bool sqTrailingZeroes() const;
    void setSqTrailingZeroes(bool squeezeTrailingZeroes);

    int dim() const;
    void setDim(int dim);

    int type() const;
    void setType(int type);

    bool tableViewActive() const;
    void setTableViewActive(bool tableViewActive);

    QList<QStringList> uncheckedLabels() const;

    void setUncheckedLabels(const QList<QStringList> &uncheckedLabels);

    int numericalPrecision() const;
    void setNumericalPrecision(int numericalPrecision);

    bool restoreSqZeroes() const;
    void setRestoreSqZeroes(bool restoreSqZeroes);

    int valFormatIndex() const;
    void setValFormatIndex(int valFormatIndex);

    bool sqDefaults() const;
    void setSqDefaults(bool sqDefaults);

    QByteArray listViewHeaderState() const;
    void setListViewHeaderState(const QByteArray &listViewHeaderState);

    QByteArray tableViewFilterHeaderState() const;
    void setTableViewFilterHeaderState(const QByteArray &tableViewFilterHeaderState);

    bool tableViewLoaded() const;
    void setTableViewLoaded(bool tableViewLoaded);

    int tvColDim() const;
    void setTvColDim(int tvColDim);

    QList<int> tvDimOrder() const;
    void setTvDimOrder(const QList<int> &tvDimOrder);

    ValueFilter &getValueFilter(int valueColumn);
    void setValueFilter(const QList<ValueFilter> &valueFilter);

    QList<bool> getShowAttributes() const;
    void setShowAttributes(const QList<bool> &value);

    QList<int> getTableViewColumnWidths() const;
    void setTableViewColumnWidths(const QList<int> &tableViewColumnWidths);

    bool autoResizeLV() const;
    void setAutoResizeLV(bool newAutoResizeLV);

    bool autoResizeTV() const;
    void setAutoResizeTV(bool newAutoResizeTV);

    void read(const QVariantMap &map);
    void write(QVariantMap &map) const;

    bool showHeatmap() const;
    void setShowHeatmap(bool showHeatmap);

    bool heatmapUseFilter() const;
    void setHeatmapUseFilter(bool heatmapUseFilter);

    int heatedAttributes() const;
    void setHeatedAttributes(int newHeatedAttrib);

private:
    enum StateFlag {
        sfSqDefaults        = 0x001,
        sfSqTrailingZeroes  = 0x002,
        sfRestoreSqZeroes   = 0x004,
        sfTableViewActive   = 0x008,
        sfTableViewLoaded   = 0x010,
        sfAutoResizeLV      = 0x020,     // true
        sfAutoResizeTV      = 0x040,     // true
        sfShowHeatmap       = 0x080,
        sfHeatmapUseFilter  = 0x100,
    };
    Q_DECLARE_FLAGS(StateFlags, StateFlag);

    StateFlags mStateFlags;
    // bool mSqDefaults;
    // bool mSqTrailingZeroes;
    // bool mRestoreSqZeroes;
    // bool mTableViewActive;
    // bool mTableViewLoaded = false;

    // bool mAutoResizeLV = true;
    // bool mAutoResizeTV = true;
    // bool mShowHeatmap = false;
    // bool mHeatmapUseFilter = false;

    int mNumericalPrecision;
    int mValFormatIndex;
    int mDim;
    int mType;
    int mHeatedAttributes = 1;

    QList<bool> mShowAttributes;

    // table view state
    int mTvColDim;
    QList<int> mTvDimOrder;

    // column filters
    QList<QStringList> mUncheckedLabels;

    // value filters
    QList<ValueFilter> mValueFilter;

    QByteArray mListViewHeaderState;
    QByteArray mTableViewFilterHeaderState;
    QList<int> mTableViewColumnWidths;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H
