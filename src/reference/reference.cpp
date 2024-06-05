/*
 * This file is part of the GAMS Studio project.
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
#include "reference.h"

#include <QtCore5Compat/QTextCodec>

namespace gams {
namespace studio {
namespace reference {

QRegularExpression Reference::mRexSplit("\\s+");

Reference::Reference(const QString &referenceFile, const QString &encodingName, QObject *parent) :
    QObject(parent), mReferenceFile(QDir::toNativeSeparators(referenceFile))
{
    loadReferenceFile(encodingName, false);
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
    case SymbolDataType::Macro :
        return mMacroReference;
    case SymbolDataType::Unused :
        return mUnusedReference;
    case SymbolDataType::Unknown :
        return (mReference.isEmpty() ? QList<SymbolReferenceItem*>() : mReference.values());
    default:
        return (mReference.isEmpty() ? QList<SymbolReferenceItem*>() : mReference.values());
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

FileReferenceItem* Reference::getFileUsedReference(FileReferenceId id)
{
    return mFileUsedReference[id];
}

int Reference::getNumberOfFileUsed() const
{
    return (mFileUsedReference.size()<=0 ? mFileUsed.size() : mFileUsedReference.size()-1);
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

int Reference::errorLine() const
{
    return  mLastErrorLine;
}

void Reference::loadReferenceFile(const QString &encodingName, bool triggerReload)
{
    if (triggerReload) {
        mLastErrorLine = -1;
        mLastReadFileSize = 0;
        clear();
    }
    emit loadStarted();

    mState = ReferenceState::Loading;
    qint64 lastReadFileSize = parseFile(mReferenceFile, encodingName);
    if (mLastErrorLine==-1) {
        mPendingReload = false;
        mState = ReferenceState::SuccessfullyLoaded;
    } else {
        if (!mPendingReload) {
            if (triggerReload || lastReadFileSize!=mLastReadFileSize) {
                mPendingReload = true;
                mState = ReferenceState::Loading;
            } else {
                mPendingReload = false;
                mState = ReferenceState::UnsuccessfullyLoaded;
            }
        } else {
            mPendingReload = false;
            mState = ReferenceState::UnsuccessfullyLoaded;
        }
    }
    emit loadFinished( mState == ReferenceState::SuccessfullyLoaded, mPendingReload );
    mLastReadFileSize = lastReadFileSize;
    return;
}

qint64 Reference::parseFile(const QString &referenceFile, const QString &encodingName)
{
    QFile file(referenceFile);
    int lineread = 0;
    if(!file.open(QFile::ReadOnly)) {
        mLastErrorLine=lineread;
        return file.size();
    }
    QTextCodec *codec = QTextCodec::codecForName(encodingName.toLatin1());
    if (!codec) codec = QTextCodec::codecForName("UTF-8");

    if (file.size() == mLastReadFileSize) {
        mLastErrorLine=lineread;
        return file.size();
    }
    QStringList recordList;
    QString idx;
    while (!file.atEnd()) {
        QByteArray arry = file.readLine();
        // TODO(JM) when switching back to QTextStream this can be removed, as stream doesn't append the \n
        if (arry.endsWith('\n')) {
           if (arry.length() > 1 && arry.at(arry.length()-2) == '\r')
               arry.chop(2);
           else
               arry.chop(1);
        }

        QString line = codec ? codec->toUnicode(arry) : QString(arry);
        lineread++;
        recordList = line.split(mRexSplit, Qt::SkipEmptyParts);
        if (recordList.size() <= 0) {
           addSymbolReferenceItem();
           mLastErrorLine=lineread;
           return file.size();
        }
        idx = recordList.first();
        if (idx.toInt()== 0)         // start of symboltable
            break;
        if (recordList.size() < 11)  { // unexpected size of elements
            addSymbolReferenceItem();
            mLastErrorLine=lineread;
            return file.size();
        }
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
    if (file.atEnd()) {
        addSymbolReferenceItem();
        mLastErrorLine=lineread;
        return file.size();
    }
    // start of symboltable
    if (recordList.size() < 2)  { // only the first two elements are used
        addSymbolReferenceItem();
        mLastErrorLine=lineread;
        return file.size();
    }
    recordList.removeFirst();
    int size = recordList.first().toInt();
    int expectedLineread = size+lineread;
    while (!file.atEnd()) {
        lineread++;
        if (lineread > expectedLineread)  // ignore the rest of the file contents
            break;
        QByteArray arry = file.readLine();
        // TODO(JM) when switching back to QTextStream this can be removed, as stream doesn't append the \n
        if (arry.endsWith('\n')) {
            if (arry.length() > 1 && arry.at(arry.length()-2) == '\r')
                arry.chop(2);
            else
                arry.chop(1);
        }

        QStringList recordList = (codec ? codec->toUnicode(arry) : QString(arry)).split(mRexSplit, Qt::SkipEmptyParts);

        if (recordList.size() <= 0 || recordList.size() < 6) { // unexpected size of elements
            addSymbolReferenceItem();
            mLastErrorLine=lineread;
            return file.size();
        }
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
                const QString &d = recordList.at(6+dim);
                if (d.toInt() > 0) // if dimension > 0 and domain is specified
                    domain << d.toInt();
           }
        } // do not have dimension reference if dimension = 0
        ref->setDomain(domain);
        ref->setNumberOfElements(numberOfElements.toInt());
        QStringList text;
        // last element (explanatory text) may contains whitespaces
        for (int i=6+dimension.toInt(); i< recordList.size(); i++)
            text << recordList.at(i);
        ref->setExplanatoryText(text.join(' '));
    }
    if (lineread > expectedLineread && !file.atEnd()) {
        QByteArray arry = file.readLine();
        // TODO(JM) when switching back to QTextStream this can be removed, as stream doesn't append the \n
        if (arry.endsWith('\n')) {
            if (arry.length() > 1 && arry.at(arry.length()-2) == '\r')
                arry.chop(2);
            else
                arry.chop(1);
        }

        QStringList recordList = (codec ? codec->toUnicode(arry) : QString(arry)).split(mRexSplit, Qt::SkipEmptyParts);
        if (recordList.isEmpty()) {
            addSymbolReferenceItem();
            mLastErrorLine=lineread;
            return file.size();
        }

        int id = recordList.first().toInt();
        if (id != 0) {
            addSymbolReferenceItem();
            mLastErrorLine=lineread;
            return file.size();
        }
        QList<QVariant> rootdata;
        rootdata << QVariant(id) << QVariant() << QVariant()
                 << QVariant()   << QVariant();
        FileReferenceItem* item = new FileReferenceItem(rootdata);
        mFileUsedReference[id] = item;

        recordList.removeFirst();
        size = recordList.first().toInt();
        expectedLineread += size;
        while (!file.atEnd()) {
            if (lineread > expectedLineread)
                break;
            QByteArray arry = file.readLine();
            // TODO(JM) when switching back to QTextStream this can be removed, as stream doesn't append the \n
            if (arry.endsWith('\n')) {
                if (arry.length() > 1 && arry.at(arry.length()-2) == '\r')
                    arry.chop(2);
                else
                    arry.chop(1);
            }

            QStringList recordList = (codec ? codec->toUnicode(arry) : QString(arry)).split(mRexSplit, Qt::SkipEmptyParts);
            if (recordList.size() < 6)
                break;
            id = recordList.first().toInt();
            QString globalLineno   = recordList.at(1);
            QString referenceType  = recordList.at(2);
            QString parentIdx      = recordList.at(3);
            QString localLineumber = recordList.at(4);
            QString location       = recordList.at(5);
            for (int i=6; i<recordList.size(); ++i) {
                location += " ";
                location += recordList.at(i);
            }
            QList<QVariant> data;
            data << QVariant( location )   << QVariant(referenceType)
                 << QVariant(globalLineno) << QVariant(localLineumber) << QVariant(id);
            int parentId = parentIdx.toInt();
            if (!mFileUsedReference.contains(parentId))
                break;
            FileReferenceItem* parentItem = mFileUsedReference[parentId];
            item = new FileReferenceItem(data, parentItem);
            parentItem->appendChild( item );
            mFileUsedReference[id]  = item;
            lineread++;
        }
     }
     if (mFileUsedReference.size()==0 && mFileUsed.size()>0) {
         int id = 0;
         QList<QVariant> rootdata;
         rootdata << QVariant(id) << QVariant() << QVariant()
                  << QVariant()   << QVariant();
         FileReferenceItem* rootitem = new FileReferenceItem(rootdata);
         mFileUsedReference[id] = rootitem;
         for(int i=0; i<mFileUsed.size(); ++i) {
             id = i+1;
             QList<QVariant> data;
             data << QVariant( mFileUsed.at(i) ) << QVariant()
                  << QVariant() << QVariant() << QVariant(id);
             FileReferenceItem* item = new FileReferenceItem(data, rootitem);
             rootitem->appendChild( item );
             mFileUsedReference[id]  = item;
         }
     }
     if (lineread <= expectedLineread) {
         addSymbolReferenceItem();
         mLastErrorLine=lineread;
         return file.size();
     }
     addSymbolReferenceItem();
     mLastErrorLine=-1;
     return file.size();
;
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

void Reference::addSymbolReferenceItem() {
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
        case SymbolDataType::Model :
             mModelReference.append( ref );
             break;
        case SymbolDataType::Funct :
             mFunctionReference.append( ref );
             break;
        case SymbolDataType::Macro :
             mMacroReference.append( ref );
             break;
        case SymbolDataType::File :
             mFileReference.append( ref );
             break;
        default:
             break;
        }
        if (ref->isUnused())
             mUnusedReference.append( ref );
        ++it;
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
    mMacroReference.clear();
    mUnusedReference.clear();

    mFileUsed.clear();

    mSymbolNameMap.clear();

    qDeleteAll(mFileUsedReference);
    mFileUsedReference.clear();

    qDeleteAll(mReference);
    mReference.clear();
}

} // namespace reference
} // namespace studio
} // namespace gams
