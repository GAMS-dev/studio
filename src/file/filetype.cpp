/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "filetype.h"
#include "exception.h"
#include "theme.h"

namespace gams {
namespace studio {

const QStringList FileType::CInvalidUserTypes {"", "ref", "gdx", "log", "lst", "lxi", "opt", "yaml", "exe", "efi"};

QList<FileType*> FileType::mFileTypes {
    new FileType(FileKind::PrO, {""}, "Project Options", false),
    new FileType(FileKind::Gsp, {"gsp" ,"pro"}, "GAMS Studio Project", false),
    new FileType(FileKind::Gms, {"gms", "inc", "dmp"}, "GAMS Source Code", false),
    new FileType(FileKind::Txt, {"txt"}, "Text File (editable)", false),
    new FileType(FileKind::TxtRO, {"log"}, "Text File (read only)", false),
    new FileType(FileKind::Lst, {"lst"}, "GAMS List File", true),
    new FileType(FileKind::Lxi, {"lxi"}, "GAMS List File Index", true),
    new FileType(FileKind::Gdx, {"gdx"}, "GAMS Data", true),
    new FileType(FileKind::Ref, {"ref"}, "GAMS Ref File", true),
    new FileType(FileKind::Log, {"~log"}, "GAMS Log File", true),
    new FileType(FileKind::Opt, {"opt"}, "Solver Option File", false),
    new FileType(FileKind::Guc, {"yaml"}, "GAMS Config File", false),
    new FileType(FileKind::Efi, {"efi"}, "External Files", false),
};

FileType *FileType::mNone = new FileType(FileKind::None, {""}, "Unknown File", false);
int FileType::mGmsFixedTypes = 0;
QStringList FileType::mUserGamsTypes;

FileType::FileType(FileKind kind, QStringList suffix, QString description, bool autoReload)
    : mKind(kind), mSuffix(suffix), mDescription(description)
    , mAutoReload(autoReload)
{
    if (mGmsFixedTypes == 0 && kind == FileKind::Gms) mGmsFixedTypes = mSuffix.length();
}

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

QString FileType::defaultSuffix() const
{
    return mSuffix.first();
}

FileType &FileType::from(QString suffix)
{
    for (FileType *ft: qAsConst(mFileTypes)) {
        if (ft->mSuffix.contains(suffix, Qt::CaseInsensitive))
            return *ft;
    }

    QString pattern("[oO][pP][2-9]|[oO][1-9]\\d|[1-9]\\d\\d+");
    QRegularExpression rx("\\A(?:" + pattern + ")\\z" );
    QRegularExpressionMatch match = rx.match(suffix, 0, QRegularExpression::NormalMatch);
    if (match.hasMatch())
        return FileType::from(FileKind::Opt);

    return *mNone;
}

FileType& FileType::from(FileKind kind)
{
    for (FileType *ft: qAsConst(mFileTypes)) {
        if (ft->mKind == kind)
            return *ft;
    }
    return *mNone;
}

QStringList FileType::validateSuffixList(const QString &commaSeparatedList, QStringList *invalid)
{
    QStringList res = commaSeparatedList.split(QRegularExpression("\\h*,\\h*"), Qt::SkipEmptyParts);
    for (const QString &suf : CInvalidUserTypes) {
        if (res.contains(suf, Qt::CaseInsensitive)) {
            res.removeAll(suf);
            if (invalid) *invalid << suf;
        }
    }
    return res;
}

void FileType::setUserGamsTypes(const QStringList &suffix)
{
    mUserGamsTypes = suffix;
    QStringList allSuffix;
    int i = 0;
    for (i = 0; i < mFileTypes.length(); ++i) {
        if (mFileTypes.at(i)->mKind == FileKind::Gms) {
            FileType *ft = mFileTypes.at(i);
            allSuffix = ft->mSuffix;
            while (allSuffix.length() > mGmsFixedTypes) allSuffix.removeLast();
            allSuffix.append(suffix);
            mFileTypes.replace(i, new FileType(ft->mKind, allSuffix, ft->mDescription, ft->mAutoReload));
            break;
        }
    }
}

const QStringList FileType::userGamsTypes()
{
    return mUserGamsTypes;
}

const QStringList FileType::invalidUserGamsTypes()
{
    return CInvalidUserTypes;
}

void FileType::clear()
{
    while (!mFileTypes.isEmpty()) {
        FileType* ft = mFileTypes.takeLast();
        delete ft;
    }
    delete mNone;
    mNone = nullptr;
}


} // namespace studio
} // namespace gams
