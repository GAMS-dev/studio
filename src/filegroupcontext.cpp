/*
 * This file is part of the GAMS IDE project.
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
#include "filegroupcontext.h"
#include "exception.h"

namespace gams {
namespace ide {

FileGroupContext::~FileGroupContext()
{
    // TODO(JM)  delete the entry in the parents childList
    mChildList.clear();
}

void FileGroupContext::setFlag(ContextFlag flag, bool value)
{
    if (flag == FileSystemContext::cfEditMod || flag == FileSystemContext::cfFileMod)
        throw QException();
    FileSystemContext::setFlag(flag, value);
}

void FileGroupContext::unsetFlag(ContextFlag flag)
{
    if (flag == FileSystemContext::cfEditMod || flag == FileSystemContext::cfFileMod)
        throw QException();
    FileSystemContext::unsetFlag(flag);
}

int FileGroupContext::peekIndex(const QString& name, bool *exactMatch)
{
    if (exactMatch)
        *exactMatch = false;
    for (int i = 0; i < childCount(); ++i) {
        FileSystemContext *child = childEntry(i);
        int comp = name.compare(child->name(), Qt::CaseInsensitive);
        if (comp >= 0) {
            if (comp == 0 && exactMatch)
                *exactMatch = true;
            return i;
        }
    }
    return childCount();
}

void FileGroupContext::insertChild(FileSystemContext* child)
{
    if (!child) return;
    int pos = 0;
    QString name = child->name();
    for (int i = 0; i < childCount(); ++i) {
        int comp = name.compare(childEntry(i)->name(), Qt::CaseInsensitive);
        if (comp > 0) {
            pos = i;
            break;
        }
    }
    insertChild(pos, child);
}

void FileGroupContext::insertChild(int pos, FileSystemContext* child)
{
    if (child == this)
        throw FATAL() << "can't add a element to itself";
    mChildList.insert(pos, child);
    if (child->flags() & cfActive) {
        setFlag(cfActive);
    }
}

void FileGroupContext::removeChild(FileSystemContext* child)
{
    mChildList.removeOne(child);
}

void FileGroupContext::checkFlags()
{
    bool active = false;
    for (FileSystemContext *fc: mChildList) {
        if (fc->testFlag(cfActive)) {
            active = true;
            break;
        }
    }
    setFlag(cfActive, active);
}

int FileGroupContext::childCount()
{
    return mChildList.count();
}

int FileGroupContext::indexOf(FileSystemContext* child)
{
    return mChildList.indexOf(child);
}

FileSystemContext*FileGroupContext::childEntry(int index)
{
    return mChildList.at(index);
}

void FileGroupContext::directoryChanged(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) {

    } else {
        emit contentChanged(mId, dir);
    }
}

FileGroupContext::FileGroupContext(FileGroupContext* parent, int id, QString name, QString location, bool isGist)
    : FileSystemContext(parent, id, name, location, isGist)
{
    mFlags = FileSystemContext::cfGroup;
}

} // namespace ide
} // namespace gams
