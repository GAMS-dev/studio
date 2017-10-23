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
    while (mChildList.size()) {
        FileSystemContext* fsc = mChildList.takeFirst();
        delete fsc;
    }
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

FileSystemContext* FileGroupContext::findFile(QString filePath)
{
    for (int i = 0; i < childCount(); i++) {
        FileSystemContext *child = childEntry(i);
        if (child->location() == filePath)
            return child;
        if (child->type() == FileSystemContext::FileGroup) {
            FileGroupContext *group = static_cast<FileGroupContext*>(child);
            return group->findFile(filePath);
        }
//        if (child->childCount() > 0) {
//            for (int j = 0; j < child->childCount(); j++) {
//                FileSystemContext *element = child->childEntry(j)->findFile(filePath);
//                if (element != nullptr) {
//                    return element;
//                }
//            }
//        }
    }
    return nullptr;
}

void FileGroupContext::setLocation(const QString& location)
{
    Q_UNUSED(location);
    EXCEPT() << "The location of a FileGroupContext can't be changed.";
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
    if (hit) pos++;
    mChildList.insert(pos, child);
    if (child->testFlag(cfActive))
        setFlag(cfActive);
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

QString FileGroupContext::runableGms()
{
    // TODO(JM) for projects the project file has to be parsed for the main runableGms
    qDebug() << "runableGms:";
    qDebug() << QDir(mLocation).filePath(mRunInfo);
    return QDir(mLocation).filePath(mRunInfo);
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
    return mDirWatcher;
}

void FileGroupContext::setWatched(bool watch)
{
    if (!watch) {
        if (mDirWatcher) {
            mDirWatcher->deleteLater();
            mDirWatcher = nullptr;
        }
        return;
    }
    if (!mDirWatcher) {
        mDirWatcher = new QFileSystemWatcher(QStringList()<<location(), this);
        connect(mDirWatcher, &QFileSystemWatcher::directoryChanged, this, &FileGroupContext::directoryChanged);
    }
    mDirWatcher->addPath(location());
    qDebug() << "added watcher for" << location();
}

void FileGroupContext::directoryChanged(const QString& path)
{
    qDebug() << "Dir changed";
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

FileGroupContext::FileGroupContext(int id, QString name, QString location, QString runInfo)
    : FileSystemContext(id, name, location, FileSystemContext::FileGroup)
{
    mRunInfo = runInfo;
}

} // namespace studio
} // namespace gams
