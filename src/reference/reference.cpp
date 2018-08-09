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
#include <QDebug>

#include "reference.h"

namespace gams {
namespace studio {

Reference::Reference(QString referenceFile)
{
    mReferenceFile = QDir::toNativeSeparators(referenceFile);
    mValid = parseFile(mReferenceFile);
}

Reference::~Reference()
{
    mSetReference.clear();
    mAcronymReference.clear();
    mParReference.clear();
    mVarReference.clear();
    mEquReference.clear();
    mFileReference.clear();
    mModelReference.clear();
    mFunctionReference.clear();

    mSymbolNameMap.clear();

    qDeleteAll(mReference);
    mReference.clear();
}

QList<SymbolReferenceItem *> Reference::findReference(SymbolDataType::SymbolType type)
{
    switch(type) {
    case SymbolDataType::Set :
        return mSetReference;
    case SymbolDataType::Acronym :
        return mAcronymReference;
    case SymbolDataType::Parameter :
        return mParReference;
    case SymbolDataType::Variable :
        return mVarReference;
    case SymbolDataType::Equation :
        return mEquReference;
    case SymbolDataType::File :
        return mFileReference;
    case SymbolDataType::Model :
        return mModelReference;
    case SymbolDataType::Funct :
        return mFunctionReference;
    default:
        return mReference.values();
    }

}

SymbolReferenceItem *Reference::findReference(SymbolId symbolid)
{
    if (isValid() && mReference.contains(symbolid))
        return mReference[symbolid];

    return nullptr;
}

SymbolReferenceItem *Reference::findReference(const QString &symbolName)
{
    if (isValid() && mSymbolNameMap.contains(symbolName)) {
        SymbolId id =  mSymbolNameMap[symbolName];
        if (mReference.contains(id))
           return mReference[id];
    }

    return nullptr;
}

bool Reference::contains(SymbolId id) const
{
    return mReference.contains(id);
}

bool Reference::contains(const QString &symbolName) const
{
    return mSymbolNameMap.contains(symbolName);
}

bool Reference::isEmpty() const
{
    return mReference.isEmpty();
}


bool Reference::isValid() const
{
    return mValid;
}

int Reference::size() const
{
    return mReference.size();
}

QList<SymbolId> Reference::symbolIDList() const
{
    return mReference.keys();
}

QList<QString> Reference::symbolNameList() const
{
    return mSymbolNameMap.keys();
}

QString Reference::getFileLocation() const
{
    return mReferenceFile;
}

bool Reference::parseFile(QString referenceFile)
{
    QFile file(referenceFile);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "cannot open file [" << referenceFile << "]";
        return false;
    }
    QTextStream in(&file);

    QStringList recordList;
    QString idx;
    while (!in.atEnd()) {
        recordList = in.readLine().split(' ');
        idx = recordList.first();
        if (idx.toInt()== 0)
            break;
        recordList.removeFirst();
        QString id = recordList.at(0);
        QString symbolName = recordList.at(1);
        QString symbolType = recordList.at(2);
        QString referenceType = recordList.at(3);
        QString lineNumber = recordList.at(5);
        QString columnNumber = recordList.at(6);
        QString location = recordList.at(9);

        SymbolDataType::SymbolType type = SymbolDataType::typeFrom(symbolType);
        if (!mReference.contains(id.toInt()))
            mReference[id.toInt()] = new SymbolReferenceItem(id.toInt(), symbolName, type);
        SymbolReferenceItem* ref = mReference[id.toInt()];
        addReferenceInfo(ref, referenceType, lineNumber.toInt(), columnNumber.toInt(), location);
    }
    if (in.atEnd())
        return false;

    recordList.removeFirst();
    int size = recordList.first().toInt();
    while (!in.atEnd()) {
        recordList = in.readLine().split(' ');
        idx = recordList.first();
        QString id = recordList.at(0);

        if (!mReference.contains(id.toInt())) // ignore other unreferenced symbols
            continue;

        QString symbolName = recordList.at(1);
//        QString symbolType = recordList.at(3);
        QString dimension = recordList.at(4);
        QString numberOfElements = recordList.at(5);

        SymbolReferenceItem* ref = mReference[id.toInt()];
        ref->setDimension(dimension.toInt());
        mSymbolNameMap[symbolName] = id.toInt();

        QList<SymbolId> domain;
        if (dimension.toInt() > 0) {
            for(int dim=0; dim < dimension.toInt(); dim++) {
                QString d = recordList.at(6+dim);
                if (d.toInt() > 0)
                    domain << d.toInt();
           }
        } // do not have dimension reference if dimension = 0;
        ref->setDomain(domain);
        ref->setNumberOfElements(numberOfElements.toInt());
        QStringList text;
        for (int idx=6+dimension.toInt(); idx< recordList.size(); idx++)
            text << recordList.at(idx);
        ref->setExplanatoryText(text.join(' '));
    }

    if (idx.toInt()!=size)
        return false;

     QMap<SymbolId, SymbolReferenceItem*>::const_iterator it = mReference.constBegin();
     while (it != mReference.constEnd()) {
         SymbolReferenceItem* ref = it.value();
         switch(ref->type()) {
         case SymbolDataType::Set :
             mSetReference.append( ref );
             break;
         case SymbolDataType::Acronym :
             mAcronymReference.append( ref );
             break;
         case SymbolDataType::Parameter :
             mParReference.append( ref );
             break;
         case SymbolDataType::Variable :
             mVarReference.append( ref );
             break;
         case SymbolDataType::Equation :
             mEquReference.append( ref );
             break;
         case SymbolDataType::File :
             mFileReference.append( ref );
             break;
         case SymbolDataType::Model :
             mModelReference.append( ref );
             break;
         case SymbolDataType::Funct :
             mFunctionReference.append( ref );
             break;
         default:
             break;
         }
         ++it;
    }

    return true;
}

void Reference::addReferenceInfo(SymbolReferenceItem *ref, const QString &referenceType, int lineNumber, int columnNumber, const QString &location)
{
    if (QString::compare(referenceType, "declared", Qt::CaseInsensitive)==0) {
        ref->addDeclare(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "defined", Qt::CaseInsensitive)==0) {
        ref->addDefine(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "assigned", Qt::CaseInsensitive)==0) {
        ref->addAssign(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "impl-assign", Qt::CaseInsensitive)==0) {
        ref->addImplicitAssign(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "ref", Qt::CaseInsensitive)==0) {
        ref->addReference(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "control", Qt::CaseInsensitive)==0) {
        ref->addControl(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "index", Qt::CaseInsensitive)==0) {
        ref->addIndex(new ReferenceItem(location, lineNumber, columnNumber));
    }
}

void Reference::dumpAll()
{
    QMap<SymbolId, SymbolReferenceItem*>::const_iterator i = mReference.constBegin();
    while (i != mReference.constEnd()) {
        qDebug() << "--- id=" << i.key() << "---";
        SymbolReferenceItem* ref = i.value();
        ref->dumpAll();
        ++i;
    }
}

} // namespace studio
} // namespace gams
