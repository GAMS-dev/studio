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
#include "symboldatatype.h"

namespace gams {
namespace studio {

QList<SymbolDataType*> SymbolDataType::mList;
SymbolDataType* SymbolDataType::mUndefined = nullptr;

SymbolDataType::SymbolType SymbolDataType::type() const
{
    return mType;
}

QStringList SymbolDataType::description() const
{
    return mDescription;
}

bool SymbolDataType::operator ==(const SymbolDataType &symbolDataType) const
{
    return (this == &symbolDataType);
}

bool SymbolDataType::operator !=(const SymbolDataType &symbolDataType) const
{
    return (this != &symbolDataType);
}

bool SymbolDataType::operator ==(const SymbolDataType::SymbolType &type) const
{
    return (mType == type);
}

bool SymbolDataType::operator !=(const SymbolDataType::SymbolType &type) const
{
    return (mType != type);
}

const QList<SymbolDataType*> SymbolDataType::list()
{
    if (mList.isEmpty()) {
        mUndefined = new SymbolDataType(Undefined, "UNDEFINED");
        mList << new SymbolDataType(Funct, "FUNCT,FUNCTIONS");
        mList << new SymbolDataType(Set, "SET,SETS");
        mList << new SymbolDataType(Acronym, "ACRNM,ACRONYMS");
        mList << new SymbolDataType(Parameter, "PARAM,PARAMETERS");
        mList << new SymbolDataType(Variable, "VAR,VARIABLES");
        mList << new SymbolDataType(Equation, "EQU,EQUATIONS");
        mList << new SymbolDataType(Model, "MODEL,MODELS");
        mList << new SymbolDataType(File, "FILE,FILES");
        mList << new SymbolDataType(Pred, "PRED");
    }
    return mList;
}

SymbolDataType &SymbolDataType::from(QString typeDescription)
{
    for (SymbolDataType* t : list()) {
        if (t->mDescription.contains(typeDescription, Qt::CaseInsensitive))
            return *t;
    }
    return *mUndefined;
}

SymbolDataType &SymbolDataType::from(SymbolDataType::SymbolType type)
{
    for (SymbolDataType* t : list()) {
        if (t->mType == type)
            return *t;
    }
    return *mUndefined;
}

void SymbolDataType::clear()
{
    while (!mList.isEmpty()) {
        SymbolDataType* t = mList.takeFirst();
        delete t;
    }
    delete mUndefined;
    mUndefined = nullptr;
}

SymbolDataType::SymbolDataType(SymbolType type, QString typeDescription) :
    mType(type), mDescription(typeDescription.split(",", QString::SkipEmptyParts))
{
}

} // namespace studio
} // namespace gams
