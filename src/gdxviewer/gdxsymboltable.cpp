/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "gdxsymboltable.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolTable::GdxSymbolTable(gdxHandle_t gdx, QMutex* gdxMutex, QObject *parent)
    : QAbstractTableModel(parent), mGdx(gdx), mGdxMutex(gdxMutex)
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

GdxSymbolTable::~GdxSymbolTable()
{
    for(auto gdxSymbol : mGdxSymbols)
        delete gdxSymbol;
    if(mLabelCompIdx)
        delete[] mLabelCompIdx;
}

QVariant GdxSymbolTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
            if (section < mHeaderText.size())
                return mHeaderText.at(section);
    }
    else if (role == Qt::ToolTipRole)
    {
        QString description("<html><head/><body>");
        description += "<p><span style=\" font-weight:600;\">Sort: </span>Left click sorts the column in alphanumerical order using a stable sort mechanism. Sorting direction can be changed by clicking again.</p></p>";
        description += "</body></html>";
        return description;
    }
    return QVariant();
}

int GdxSymbolTable::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mGdxSymbols.size();
}

int GdxSymbolTable::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeaderText.size();
}

QVariant GdxSymbolTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
    {
        GdxSymbol* symbol = mGdxSymbols.at(index.row());
        switch(index.column())
        {
        case 0: return symbol->nr(); break;
        case 1: return symbol->name(); break;
        case 2: return typeAsString(symbol->type()); break;
        case 3: return symbol->dim(); break;
        case 4: return symbol->recordCount(); break;
        case 5: return symbol->isLoaded(); break;
        case 6: return symbol->explText(); break;
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        Qt::AlignmentFlag aFlag;
        switch(index.column())
        {
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

void GdxSymbolTable::loadGDXSymbols()
{
    QMutexLocker locker(mGdxMutex);
    for(int i=0; i<mSymbolCount+1; i++)
        mGdxSymbols.append(new GdxSymbol(mGdx, mGdxMutex, i, this));
    locker.unlock();
}

//TODO (CW): refactor
void GdxSymbolTable::createSortIndex()
{
    QList<QPair<QString, int>> l;
    for(int uel=0; uel<=mUelCount; uel++)
        l.append(QPair<QString, int>(mUel2Label.at(uel), uel));
    std::sort(l.begin(), l.end(), [](QPair<QString, int> a, QPair<QString, int> b) { return a.first < b.first; });

    mLabelCompIdx = new int[mUelCount+1];
    int idx = 0;
    for(QPair<QString, int> p : l)
    {
        mLabelCompIdx[p.second] = idx;
        idx++;
    }
}

int GdxSymbolTable::symbolCount() const
{
    return mSymbolCount;
}

void GdxSymbolTable::loadUel2Label()
{
    char label[GMS_UEL_IDENT_SIZE];
    int map;
    for(int i=0; i<=mUelCount; i++)
    {
        gdxUMUelGet(mGdx, i, label, &map);
        mUel2Label.append(QString(label));
    }
}

void GdxSymbolTable::loadStringPool()
{
    int strNr = 1;
    int node;
    char text[GMS_SSSIZE];
    mStrPool.append("Y");
    while(gdxGetElemText(mGdx, strNr, text, &node))
    {
        mStrPool.append(QString(text));
        strNr++;
    }
}

void GdxSymbolTable::reportIoError(int errNr, QString message)
{
    EXCEPT() << "Fatal I/O Error = " << errNr << " when calling " << message;
}

int *GdxSymbolTable::labelCompIdx()
{
    if(!mLabelCompIdx)
        this->createSortIndex();
    return mLabelCompIdx;
}

QStringList GdxSymbolTable::strPool() const
{
    return mStrPool;
}

QStringList GdxSymbolTable::uel2Label() const
{
    return mUel2Label;
}

QList<GdxSymbol *> GdxSymbolTable::gdxSymbols() const
{
    return mGdxSymbols;
}

QString GdxSymbolTable::typeAsString(int type) const
{
    switch(type)
    {
        case GMS_DT_SET: return "Set"; break;
        case GMS_DT_PAR: return "Parameter"; break;
        case GMS_DT_VAR: return "Variable"; break;
        case GMS_DT_EQU: return "Equation"; break;
        case GMS_DT_ALIAS: return "Alias"; break;
        default: return ""; break;
    }
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
