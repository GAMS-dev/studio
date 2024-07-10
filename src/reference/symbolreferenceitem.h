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
#ifndef SYMBOLREFERENCEITEM_H
#define SYMBOLREFERENCEITEM_H

#include <QtCore>
#include "referencedatatype.h"
#include "symboldatatype.h"

namespace gams {
namespace studio {
namespace reference {

typedef int SymbolId;

struct ReferenceItem {
    ReferenceItem() { }
    ReferenceItem(SymbolId id, ReferenceDataType::ReferenceType type, const QString &loc, int line, int col) :
        symbolID(id), referenceType(type), location(loc), lineNumber(line), columnNumber(col) { }

    SymbolId symbolID = -1;
    ReferenceDataType::ReferenceType referenceType = ReferenceDataType::ReferenceType::Unknown;
    QString location;
    int lineNumber = -1;
    int columnNumber = -1;
};

class SymbolReferenceItem
{
public:
    SymbolReferenceItem(SymbolId id, const QString &name, SymbolDataType::SymbolType type);
    ~SymbolReferenceItem();

    QString name() const;
    SymbolId id() const;
    SymbolDataType::SymbolType type() const;

    int dimension() const;
    void setDimension(int dimension);

    QList<SymbolId> domain() const;
    void setDomain(const QList<SymbolId> &domain);

    int numberOfElements() const;
    void setNumberOfElements(int number);

    QString explanatoryText() const;
    void setExplanatoryText(const QString &text);

    QList<ReferenceItem *> define() const;
    void addDefine(ReferenceItem* define);

    QList<ReferenceItem *> declare() const;
    void addDeclare(ReferenceItem* declare);

    QList<ReferenceItem *> assign() const;
    void addAssign(ReferenceItem* assign);

    QList<ReferenceItem *> implicitAssign() const;
    void addImplicitAssign(ReferenceItem* implassign);

    QList<ReferenceItem *> reference() const;
    void addReference(ReferenceItem* reference);

    QList<ReferenceItem *> control() const;
    void addControl(ReferenceItem* control);

    QList<ReferenceItem *> index() const;
    void addIndex(ReferenceItem* index);

    bool isDefined() const;
    bool isAssigned() const;
    bool isImplicitAssigned() const;
    bool isReferenced() const;
    bool isControlled() const;
    bool isIndexed() const;
    bool isUnused() const;

private:
    SymbolId mID;
    QString mName;
    SymbolDataType::SymbolType mType;
    int mDimension;
    QList<SymbolId> mDomain;
    int mNumberOfElements;
    QString mExplanatoryText;
    QList<ReferenceItem *> mDefine;
    QList<ReferenceItem *> mDeclare;
    QList<ReferenceItem *> mAssign;
    QList<ReferenceItem *> mImplicitAssign;
    QList<ReferenceItem *> mReference;
    QList<ReferenceItem *> mControl;
    QList<ReferenceItem *> mIndex;
};

} // namespace reference
} // namespace studio
} // namespace gams

Q_DECLARE_METATYPE(gams::studio::reference::ReferenceItem)

#endif // SYMBOLREFERENCEITEM_H
