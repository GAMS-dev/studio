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
#include "filegroupcontext.h"
#include "exception.h"

namespace gams {
namespace studio {

FileGroupContext::~FileGroupContext()
{
    setParentEntry(nullptr);
    mChildList.clear();
}

void FileGroupContext::setFlag(ContextFlag flag, bool value)
{
    if (flag == FileSystemContext::cfEditMod || flag == FileSystemContext::cfFileMod)
        throw QException();
    FileSystemContext::setFlag(flag, value);

    // distribute missing flag to child entries
    if (flag == FileSystemContext::cfMissing && flag) {
        for (FileSystemContext *fc: mChildList) {
            fc->setFlag(flag);
        }
    }
}

void FileGroupContext::unsetFlag(ContextFlag flag)
{
    if (flag == FileSystemContext::cfEditMod || flag == FileSystemContext::cfFileMod)
        throw QException();
    FileSystemContext::setFlag(flag, false);
}

int FileGroupContext::peekIndex(const QString& name, bool *hit)
{
    if (hit) *hit = false;
    for (int i = 0; i < childCount(); ++i) {
        FileSystemContext *child = childEntry(i);
        QString other = child->name();
        int comp = name.compare(other, Qt::CaseInsensitive);
        if (comp < 0) return i;
        if (comp == 0) {
            if (hit) *hit = true;
            return i;
        }
    }
    return childCount();
}

void FileGroupContext::insertChild(FileSystemContext* child)
{
    if (!child) return;
    bool hit;
    int pos = peekIndex(child->name(), &hit);
    if (!hit) {
        mChildList.insert(pos, child);
        if (child->testFlag(cfActive))
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

QIcon FileGroupContext::icon()
{
    return QIcon::fromTheme("folder", QIcon(":/img/folder-open"));
}

bool FileGroupContext::isWatched()
{
    return mFsWatcher;
}

QFileSystemWatcher*FileGroupContext::watchIt()
{
    if (!mFsWatcher) {
        mFsWatcher = new QFileSystemWatcher(QStringList()<<location(), this);
        mFsWatcher->addPath(location());
        qDebug() << "added watcher for" << location();
    }
    return mFsWatcher;
}

void FileGroupContext::directoryChanged(const QString& path)
{
    QDir dir(path);
    if (dir.exists()) {
        emit contentChanged(mId, dir);
        return;
    }
    if (testFlag(cfActive)) {
        setFlag(cfMissing);
        return;
    }
    deleteLater();
}

FileGroupContext::FileGroupContext(FileGroupContext* parent, int id, QString name, QString location)
    : FileSystemContext(parent, id, name, location)
{
    mFlags = FileSystemContext::cfGroup;
}

} // namespace studio
} // namespace gams
