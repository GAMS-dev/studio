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
#include "symboldatatype.h"

namespace gams {
namespace studio {
namespace reference {

QList<SymbolDataType*> SymbolDataType::mList;
SymbolDataType* SymbolDataType::mUnknown = nullptr;

SymbolDataType::SymbolType SymbolDataType::type() const
{
    return mType;
}

QString SymbolDataType::name() const
{
    return mDescription.last();
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
        mUnknown = new SymbolDataType(Unknown, "UNKNOWN,,Unknown");
        mList << new SymbolDataType(Funct, "FUNCT,FUNCTIONS,Function");
        mList << new SymbolDataType(Set, "SET,SETS,Set");
        mList << new SymbolDataType(Acronym, "ACRNM,ACRONYMS,Acronym");
        mList << new SymbolDataType(Parameter, "PARAM,PARAMETERS,Parameter");
        mList << new SymbolDataType(Variable, "VAR,VARIABLES,Variable");
        mList << new SymbolDataType(Equation, "EQU,EQUATIONS,Equation");
        mList << new SymbolDataType(Model, "MODEL,MODELS,Model");
        mList << new SymbolDataType(File, "FILE,FILES,File");
        mList << new SymbolDataType(Macro, "MACRO,MACROS,Macro");
        mList << new SymbolDataType(Unused, "UNUSED,,Unused");
    }
    return mList;
}

SymbolDataType::SymbolType SymbolDataType::typeFrom(const QString& typeDescription)
{
    for (SymbolDataType* t : list()) {
        if (t->mDescription.contains(typeDescription, Qt::CaseInsensitive))
            return t->type();
    }
    return Unknown;
}

SymbolDataType &SymbolDataType::from(const QString& typeDescription)
{
    for (SymbolDataType* t : list()) {
        if (t->mDescription.contains(typeDescription, Qt::CaseInsensitive))
            return *t;
    }
    return *mUnknown;
}

SymbolDataType &SymbolDataType::from(SymbolDataType::SymbolType type)
{
    for (SymbolDataType* t : list()) {
        if (t->mType == type)
            return *t;
    }
    return *mUnknown;
}

void SymbolDataType::clear()
{
    while (!mList.isEmpty()) {
        SymbolDataType* t = mList.takeFirst();
        delete t;
    }
    delete mUnknown;
    mUnknown = nullptr;
}

SymbolDataType::SymbolDataType(SymbolType type, const QString& typeDescription) :
    mType(type), mDescription(typeDescription.split(",", Qt::SkipEmptyParts))
{
}

} // namespace reference
} // namespace studio
} // namespace gams
