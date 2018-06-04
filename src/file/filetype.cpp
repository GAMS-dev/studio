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

FileType::FileType(FileKind kind, QString suffix, QString description, bool autoReload)
    : mKind(kind), mSuffix(suffix.split(",", QString::SkipEmptyParts)), mDescription(description)
    , mAutoReload(autoReload)
{}

FileKind FileType::kind() const
{
    return mKind;
}

bool FileType::operator ==(const FileType& fileType) const
{
    return (this == &fileType);
}

bool FileType::operator !=(const FileType& fileType) const
{
    return (this != &fileType);
}

bool FileType::operator ==(const FileKind &kind) const
{
    return (mKind == kind);
}

bool FileType::operator !=(const FileKind &kind) const
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
        mList << new FileType(FileKind::Gsp, "gsp,pro", "GAMS Studio Project", false);
        mList << new FileType(FileKind::Gms, "gms,inc", "GAMS Source Code", false);
        mList << new FileType(FileKind::Txt, "txt", "Text File", false);
        mList << new FileType(FileKind::Lst, "lst", "GAMS List File", true);
        mList << new FileType(FileKind::Lxi, "lxi", "GAMS List File Index", true);
        mList << new FileType(FileKind::Gdx, "gdx", "GAMS Data", true);
        mList << new FileType(FileKind::Ref, "ref", "GAMS Ref File", true);
        mList << new FileType(FileKind::Log, "log", "GAMS Log File", false);
        mNone = new FileType(FileKind::None, "", "Unknown File", false);
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

FileType& FileType::from(FileKind kind)
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
