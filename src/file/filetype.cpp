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
#include "filetype.h"
#include "exception.h"

namespace gams {
namespace studio {

QList<FileType*> FileType::mList;
FileType *FileType::mNone = nullptr;

FileType::FileType(Kind kind, QString suffix, QString description, bool autoReload, const Kind dependant)
    : mKind(kind), mSuffix(suffix.split(",", QString::SkipEmptyParts)), mDescription(description)
    , mAutoReload(autoReload), mDependant(dependant)
{}

FileType::Kind FileType::kind() const
{
    return mKind;
}


FileType::Kind FileType::dependant() const
{
    return mDependant;
}

bool FileType::operator ==(const FileType& fileType) const
{
    return (this == &fileType);
}

bool FileType::operator !=(const FileType& fileType) const
{
    return (this != &fileType);
}

bool FileType::operator ==(const FileType::Kind& kind) const
{
    return (mKind == kind);
}

bool FileType::operator !=(const FileType::Kind& kind) const
{
    return (mKind != kind);
}

bool FileType::autoReload() const
{
    return mAutoReload;
}

QString FileType::description() const
{
    return mDescription;
}

QStringList FileType::suffix() const
{
    return mSuffix;
}

const QList<FileType*> FileType::list()
{
    if (mList.isEmpty()) {
        mList << new FileType(Gsp, "gsp,pro", "GAMS Studio Project", false);
        mList << new FileType(Gms, "gms,inc", "GAMS Source Code", false);
        mList << new FileType(Txt, "txt", "Text File", false);
        mList << new FileType(Lst, "lst", "GAMS List File", true);
        mList << new FileType(Lxi, "lxi", "GAMS List File Index", true, Lst);
        mList << new FileType(Gdx, "gdx", "GAMS Data", true);
        mList << new FileType(Ref, "ref", "GAMS Ref File", true);
        mList << new FileType(Log, "log", "GAMS Log File", false);
        mNone = new FileType(None, "", "Unknown File", false);
    }
    return mList;
}

FileType &FileType::from(QString suffix)
{
    for (FileType *ft: list()) {
        if (ft->mSuffix.contains(suffix, Qt::CaseInsensitive))
            return *ft;
    }
    return *mNone;
}

FileType& FileType::from(FileType::Kind kind)
{
    for (FileType *ft: list()) {
        if (ft->mKind == kind)
            return *ft;
    }
    return *mNone;
}

void FileType::clear()
{
    while (!mList.isEmpty()) {
        FileType* ft = mList.takeFirst();
        delete ft;
    }
    delete mNone;
    mNone = nullptr;
}

} // namespace studio
} // namespace gams
