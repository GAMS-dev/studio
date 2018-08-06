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
#ifndef SYMBOLREFERENCEITEM_H
#define SYMBOLREFERENCEITEM_H

#include "symboldatatype.h"

namespace gams {
namespace studio {

typedef int SymbolId;

struct ReferenceItem {
    ReferenceItem(QString loc, int line, int col) :
        location(loc), lineNumber(line), columnNumber(col) { }

    QString location;
    int lineNumber;
    int columnNumber;
};

class SymbolReferenceItem
{
public:
    SymbolReferenceItem(SymbolId id, QString name, SymbolDataType type);
    ~SymbolReferenceItem();

    QString name() const;
    SymbolId id() const;
    SymbolDataType type() const;

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

    void dumpAll();
private:
    SymbolId mID;
    SymbolDataType mType;
    QString mName;
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

} // namespace studio
} // namespace gams

#endif // SYMBOLREFERENCEITEM_H
