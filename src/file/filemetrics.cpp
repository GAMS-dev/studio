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
#include "filemetrics.h"

namespace gams {
namespace studio {

FileMetrics::FileMetrics()
    : mExists(false), mSize(0), mType(&FileType::from(FileKind::None))
{ }

FileMetrics::FileMetrics(const FileMetrics& other)
    : mExists(other.mExists), mSize(other.mSize), mCreated(other.mCreated)
    , mModified(other.mModified), mType(other.mType)
{ }

FileMetrics& FileMetrics::operator=(const FileMetrics& other)
{
    mExists = other.mExists;
    mSize = other.mSize;
    mCreated = other.mCreated;
    mModified = other.mModified;
    mType = other.mType;
    return *this;
}

const FileType& FileMetrics::fileType() const
{
    return *mType;
}

FileMetrics::FileMetrics(QFileInfo fileInfo, const FileType* knownType)
    : mType(knownType ? knownType : &FileType::from(fileInfo.suffix()))
{
    mExists = fileInfo.exists();
    mSize = mExists ? fileInfo.size() : 0;
    mCreated = mExists ? fileInfo.birthTime() : QDateTime();
    mModified = mExists ? fileInfo.lastModified() : QDateTime();
}

FileMetrics::ChangeKind FileMetrics::check(QFileInfo fileInfo)
{
    if (mModified.isNull()) return ckSkip;
    if (!fileInfo.exists()) {
        // TODO(JM) #106: find a file in the path fitting created, modified and size values
        return ckNotFound;
    }
    if (fileInfo.lastModified() != mModified) return ckModified;
    return ckUnchanged;
}


} // namespace studio
} // namespace gams
