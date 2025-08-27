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
#include "referencedatatype.h"

namespace gams {
namespace studio {
namespace reference {

QList<ReferenceDataType*> ReferenceDataType::mList;
ReferenceDataType* ReferenceDataType::mUnknown = nullptr;

ReferenceDataType::ReferenceType ReferenceDataType::type() const
{
    return mType;
}

QString ReferenceDataType::name() const
{
    return mName.last();
}

QString ReferenceDataType::description() const
{
    return mDescription;
}

bool ReferenceDataType::operator ==(const ReferenceDataType &refDataType) const
{
    return (this == &refDataType);
}

bool ReferenceDataType::operator !=(const ReferenceDataType &refDataType) const
{
    return (this != &refDataType);
}

bool ReferenceDataType::operator ==(const ReferenceDataType::ReferenceType &type) const
{
    return (mType == type);
}

bool ReferenceDataType::operator !=(const ReferenceDataType::ReferenceType &type) const
{
    return (mType != type);
}

const QList<ReferenceDataType*> ReferenceDataType::list()
{
    if (mList.isEmpty()) {
        mUnknown = new ReferenceDataType(Unknown, "Unknown,Unknown", "Unknown reference type");
        mList << new ReferenceDataType(Declare, "declared,Declared", "The identifier is declared as to type. This must be the first appearance of the identifier");
        mList << new ReferenceDataType(Define, "defined,Defined", "An initialization (for a table or a data list between slashes) or symbolic definition (for an equation) starts for the identifier");
        mList << new ReferenceDataType(Assign, "assign,Assigned", "Values are replaced because the identifier appears on the left-hand side of an assignment statement");
        mList << new ReferenceDataType(ImplicitAssign, "impl-asn,Implicitly Assigned", "An equation or variable will be updated as a result of being referred to implicitly in a solve statement");
        mList << new ReferenceDataType(Control, "control,Controlled", "A set is used as (part of) the driving index in an assignment, equation, loop or indexed operation");
        mList << new ReferenceDataType(Reference, "ref,Referenced", "The symbol has been referenced on the right-hand side of an assignment or in a display, equation, model, solve statement or put statetement");
        mList << new ReferenceDataType(Index, "index,Indexed", "A set is used as (part of) the driving index only for set labels. Appears only in the cross reference map of unique elements");
    }
    return mList;
}

ReferenceDataType::ReferenceType ReferenceDataType::typeFrom(int type)
{
    for (ReferenceDataType* t : list()) {
        if (type==static_cast<int>(t->type()))
            return t->type();
    }
    return Unknown;

}

ReferenceDataType::ReferenceType ReferenceDataType::typeFrom(const QString &name)
{
    for (ReferenceDataType* t : list()) {
        if (t->mName.contains(name, Qt::CaseInsensitive))
            return t->type();
    }
    return Unknown;
}


ReferenceDataType &ReferenceDataType::from(const QString& name)
{
    for (ReferenceDataType* t : list()) {
        if (t->mName.contains(name, Qt::CaseInsensitive))
            return *t;
    }
    return *mUnknown;
}

ReferenceDataType &ReferenceDataType::from(const ReferenceType type)
{
    for (ReferenceDataType* t : list()) {
        if (t->mType == type)
            return *t;
    }
    return *mUnknown;
}

void ReferenceDataType::clear()
{
    while (!mList.isEmpty()) {
        ReferenceDataType* t = mList.takeFirst();
        delete t;
    }
    delete mUnknown;
    mUnknown = nullptr;
}

ReferenceDataType::ReferenceDataType(ReferenceDataType::ReferenceType type, const QString& name, const QString &typeDescription) :
    mType(type), mName(name.split(",", Qt::SkipEmptyParts)), mDescription(typeDescription)
{

}

} // namespace reference
} // namespace studio
} // namespace gams
