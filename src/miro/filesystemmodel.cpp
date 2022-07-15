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
#include "filesystemmodel.h"

namespace gams {
namespace studio {
namespace miro {

FilteredFileSystemModel::FilteredFileSystemModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{}

bool FilteredFileSystemModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)
    return source_column == 0;
}

FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    connect(this, &QFileSystemModel::directoryLoaded,
            this, &FileSystemModel::newDirectoryData);
}

QVariant FileSystemModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    if (role == Qt::CheckStateRole) {
        QString path = rootDirectory().relativeFilePath(filePath(idx));
        if (isDir(idx))
            return subdirectoryCheckState(path);
        else
            return mCheckedFiles.contains(path) ? Qt::Checked : Qt::Unchecked;
    }
    return QFileSystemModel::data(idx, role);
}

bool FileSystemModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && idx.column() == 0) {
        updateDirInfo(idx);
        QString file = rootDirectory().relativeFilePath(filePath(idx));
        if (value.toInt() != Qt::Unchecked) {
            if (isDir(idx))
                updateChildSelection(idx);
            mCheckedFiles.insert(file);
            emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
            addParentSelection(idx.parent());
        } else {
            if (isDir(idx))
                updateChildSelection(idx, true);
            mCheckedFiles.erase(mCheckedFiles.find(file));
            emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
            removeParentSelection(idx.parent());
        }
        return true;
    }
    return  QFileSystemModel::setData(idx, value, role);
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

void FileSystemModel::parseFolders()
{
    QDirIterator iter(rootDirectory().path(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        QString sub = iter.next();
        QModelIndex idx = index(sub);
        if (idx.isValid())
            fetchMore(idx);
    }
}

void FileSystemModel::selectAll()
{
    beginResetModel();
    selectAllFiles(rootDirectory());
    endResetModel();
}

void FileSystemModel::selectAllFiles(const QDir &dir)
{
    for (const QFileInfo &info : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (info.isDir())
            selectAllFiles(QDir(info.filePath()));
        mCheckedFiles << rootDirectory().relativeFilePath(info.filePath());
    }
}

void FileSystemModel::clearSelection()
{
    beginResetModel();
    mCheckedFiles.clear();
    endResetModel();
}

QStringList FileSystemModel::selectedFiles()
{
    QStringList selection;
    for (auto file: mCheckedFiles) {
        QFileInfo fileInfo(rootDirectory(), file);
        if (fileInfo.isFile()) {
            selection << file;
        } else if (fileInfo.isDir()){
            QDir dir(rootDirectory().absoluteFilePath(file));
            if (dir.isEmpty())
                selection << file;
        }
    }
    return selection;
}

void FileSystemModel::setSelectedFiles(const QStringList &files)
{
    beginResetModel();
    for (const QString &file : files) {
        mCheckedFiles << file;
    }
    endResetModel();
}

void FileSystemModel::newDirectoryData(const QString &path)
{
    QString relPath = rootDirectory().relativeFilePath(path);
    updateDirInfo(index(path));
}

int FileSystemModel::checkedChildren(const QString &path) const
{
    int checked = 0;
    if (isDir(index(path))) {
        QDir dir(rootDirectory().absoluteFilePath(path));
        for (auto info: dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            auto child = subPath(info.absoluteFilePath());
            if (child == path)
                continue;
            if (!child.startsWith(path))
                continue;
            if (mCheckedFiles.contains(child))
                ++checked;
        }
    }
    return checked;
}

Qt::CheckState FileSystemModel::directroyCheckState(const QString &path) const
{
    if (!mDirChildren.contains(path))
        return Qt::Unchecked;
    int childCount = mDirChildren[path];
    if (childCount == 0) return mCheckedFiles.contains(path) ? Qt::Checked : Qt::Unchecked;
    int checked = checkedChildren(path);
    if (checked > 0 && checked < childCount)
        return Qt::PartiallyChecked;
    if (childCount > 0 && checked == childCount)
        return Qt::Checked;
    return Qt::Unchecked;
}

Qt::CheckState FileSystemModel::subdirectoryCheckState(const QString &path) const
{
    QDirIterator iter(rootDirectory().absoluteFilePath(path), QDir::Dirs | QDir::NoDotAndDotDot);
    while (iter.hasNext()) {
        QString nextPath = iter.next();
        if (directroyCheckState(subPath(nextPath)) == Qt::PartiallyChecked)
            return Qt::PartiallyChecked;
    }
    return directroyCheckState(path);
}

void FileSystemModel::updateDirInfo(const QModelIndex &idx)
{
    if (!isDir(idx))
        return;
    auto path = subPath(idx);
    if (!mDirChildren.contains(path)) {
        QDir dir(filePath(idx));
        mDirChildren[path] = int(dir.count()) - 2;
    }
}

void FileSystemModel::updateParentDirInfo(const QModelIndex &parent)
{
    if (!parent.isValid())
        return;
    while (canFetchMore(parent))
        fetchMore(parent);
    updateDirInfo(parent);
    updateParentDirInfo(parent.parent());
}

void FileSystemModel::updateChildSelection(const QModelIndex &idx, bool remove)
{
    while (canFetchMore(idx))
        fetchMore(idx);

    QModelIndex startIdx = QModelIndex();
    QModelIndex subIdx = QModelIndex();
    for (int r = 0; r < rowCount(idx); ++r) {
        subIdx = index(r, 0, idx);
        if (r == 0) startIdx = subIdx;
        updateDirInfo(subIdx);

        auto idxPath = rootDirectory().relativeFilePath(filePath(subIdx));
        if (remove)
            mCheckedFiles.remove(idxPath);
        else
            mCheckedFiles.insert(idxPath);

        if (isDir(subIdx))
            updateChildSelection(subIdx, remove);
    }
    emit dataChanged(startIdx, subIdx, QVector<int>() << Qt::CheckStateRole);
}

void FileSystemModel::addParentSelection(const QModelIndex &idx)
{
    if (!idx.isValid()) return;

    updateParentDirInfo(idx);
    QString path = rootDirectory().relativeFilePath(filePath(idx));
    if (path.isEmpty()) return;

    if (!isDir(idx))
        mCheckedFiles.insert(path);

    emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
    addParentSelection(idx.parent());
}

void FileSystemModel::removeParentSelection(const QModelIndex &idx)
{
    if (!idx.isValid())
        return;
    auto path = rootDirectory().relativeFilePath(filePath(idx));
    mCheckedFiles.remove(path);
    removeParentSelection(idx.parent());
    emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
}

QString FileSystemModel::subPath(const QModelIndex &idx) const
{
    return filePath(idx).remove(0, rootDirectory().canonicalPath().size()+1);
}

QString FileSystemModel::subPath(const QString &path) const
{
    return QString(path).remove(0, rootDirectory().canonicalPath().size()+1);
}

}
}
}
