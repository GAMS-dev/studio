/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "filesystemcontext.h"
#include "filegroupcontext.h"

namespace gams {
namespace studio {

FileSystemContext::FileSystemContext(FileGroupContext* parent, int id, QString name, QString location)
    : QObject(parent), mId(id), mParent(nullptr), mName(name), mLocation(location), mFlags(cfNone), mType(FileSystem)
{
    setParentEntry(parent);
}

FileSystemContext::FileSystemContext(FileGroupContext* parent, int id, QString name, QString location, ContextType type)
    : QObject(parent), mId(id), mParent(nullptr), mName(name), mLocation(location), mFlags(cfNone), mType(type)
{
    setParentEntry(parent);
}

void FileSystemContext::checkFlags()
{
}

FileSystemContext::~FileSystemContext()
{
    if (mParent) {
        mParent->removeChild(this);
    }
}

int FileSystemContext::id() const
{
    return mId;
}

int FileSystemContext::type() const
{
    return mType;
}

FileGroupContext* FileSystemContext::parentEntry() const
{
    return mParent;
}

void FileSystemContext::setParentEntry(FileGroupContext* parent)
{
    if (parent != mParent) {
        if (mParent) mParent->removeChild(this);
        mParent = parent;
        if (mParent) mParent->insertChild(this);
    }
}

FileSystemContext* FileSystemContext::childEntry(int index)
{
    Q_UNUSED(index);
    return nullptr;
}

int FileSystemContext::childCount()
{
    return 0;
}

const QString FileSystemContext::caption()
{
    return mName;
}

const QString FileSystemContext::name()
{
    return mName;
}

void FileSystemContext::setName(const QString& name)
{
    if (mName != name) {
        mName = name;
        emit changed(mId);
    }
}

const QString& FileSystemContext::location() const
{
    return mLocation;
}

void FileSystemContext::setLocation(const QString& location)
{
    if (!location.isEmpty()) {
        QFileInfo fi(location);
        mLocation = fi.canonicalFilePath();
        setName(fi.fileName());
    }
}

QIcon FileSystemContext::icon()
{
    return QIcon();
}

const FileSystemContext::ContextFlags& FileSystemContext::flags() const
{
    return mFlags;
}

void FileSystemContext::setFlag(ContextFlag flag, bool value)
{
    bool current = testFlag(flag);
    if (current == value) return;
    mFlags.setFlag(flag, value);
    if (mParent)
        mParent->checkFlags();
    emit changed(mId);
}

void FileSystemContext::unsetFlag(ContextFlag flag)
{
    setFlag(flag, false);
}

bool FileSystemContext::testFlag(FileSystemContext::ContextFlag flag)
{
    return mFlags.testFlag(flag);
}

FileSystemContext* FileSystemContext::findFile(QString filePath)
{
    if(location() == filePath)
        return this;
    else
        return nullptr;
}

} // namespace studio
} // namespace gams
