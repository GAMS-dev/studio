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
#ifndef GAMS_STUDIO_GDXVIEWER_VALUEFILTER_H
#define GAMS_STUDIO_GDXVIEWER_VALUEFILTER_H

#include <QWidgetAction>
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class ValueFilter : public QWidgetAction
{
public:
    ValueFilter(GdxSymbol* symbol, int valueColumn, QWidget *parent = nullptr);
    QWidget* createWidget(QWidget * parent) override;
    void updateFilter();
    void reset();

    double min() const;
    double max() const;
    double currentMin() const;
    double currentMax() const;

    bool showUndef() const;
    void setShowUndef(bool showUndef);

    bool showNA() const;
    void setShowNA(bool showNA);

    bool showPInf() const;
    void setShowPInf(bool showPInf);

    bool showMInf() const;
    void setShowMInf(bool showMInf);

    bool showEps() const;
    void setShowEps(bool showEps);

    bool showAcronym() const;
    void setShowAcronym(bool showAcronym);

    bool exclude() const;
    void setExclude(bool exclude);

    void setCurrentMin(double currentMin);
    void setCurrentMax(double currentMax);

    void setFocus();

private:
    GdxSymbol* mSymbol = nullptr;
    int mValueColumn;

    double mMin;
    double mMax;

    double mCurrentMin;
    double mCurrentMax;

    bool mExclude = false;

    bool mShowUndef = true;
    bool mShowNA = true;
    bool mShowPInf = true;
    bool mShowMInf = true;
    bool mShowEps = true;
    bool mShowAcronym = true;

    QWidget *mWidget = nullptr;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_VALUEFILTER_H
