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
#ifndef REFERENCE_H
#define REFERENCE_H

#include <QString>
#include <QMap>
#include <QDir>

#include "symbolreferenceitem.h"

namespace gams {
namespace studio {

class Reference
{
public:
    Reference(QString referenceFile);
    ~Reference();

    QList<SymbolReferenceItem *> findReference(SymbolDataType::SymbolType type);

    SymbolReferenceItem* findReference(SymbolId symbolid);
    SymbolReferenceItem* findReference(const QString &symbolName);

    bool contains(SymbolId id) const;
    bool contains(const QString &symbolName) const;
    bool isEmpty() const;
    bool isValid() const;
    int size() const;

    QList<SymbolId> symbolIDList() const;
    QList<QString> symbolNameList() const;

    QString getFileLocation() const;

    void dumpAll();

private:
    bool parseFile(QString referenceFile);
    void addReferenceInfo(SymbolReferenceItem* ref, const QString &referenceType, int lineNumber, int columnNumber, const QString &location);

    bool mValid;
    QString mReferenceFile;

    QList<SymbolReferenceItem *> mSetReference;
    QList<SymbolReferenceItem *> mAcronymReference;
    QList<SymbolReferenceItem *> mParReference;
    QList<SymbolReferenceItem *> mVarReference;
    QList<SymbolReferenceItem *> mEquReference;
    QList<SymbolReferenceItem *> mFileReference;
    QList<SymbolReferenceItem *> mModelReference;
    QList<SymbolReferenceItem *> mFunctionReference;
    QList<SymbolReferenceItem *> mUnusedReference;

    QMap<QString, SymbolId> mSymbolNameMap;
    QMap<SymbolId, SymbolReferenceItem*> mReference;
};

} // namespace studio
} // namespace gams

#endif // REFERENCE_H
