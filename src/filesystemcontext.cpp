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
#include "filesystemcontext.h"

namespace gams {
namespace ide {

FileSystemContext::FileSystemContext(FileSystemContext* parent, int id, QString name, QString location, bool isGist)
    : QObject(parent), mId(id), mName(name), mPath(location), mIsGist(isGist)
{
    if (parent) {
        int sort = parent->peekIndex(name, true);
        QObjectList *childrn = const_cast<QObjectList*>(&parent->children());
        if (childrn->count() > 1)
            childrn->move(childrn->count()-1, sort);
    }
}

bool FileSystemContext::active() const
{
    return mActive;
}

FileSystemContext::~FileSystemContext()
{
    // QObject should delete the entry in the parents children for us
}

int FileSystemContext::id() const
{
    return mId;
}

bool FileSystemContext::matches(const QString &name, bool isGist) const
{
    return isGist == mIsGist && mName.compare(name, Qt::CaseInsensitive) == 0;
}

FileSystemContext* FileSystemContext::child(int index) const
{
    if (index < 0 || index >= children().count())
        return nullptr;
    return qobject_cast<FileSystemContext*>(children().at(index));
}

FileSystemContext* FileSystemContext::parentEntry() const
{
    return qobject_cast<FileSystemContext*>(parent());
}

int FileSystemContext::peekIndex(QString name, bool skipLast)
{
    int res = 0;
    // TODO(JM) need to skip the new appended entry
    int size = skipLast ? children().count()-1 : children().count();
    for (int i = 0; i < size; ++i) {
        if (child(i)->name().compare(name, Qt::CaseInsensitive) <= 0)
            res = i+1;
    }
    return res;
}

bool FileSystemContext::isGist() const
{
    return mIsGist;
}

const QString FileSystemContext::name()
{
    return mName;
}

void FileSystemContext::setName(const QString& name)
{
    mName = name;
}

const QString& FileSystemContext::location() const
{
    return mPath;
}


} // namespace ide
} // namespace gams
