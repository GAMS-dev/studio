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
#include <QDebug>
#include "symbolreferenceitem.h"

namespace gams {
namespace studio {
namespace reference {

SymbolReferenceItem::SymbolReferenceItem(SymbolId id, const QString &name, SymbolDataType::SymbolType type) :
    mID(id), mName(name), mType(type)
{

}

SymbolReferenceItem::~SymbolReferenceItem()
{
    mDomain.clear();

    qDeleteAll(mDefine);
    mDefine.clear();

    qDeleteAll(mDeclare);
    mDeclare.clear();

    qDeleteAll(mAssign);
    mAssign.clear();

    qDeleteAll(mImplicitAssign);
    mImplicitAssign.clear();

    qDeleteAll(mReference);
    mReference.clear();

    qDeleteAll(mControl);
    mControl.clear();

    qDeleteAll(mIndex);
    mIndex.clear();
}

SymbolDataType::SymbolType SymbolReferenceItem::type() const
{
    return mType;
}

SymbolId SymbolReferenceItem::id() const
{
    return mID;
}

QString SymbolReferenceItem::name() const
{
    return mName;
}

int SymbolReferenceItem::dimension() const
{
    return mDimension;
}

void SymbolReferenceItem::setDimension(int dimension)
{
    mDimension = dimension;
}

QList<SymbolId> SymbolReferenceItem::domain() const
{
    return mDomain;
}

void SymbolReferenceItem::setDomain(const QList<SymbolId> &domain)
{
    mDomain = domain;
}

int SymbolReferenceItem::numberOfElements() const
{
    return mNumberOfElements;
}

void SymbolReferenceItem::setNumberOfElements(int number)
{
    mNumberOfElements = number;
}

QString SymbolReferenceItem::explanatoryText() const
{
    return mExplanatoryText;
}

void SymbolReferenceItem::setExplanatoryText(const QString &text)
{
    mExplanatoryText = text;
}

QList<ReferenceItem *> SymbolReferenceItem::define() const
{
    return mDefine;
}

void SymbolReferenceItem::addDefine(ReferenceItem *define)
{
    mDefine.append(define);
}

QList<ReferenceItem *> SymbolReferenceItem::declare() const
{
    return mDeclare;
}

void SymbolReferenceItem::addDeclare(ReferenceItem *declare)
{
    mDeclare.append(declare);
}

QList<ReferenceItem *> SymbolReferenceItem::assign() const
{
    return mAssign;
}

void SymbolReferenceItem::addAssign(ReferenceItem *assign)
{
    mAssign.append(assign);
}

QList<ReferenceItem *> SymbolReferenceItem::implicitAssign() const
{
    return mImplicitAssign;
}

void SymbolReferenceItem::addImplicitAssign(ReferenceItem *implassign)
{
    mImplicitAssign.append(implassign);
}

QList<ReferenceItem *> SymbolReferenceItem::reference() const
{
    return mReference;
}

void SymbolReferenceItem::addReference(ReferenceItem *reference)
{
    mReference.append(reference);
}

QList<ReferenceItem *> SymbolReferenceItem::control() const
{
    return mControl;
}

void SymbolReferenceItem::addControl(ReferenceItem *control)
{
    mControl.append(control);
}

QList<ReferenceItem *> SymbolReferenceItem::index() const
{
    return mIndex;
}

void SymbolReferenceItem::addIndex(ReferenceItem *index)
{
    mIndex.append(index);
}

bool SymbolReferenceItem::isDefined() const
{
    return (mDefine.isEmpty());
}

bool SymbolReferenceItem::isAssigned() const
{
    return (mAssign.isEmpty());
}

bool SymbolReferenceItem::isImplicitAssigned() const
{
    return (mImplicitAssign.isEmpty());
}

bool SymbolReferenceItem::isReferenced() const
{
    return (mReference.isEmpty());
}

bool SymbolReferenceItem::isControlled() const
{
    return (mControl.isEmpty());
}

bool SymbolReferenceItem::isIndexed() const
{
    return (mIndex.isEmpty());
}

bool SymbolReferenceItem::isUnused() const
{
    return (mAssign.size()+mImplicitAssign.size()+mReference.size()+mControl.size()+mIndex.size() == 0);
}

} // namespace reference
} // namespace studio
} // namespace gams
