/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "reference.h"

namespace gams {
namespace studio {
namespace reference {

Reference::Reference(QString referenceFile, QTextCodec* codec, QObject *parent) :
    QObject(parent), mCodec(codec), mReferenceFile(QDir::toNativeSeparators(referenceFile))
{
    loadReferenceFile(mCodec);
}

Reference::~Reference()
{
    clear();
}

QList<SymbolReferenceItem *> Reference::findReferenceFromType(SymbolDataType::SymbolType type)
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
    case SymbolDataType::Unused :
        return mUnusedReference;
    case SymbolDataType::Unknown :
        return mReference.values();
    default:
        return mReference.values();
    }

}

SymbolReferenceItem *Reference::findReferenceFromId(SymbolId symbolid)
{
    if (isValid() && mReference.contains(symbolid))
        return mReference[symbolid];

    return nullptr;
}

SymbolReferenceItem *Reference::findReferenceFromName(const QString &symbolName)
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
    return (mState == SuccessfullyLoaded);
}

int Reference::size() const
{
    return mReference.size();
}

QStringList Reference::getFileUsed() const
{
    return mFileUsed;
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

Reference::ReferenceState Reference::state() const
{
    return mState;
}

QTextCodec *Reference::codec() const
{
    return mCodec;
}

void Reference::loadReferenceFile(QTextCodec* codec)
{
    emit loadStarted();
    mCodec = codec;
    mState = ReferenceState::Loading;
    clear();
    mState = (parseFile(mReferenceFile) ?  ReferenceState::SuccessfullyLoaded : ReferenceState::UnsuccessfullyLoaded);
    emit loadFinished( mState == ReferenceState::SuccessfullyLoaded );
}

bool Reference::parseFile(QString referenceFile)
{
    QFile file(referenceFile);
    if(!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QTextStream in(&file);
    in.setCodec(mCodec);

    QStringList recordList;
    QString idx;
    while (!in.atEnd()) {
        QString line = in.readLine();
        recordList = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (recordList.size() <= 0)
            return false;
        idx = recordList.first();
        if (idx.toInt()== 0)         // start of symboltable
            break;
        if (recordList.size() < 11)  // unexpected size of elements
            return false;
        recordList.removeFirst();
        QString id = recordList.at(0);
        QString symbolName = recordList.at(1);
        QString symbolType = recordList.at(2);
        QString referenceType = recordList.at(3);
        QString lineNumber = recordList.at(5);
        QString columnNumber = recordList.at(6);
        QString location = line.mid(line.lastIndexOf(recordList.at(9)), line.length());

        SymbolDataType::SymbolType type = SymbolDataType::typeFrom(symbolType);
        if (!mReference.contains(id.toInt()))
            mReference[id.toInt()] = new SymbolReferenceItem(id.toInt(), symbolName, type);
        SymbolReferenceItem* ref = mReference[id.toInt()];
        addReferenceInfo(ref, referenceType, lineNumber.toInt(), columnNumber.toInt(), location);
    }
    if (in.atEnd())
        return false;


    // start of symboltable
    if (recordList.size() < 2)   // only the first two elements are used
        return false;
    recordList.removeFirst();
    int size = recordList.first().toInt();
    while (!in.atEnd()) {
        recordList = in.readLine().split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (recordList.size() <= 0 || recordList.size() < 6)   // unexpected size of elements
            return false;
        idx = recordList.first();
        QString id = recordList.at(0);

        if (!mReference.contains(id.toInt())) // ignore other unreferenced symbols
            continue;

        QString symbolName = recordList.at(1);
        QString dimension = recordList.at(4);
        QString numberOfElements = recordList.at(5);

        SymbolReferenceItem* ref = mReference[id.toInt()];
        ref->setDimension(dimension.toInt());
        mSymbolNameMap[symbolName] = id.toInt();

        QList<SymbolId> domain;
        if (dimension.toInt() > 0) {
            for(int dim=0; dim < dimension.toInt(); dim++) {
                QString d = recordList.at(6+dim);
                if (d.toInt() > 0) // if dimension > 0 and domain is specified
                    domain << d.toInt();
           }
        } // do not have dimension reference if dimension = 0
        ref->setDomain(domain);
        ref->setNumberOfElements(numberOfElements.toInt());
        QStringList text;
        // last element (explanatory text) may contains whitespaces
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
         if (ref->isUnused())
             mUnusedReference.append( ref );
         ++it;
    }

    return true;
}

void Reference::addReferenceInfo(SymbolReferenceItem* ref, const QString &referenceType, int lineNumber, int columnNumber, const QString &location)
{
    if (!mFileUsed.contains(QDir::toNativeSeparators(location)))
        mFileUsed << QDir::toNativeSeparators(location);

    ReferenceDataType::ReferenceType type = ReferenceDataType::typeFrom(referenceType);
    switch (type) {
    case ReferenceDataType::Declare :
        ref->addDeclare(new ReferenceItem(ref->id(), type, location, lineNumber, columnNumber));
        break;
    case ReferenceDataType::Define :
        ref->addDefine(new ReferenceItem(ref->id(), type, location, lineNumber, columnNumber));
        break;
    case ReferenceDataType::Assign :
        ref->addAssign(new ReferenceItem(ref->id(), type, location, lineNumber, columnNumber));
        break;
    case ReferenceDataType::ImplicitAssign :
        ref->addImplicitAssign(new ReferenceItem(ref->id(), type, location, lineNumber, columnNumber));
        break;
    case ReferenceDataType::Reference :
        ref->addReference(new ReferenceItem(ref->id(), type, location, lineNumber, columnNumber));
        break;
    case ReferenceDataType::Control :
        ref->addControl(new ReferenceItem(ref->id(), type, location, lineNumber, columnNumber));
        break;
    case ReferenceDataType::Index :
        ref->addIndex(new ReferenceItem(ref->id(), type, location, lineNumber, columnNumber));
        break;
    default:
        break;
    }
}

void Reference::clear()
{
    mSetReference.clear();
    mAcronymReference.clear();
    mParReference.clear();
    mVarReference.clear();
    mEquReference.clear();
    mFileReference.clear();
    mModelReference.clear();
    mFunctionReference.clear();
    mUnusedReference.clear();

    mFileUsed.clear();

    mSymbolNameMap.clear();

    qDeleteAll(mReference);
    mReference.clear();
}

} // namespace reference
} // namespace studio
} // namespace gams
