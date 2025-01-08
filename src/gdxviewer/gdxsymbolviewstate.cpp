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
#include "gdxsymbolviewstate.h"
#include "logger.h"
#include <QVariant>
#include <QLocale>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolViewState::GdxSymbolViewState()
{

}

bool GdxSymbolViewState::sqTrailingZeroes() const
{
    return mSqTrailingZeroes;
}

void GdxSymbolViewState::setSqTrailingZeroes(bool sqTrailingZeroes)
{
    mSqTrailingZeroes = sqTrailingZeroes;
}

int GdxSymbolViewState::dim() const
{
    return mDim;
}

void GdxSymbolViewState::setDim(int dim)
{
    mDim = dim;
}

int GdxSymbolViewState::type() const
{
    return mType;
}

void GdxSymbolViewState::setType(int type)
{
    mType = type;
}

bool GdxSymbolViewState::tableViewActive() const
{
    return mTableViewActive;
}

void GdxSymbolViewState::setTableViewActive(bool tableViewActive)
{
    mTableViewActive = tableViewActive;
}

QVector<QStringList> GdxSymbolViewState::uncheckedLabels() const
{
    return mUncheckedLabels;
}

void GdxSymbolViewState::setUncheckedLabels(const QVector<QStringList> &uncheckedLabels)
{
    mUncheckedLabels = uncheckedLabels;
}

int GdxSymbolViewState::numericalPrecision() const
{
    return mNumericalPrecision;
}

void GdxSymbolViewState::setNumericalPrecision(int numericalPrecision)
{
    mNumericalPrecision = numericalPrecision;
}

bool GdxSymbolViewState::restoreSqZeroes() const
{
    return mRestoreSqZeroes;
}

void GdxSymbolViewState::setRestoreSqZeroes(bool restoreSqZeroes)
{
    mRestoreSqZeroes = restoreSqZeroes;
}

int GdxSymbolViewState::valFormatIndex() const
{
    return mValFormatIndex;
}

void GdxSymbolViewState::setValFormatIndex(int valFormatIndex)
{
    mValFormatIndex = valFormatIndex;
}

bool GdxSymbolViewState::sqDefaults() const
{
    return mSqDefaults;
}

void GdxSymbolViewState::setSqDefaults(bool sqDefaults)
{
    mSqDefaults = sqDefaults;
}

QByteArray GdxSymbolViewState::listViewHeaderState() const
{
    return mListViewHeaderState;
}

void GdxSymbolViewState::setListViewHeaderState(const QByteArray &listViewHeaderState)
{
    mListViewHeaderState = listViewHeaderState;
}

QByteArray GdxSymbolViewState::tableViewFilterHeaderState() const
{
    return mTableViewFilterHeaderState;
}

void GdxSymbolViewState::setTableViewFilterHeaderState(const QByteArray &tableViewFilterHeaderState)
{
    mTableViewFilterHeaderState = tableViewFilterHeaderState;
}

bool GdxSymbolViewState::tableViewLoaded() const
{
    return mTableViewLoaded;
}

void GdxSymbolViewState::setTableViewLoaded(bool tableViewLoaded)
{
    mTableViewLoaded = tableViewLoaded;
}

int GdxSymbolViewState::tvColDim() const
{
    return mTvColDim;
}

void GdxSymbolViewState::setTvColDim(int tvColDim)
{
    mTvColDim = tvColDim;
}

QVector<int> GdxSymbolViewState::tvDimOrder() const
{
    return mTvDimOrder;
}

void GdxSymbolViewState::setTvDimOrder(const QVector<int> &tvDimOrder)
{
    mTvDimOrder = tvDimOrder;
}

QVector<ValueFilterState> GdxSymbolViewState::valueFilterState() const
{
    return mValueFilterState;
}

void GdxSymbolViewState::setValueFilterState(const QVector<ValueFilterState> &valueFilterState)
{
    mValueFilterState = valueFilterState;
}

QVector<bool> GdxSymbolViewState::getShowAttributes() const
{
    return mShowAttributes;
}

void GdxSymbolViewState::setShowAttributes(const QVector<bool> &value)
{
    mShowAttributes = value;
}

QVector<int> GdxSymbolViewState::getTableViewColumnWidths() const
{
    return mTableViewColumnWidths;
}

void GdxSymbolViewState::setTableViewColumnWidths(const QVector<int> &tableViewColumnWidths)
{
    mTableViewColumnWidths = tableViewColumnWidths;
}

bool GdxSymbolViewState::autoResizeLV() const
{
    return mAutoResizeLV;
}

void GdxSymbolViewState::setAutoResizeLV(bool newAutoResizeLV)
{
    mAutoResizeLV = newAutoResizeLV;
}

bool GdxSymbolViewState::autoResizeTV() const
{
    return mAutoResizeTV;
}

void GdxSymbolViewState::setAutoResizeTV(bool newAutoResizeTV)
{
    mAutoResizeTV = newAutoResizeTV;
}

void GdxSymbolViewState::write(QVariantMap &map) const
{
    int bools = mSqDefaults ? 1 : 0;
    bools += mSqTrailingZeroes ? 2 : 0;
    bools += mRestoreSqZeroes ? 4 : 0;
    bools += mTableViewActive ? 8 : 0;
    bools += mTableViewLoaded ? 16 : 0;
    bools += mAutoResizeLV ? 32 : 0;
    bools += mAutoResizeTV ? 64 : 0;
    map.insert("boolValues", bools);

    QString ints = QString::number(mNumericalPrecision);
    ints += ',' + QString::number(mValFormatIndex);
    ints += ',' + QString::number(mDim);
    ints += ',' + QString::number(mType);
    ints += ',' + QString::number(mTvColDim);
    map.insert("intValues", ints);

    QString showAttr;
    for (bool a : std::as_const(mShowAttributes))
        showAttr += (a ? '1' : '0');
    map.insert("showAttributes", showAttr);

    QString tvDimOrder;
    for (int order : std::as_const(mTvDimOrder))
        tvDimOrder += (tvDimOrder.isEmpty() ? "" : ",") + QString::number(order);
    map.insert("tvDimOrder", tvDimOrder);

    bool semicolon = false;
    QString labelLists;
    for (const QStringList &list : std::as_const(mUncheckedLabels)) {
        QString labels;
        bool comma = false;
        for (const QString &label : list) {
            if (comma) labels += ',';
            labels += label.toUtf8().toBase64();
            comma = true;
        }
        if (semicolon) labelLists += ';';
        labelLists += labels;
        semicolon = true;
    }
    map.insert("uncheckedLabels", labelLists);

    semicolon = false;
    QString valFilterStates;
    for (const ValueFilterState &state : std::as_const(mValueFilterState)) {
        QString valFilterState = QString::number(state.min, 'g', QLocale::FloatingPointShortest);
        valFilterState += ',' + QString::number(state.max, 'g', QLocale::FloatingPointShortest) + ',';
        valFilterState += QString(state.active ? "1" : "0") + (state.exclude ? "1" : "0") +
                (state.showUndef ? "1" : "0") + (state.showNA ? "1" : "0") + (state.showPInf ? "1" : "0") +
                (state.showMInf ? "1" : "0") + (state.showEps ? "1" : "0") + (state.showAcronym ? "1" : "0");
        if (semicolon) valFilterStates += ';';
        valFilterStates += valFilterState;
        semicolon = true;
    }
    map.insert("valueFilterStates", valFilterStates);

    map.insert("listViewHeader", mListViewHeaderState.toBase64());
    map.insert("tableViewFilterHeader", mTableViewFilterHeaderState.toBase64());

    QString tableViewColumns;
    for (int columnWidth : std::as_const(mTableViewColumnWidths)) {
        if (!tableViewColumns.isEmpty()) tableViewColumns += ',';
        tableViewColumns += QString::number(columnWidth);
    }
    map.insert("tableViewColumnWidths", tableViewColumns);
}

bool assignIfValidInt(int &var, const QString &intVal)
{
    bool ok = !intVal.isEmpty();
    int i = (ok ? intVal.toInt(&ok) : 0);
    if (ok) var = i;
    return ok;
}

bool assignIfValidDouble(double &var, const QString &doubleVal)
{
    bool ok = !doubleVal.isEmpty();
    double i = (ok ? doubleVal.toDouble(&ok) : 0.);
    if (ok) var = i;
    return ok;
}

void GdxSymbolViewState::read(const QVariantMap &map)
{
    if (map.contains("boolValues")) {
        int bools = map.value("boolValues").toInt();
        mSqDefaults = bools & 1;
        mSqTrailingZeroes = bools & 2;
        mRestoreSqZeroes = bools & 4;
        mTableViewActive = bools & 8;
        mTableViewLoaded = bools & 16;
        mAutoResizeLV = bools & 32;
        mAutoResizeTV = bools & 64;
    }

    if (map.contains("intValues")) {
        QStringList ints = map.value("intValues").toString().split(',');
        bool ok = ints.size() == 5;
        ok = ok && assignIfValidInt(mNumericalPrecision, ints.at(0));
        ok = ok && assignIfValidInt(mValFormatIndex, ints.at(1));
        ok = ok && assignIfValidInt(mDim, ints.at(2));
        ok = ok && assignIfValidInt(mType, ints.at(3));
        ok = ok && assignIfValidInt(mTvColDim, ints.at(4));
        if (!ok)
            DEB() << "Error restoring GDX symbol view: invalid value";
    }

    if (map.contains("showAttributes")) {
        mShowAttributes.clear();
        const QString attribs = map.value("showAttributes").toString();
        for (const QChar &atr : attribs)
            mShowAttributes << (atr != '0');
    }

    if (map.contains("tvDimOrder")) {
        mTvDimOrder.clear();
        const QStringList dims = map.value("tvDimOrder").toString().split(',');
        bool ok;
        for (const QString &dim : dims) {
            int val = dim.toInt(&ok);
            if (ok)
                mTvDimOrder << val;
        }
    }

    if (map.contains("uncheckedLabels")) {
        mUncheckedLabels.clear();
        const QStringList labelLists = map.value("uncheckedLabels").toString().split(';');
        for (const QString &labels: labelLists) {
            QStringList uncheckedLabels;
            const QStringList splitLabels = labels.split(',', Qt::SkipEmptyParts);
            for (const QString &coded : splitLabels) {
                uncheckedLabels << QByteArray::fromBase64(coded.toUtf8());
            }
            mUncheckedLabels << uncheckedLabels;
        }
    }

    if (map.contains("valueFilterStates")) {
        mValueFilterState.clear();
        const QStringList allFilterStates = map.value("valueFilterStates").toString().split(';');
        for (const QString &state : allFilterStates) {
            QStringList stateStrings = state.split(',');
            ValueFilterState valFilterState;
            bool ok = stateStrings.size() == 3;
            ok = ok && assignIfValidDouble(valFilterState.min, stateStrings.at(0));
            ok = ok && assignIfValidDouble(valFilterState.max, stateStrings.at(1));
            ok = ok && stateStrings.at(2).length() == 8;
            if (ok) {
                valFilterState.active = stateStrings.at(2).at(0) != '0';
                valFilterState.exclude = stateStrings.at(2).at(1) != '0';
                valFilterState.showUndef = stateStrings.at(2).at(2) != '0';
                valFilterState.showNA = stateStrings.at(2).at(3) != '0';
                valFilterState.showPInf = stateStrings.at(2).at(4) != '0';
                valFilterState.showMInf = stateStrings.at(2).at(5) != '0';
                valFilterState.showEps = stateStrings.at(2).at(6) != '0';
                valFilterState.showAcronym = stateStrings.at(2).at(7) != '0';
                mValueFilterState << valFilterState;
            } else {
                DEB() << "Error restoring GDX symbol view: invalid value filter state";
            }
        }
    }

    mListViewHeaderState = QByteArray::fromBase64(map.value("listViewHeader").toByteArray());
    mTableViewFilterHeaderState = QByteArray::fromBase64(map.value("tableViewFilterHeader").toByteArray());

    if (map.contains("tableViewColumnWidths")) {
        const QStringList tableViewColumns = map.value("tableViewColumnWidths").toString().split(',');
        for (const QString &columnWidth : tableViewColumns) {
            int value;
            if (!columnWidth.isEmpty()) {
                bool ok = assignIfValidInt(value, columnWidth);
                if (!ok) {
                    DEB() << "Error restoring GDX symbol view: invalid table column width";
                    break;
                }
                mTableViewColumnWidths << value;
            }
        }
    }
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
