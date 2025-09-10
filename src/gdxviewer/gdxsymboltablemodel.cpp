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
#include "gdxsymboltablemodel.h"
#include "gdxsymbol.h"
#include "exception.h"
#include "encoding.h"

#include <QMutex>
#include <limits>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolTableModel::GdxSymbolTableModel(gdxHandle_t gdx, QMutex* gdxMutex, const QString &encoding, QObject *parent)
    : QAbstractTableModel(parent), mGdx(gdx), mGdxMutex(gdxMutex), mDecoder(Encoding::createDecoder(encoding))
{
    gdxSystemInfo(mGdx, &mSymbolCount, &mUelCount);
    loadUel2Label();
    loadStringPool();

    mHeaderText.append("Entry");
    mHeaderText.append("Name");
    mHeaderText.append("Type");
    mHeaderText.append("Dim");
    mHeaderText.append("Records");
    mHeaderText.append("Loaded");
    mHeaderText.append("Text");

    loadGDXSymbols();
}

GdxSymbolTableModel::~GdxSymbolTableModel()
{
    for(auto gdxSymbol : std::as_const(mGdxSymbols)) {
        mGdxSymbolByName.remove(gdxSymbol->name());
        delete gdxSymbol;
    }
}

QVariant GdxSymbolTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            if (section < mHeaderText.size())
                return mHeaderText.at(section);
    }
    else if (role == Qt::ToolTipRole) {
        QString description("<html><head/><body>");
        description += "<p><span style=\" font-weight:600;\">Sort: </span>Left click sorts the column in alphanumerical order using a stable sort mechanism. Sorting direction can be changed by clicking again.</p></p>";
        description += "</body></html>";
        return description;
    }
    return QVariant();
}

int GdxSymbolTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mGdxSymbols.size();
}

int GdxSymbolTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeaderText.size();
}

QVariant GdxSymbolTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::UserRole) {
        GdxSymbol* symbol = mGdxSymbols.at(index.row());
        switch(index.column()) {
            case 0:  {
                if (role == Qt::DisplayRole)
                    return QString("%L1").arg(symbol->nr());
                else  // Qt::UserRole used for sorting in the QFilterProxyModel
                    return symbol->nr();
            }
            case 1: return symbol->name();
            case 2: return typeAsString(symbol->type(), symbol->subType());
            case 3: return symbol->dim();
            case 4: {
                if (role == Qt::DisplayRole)
                    return QString("%L1").arg(symbol->recordCount());
                else  // Qt::UserRole used for sorting in the QFilterProxyModel
                    return symbol->recordCount();
            }
            case 5: return symbol->isLoaded();
            case 6: return symbol->explText();
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        Qt::AlignmentFlag aFlag;
        switch(index.column()) {
            case 0: aFlag = Qt::AlignRight; break;
            case 1: aFlag = Qt::AlignLeft; break;
            case 2: aFlag = Qt::AlignLeft; break;
            case 3: aFlag = Qt::AlignRight; break;
            case 4: aFlag = Qt::AlignRight; break;
            case 5: aFlag = Qt::AlignLeft; break;
            case 6: aFlag = Qt::AlignLeft; break;
            default: aFlag = Qt::AlignLeft; break;
        }
        return QVariant(aFlag | Qt::AlignVCenter);
    }
    return QVariant();
}

void GdxSymbolTableModel::loadGDXSymbols()
{
    QMutexLocker locker(mGdxMutex);
    for(int i=0; i<mSymbolCount+1; i++) {
        GdxSymbol* sym = new GdxSymbol(mGdx, mGdxMutex, i, this);
        mGdxSymbols.append(sym);
        mGdxSymbolByName[sym->name()] = sym;
    }
    locker.unlock();
}

void GdxSymbolTableModel::createSortIndex()
{
    QList<QPair<QString, int>> l;
    l.reserve(mUelCount);
    for (int uel = 0; uel <= mUelCount; ++uel)
        l.append(QPair<QString, int>(uel2Label(uel), uel));
    std::sort(l.begin(), l.end(), [](const QPair<QString, int> &a, const QPair<QString, int> &b) { return a.first.compare(b.first, Qt::CaseInsensitive)<0; });

    mLabelCompIdx.resize(mUelCount+1);
    int idx = 0;
    for (const QPair<QString, int> &p : l) {
        mLabelCompIdx[p.second] = idx;
        idx++;
    }
}

int GdxSymbolTableModel::getUelCount() const
{
    return mUelCount;
}

int GdxSymbolTableModel::label2Uel(const QString &label)
{
    int uelNr = -1;
    int uelMap = -1;
    if (gdxUMFindUEL(mGdx, label.toLocal8Bit(), &uelNr, &uelMap))
        return uelNr;
    else
        return -1;
}

int GdxSymbolTableModel::symbolCount() const
{
    return mSymbolCount;
}

QString GdxSymbolTableModel::getElementText(int textNr)
{
    if (textNr <= 0)
        return QString("Y");
    else {
        char text[GMS_SSSIZE];
        int node;

        gdxGetElemText(mGdx, textNr, text, &node);
        return mDecoder.decode(text);
    }
}

void GdxSymbolTableModel::loadUel2Label()
{
    char label[GMS_UEL_IDENT_SIZE];
    int map;
    for (int i=0; i<=mUelCount; i++) {
        gdxUMUelGet(mGdx, i, label, &map);
        QString l = mDecoder.decode(label);
        mUel2Label.append(l);
    }
}

void GdxSymbolTableModel::loadStringPool()
{
    mStrPool.append("Y");
    int strNr = 1;
    int node;
    char text[GMS_SSSIZE];

    while (gdxGetElemText(mGdx, strNr, text, &node)) {
        mStrPool.append(mDecoder.decode(text));
        strNr++;
    }
}

void GdxSymbolTableModel::reportIoError(int errNr, const QString &message)
{
    EXCEPT() << "Fatal I/O Error = " << errNr << " when calling " << message;
}

QStringDecoder &GdxSymbolTableModel::decoder()
{
    return mDecoder;
}

std::vector<int> GdxSymbolTableModel::labelCompIdx()
{
    if (!mIsSortIndexCreated) {
        this->createSortIndex();
        mIsSortIndexCreated = true;
    }
    return mLabelCompIdx;
}

QString GdxSymbolTableModel::uel2Label(int uel)
{
    if (uel < 0 || uel >= mUel2Label.size()) {
        char label[GMS_UEL_IDENT_SIZE];
        int map;
        gdxUMUelGet(mGdx, uel, label, &map);
        return mDecoder.decode(label);
    }
    return this->mUel2Label.at(uel);
}

QList<GdxSymbol *> GdxSymbolTableModel::gdxSymbols() const
{
    return mGdxSymbols;
}

GdxSymbol *GdxSymbolTableModel::getSymbolByName(const QString &name) const
{
    if (mGdxSymbolByName.contains(name))
        return mGdxSymbolByName[name];
    else
        return nullptr;
}

QString GdxSymbolTableModel::typeAsString(int type, int subtype) const
{
    QString s;
    switch(type) {
    case GMS_DT_SET:
        s = "Set";
        switch(subtype) {
        case GMS_SETTYPE_SINGLETON  : s += " (Singleton)"; break;
        default: break;
        }
        break;
    case GMS_DT_PAR:
        s = "Parameter";
        break;
    case GMS_DT_VAR:
        s = "Variable";
        switch(subtype) {
        case GMS_VARTYPE_UNKNOWN: s += " (Unknown)"; break;
        case GMS_VARTYPE_BINARY: s += " (Binary)"; break;
        case GMS_VARTYPE_INTEGER: s += " (Integer)"; break;
        case GMS_VARTYPE_POSITIVE: s += " (Positive)"; break;
        case GMS_VARTYPE_NEGATIVE: s += " (Negative)"; break;
        case GMS_VARTYPE_FREE: s += " (Free)"; break;
        case GMS_VARTYPE_SOS1: s += " (SOS1)"; break;
        case GMS_VARTYPE_SOS2: s += " (SOS2)"; break;
        case GMS_VARTYPE_SEMICONT: s += " (SemiCont)"; break;
        case GMS_VARTYPE_SEMIINT: s += " (SemiInt)"; break;
        default: break;
        }
        break;
    case GMS_DT_EQU:
        s = "Equation";
        switch(subtype) {
        case GMS_EQUTYPE_E: s += " (=e=)"; break;
        case GMS_EQUTYPE_G: s += " (=g=)"; break;
        case GMS_EQUTYPE_L: s += " (=l=)"; break;
        case GMS_EQUTYPE_N: s += " (=n=)"; break;
        case GMS_EQUTYPE_X: s += " (=x=)"; break;
        case GMS_EQUTYPE_C: s += " (=c=)"; break;
        case GMS_EQUTYPE_B: s += " (=b=)"; break;
        default: break;
        }
        break;
    case GMS_DT_ALIAS:
        s = "Alias";
        break;
    default:
        s = "Unknown";
    }
    return s;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
