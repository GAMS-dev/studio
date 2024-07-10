/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "file/filetype.h"
#include "logger.h"

namespace gams {
namespace studio {
namespace fs {

FilteredFileSystemModel::FilteredFileSystemModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{ }

bool FilteredFileSystemModel::isDir(const QModelIndex &index) const
{
    return static_cast<FileSystemModel*>(sourceModel())->isDir(mapToSource(index));
}

void FilteredFileSystemModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    FileSystemModel *fsModel = qobject_cast<FileSystemModel*>(sourceModel);
    if (fsModel)
        connect(fsModel, &FileSystemModel::isFiltered, this, [this](QModelIndex source_index, bool &filtered) {
            filtered = !filterAcceptsRow(source_index.row(), source_index.parent());
        });
}

bool FilteredFileSystemModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)
    return source_column == 0;
}

bool FilteredFileSystemModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (idx.isValid()) {
        FileSystemModel* srcModel = static_cast<FileSystemModel*>(sourceModel());
        QString path = srcModel->data(idx, QFileSystemModel::FilePathRole).toString();
        if (srcModel->isDir(idx)) {
            QString text = sourceModel()->data(idx).toString();
            if (!path.startsWith(srcModel->rootPath())) return true;
            if (mHideUncommon && text.startsWith("225")) return false;
            if (!filterRegularExpression().pattern().isEmpty()) {
                QDir dir(srcModel->filePath(idx));
                const auto infos = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
                for (const QFileInfo &info : infos) {
                    QModelIndex child = srcModel->index(info.filePath());
                    if (filterAcceptsRow(child.row(), idx)) return true;
                }
                return false;
            }
        } else {
            QString text = sourceModel()->data(idx).toString();
            if (mHideUncommon && mUncommonRegEx.isValid()) {
                if (mUncommonRegEx.match(text).hasMatch()) {
                    return false;
                }
            }
            return text.contains(filterRegularExpression());
        }
        return true;
    }
    return false;
}

FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    mUpdateTimer.setInterval(50);
    mUpdateTimer.setSingleShot(true);
    connect(&mUpdateTimer, &QTimer::timeout, this, &FileSystemModel::updateDirCheckStates);
    connect(this, &QFileSystemModel::directoryLoaded, this, &FileSystemModel::newDirectoryData);
}

QVariant FileSystemModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    if (role == Qt::CheckStateRole) {
        QString path = filePath(idx);
        if (isDir(idx)) {
            if (!path.startsWith(rootPath())) return QVariant();
            return dirCheckState(path, false, true);
        }
        else
            return mSelectedFiles.contains(rootDirectory().relativeFilePath(path)) ? Qt::Checked : Qt::Unchecked;
    } else if (role == WriteBackRole) {
        auto path = rootDirectory().relativeFilePath(filePath(idx));
        if (mWriteBack.contains(path))
            return mWriteBack.value(path);
    }
    return QFileSystemModel::data(idx, role);
}

bool FileSystemModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && idx.column() == 0) {
        updateDirInfo(idx);
        QString absFile = filePath(idx);
        QString file = rootDirectory().relativeFilePath(absFile);
        if (isDir(idx) && mDirs.value(file).childCount > 0) {
            setChildSelection(idx, dirCheckState(absFile, true) == Qt::Checked);
        }
        else if (value.toInt() == Qt::Unchecked)
            mSelectedFiles.remove(file);
        else
            mSelectedFiles.insert(file);
        emit selectionCountChanged(mSelectedFiles.count());
        emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
        invalidateDirState(idx.parent());
        return true;
    } else if (role == WriteBackRole && idx.column() == 0) {
        auto file = rootDirectory().relativeFilePath(filePath(idx));
        mWriteBack.insert(file, value.toBool());
        emit selectionCountChanged(mSelectedFiles.count());
        emit dataChanged(idx, idx, QVector<int>() << WriteBackRole);
        return true;
    }
    return  QFileSystemModel::setData(idx, value, role);
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

void FileSystemModel::selectAll()
{
    selectAllFiles(rootDirectory());
}

void FileSystemModel::selectAllFiles(const QDir &dir)
{
    invalidateDirState(index(dir.path()));
    QSet<QModelIndex> indices;
    QModelIndex idx;
    for (const QFileInfo &info : visibleFileInfoList(dir)) {
        idx = index(info.filePath());
        indices << idx;
        if (info.isDir()) {
            selectAllFiles(QDir(info.filePath()));
        } else {
            mSelectedFiles << rootDirectory().relativeFilePath(info.filePath());
            emit selectionCountChanged(mSelectedFiles.count());
        }
    }
    if (!idx.isValid()) { // empty directory
        mSelectedFiles << rootDirectory().relativeFilePath(dir.path());
        idx = index(dir.path());
        indices << idx;
    }
    if (idx.isValid()) {
        for (const QModelIndex &idx: std::as_const(indices)) {
            emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
        }
        emit selectionCountChanged(mSelectedFiles.count());
        invalidateDirState(idx);
    }
    mUpdateTimer.start();
}

const QList<QFileInfo> FileSystemModel::visibleFileInfoList(const QDir &dir) const
{
    QList<QFileInfo> res;
    const auto list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : list) {
        QModelIndex idx = index(info.filePath());
        bool filtered;
        emit isFiltered(idx, filtered);
        if (!filtered) res << info;
    }
    return res;
}

void FileSystemModel::clearSelection()
{
    QSet<QModelIndex> indices;
    QSet<QString> remove;
    for (const QString &file : std::as_const(mSelectedFiles)) {
        QModelIndex idx = index(rootDirectory().absoluteFilePath(file));
        bool filtered;
        emit isFiltered(idx, filtered);
        if (!filtered) {
            remove << file;
            while (idx.isValid()) {
                indices << idx;
                idx = idx.parent();
            }
        }
    }
    for (const QString &file: std::as_const(remove))
        mSelectedFiles.remove(file);
    invalidateDirStates();
    for (const QModelIndex &idx : std::as_const(indices))
        emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
    emit selectionCountChanged(mSelectedFiles.count());
}

QStringList FileSystemModel::selectedFiles(bool addWriteBackState)
{
    QStringList selection;
    for (const auto &file: std::as_const(mSelectedFiles))
        selection << file;
    if (addWriteBackState) {
        for (int i = 0; i < selection.count(); ++i) {
            if (mWriteBack.value(selection.at(i)))
                selection[i] = selection.at(i) + " <";
        }
    }
    selection.sort();
    return selection;
}

void FileSystemModel::setSelectedFiles(const QStringList &files)
{
    mSelectedFiles.clear();
    mWriteBack.clear();
    for (const QString &file : files) {
        if (file.endsWith(" <")) {
            mSelectedFiles << file.left(file.length()-2);
            mWriteBack.insert(file.left(file.length()-2), true);
        } else
            mSelectedFiles << file;
    }
    QStringList missFiles;
    for (const QString &file : std::as_const(mSelectedFiles)) {
        if (!QFileInfo::exists(rootDirectory().absoluteFilePath(file)))
            missFiles << file;
    }
    for (const QString &file: std::as_const(missFiles)) {
        mSelectedFiles.remove(file);
    }
    if (missFiles.count()) emit missingFiles(missFiles);
    invalidateDirStates();
    emit selectionCountChanged(mSelectedFiles.count());
}

bool FileSystemModel::hasSelection()
{
    return !mSelectedFiles.isEmpty();
}

int FileSystemModel::selectionCount()
{
    return mSelectedFiles.count();
}

void FileSystemModel::newDirectoryData(const QString &path)
{
    updateDirInfo(index(path));
}

void FileSystemModel::updateDirCheckStates()
{
    QStringList dirs;
    QMap<QString,DirState>::ConstIterator it = mDirs.constBegin();
    while (it != mDirs.constEnd()) {
        if (it.value().checkState < 0) dirs << it.key();
        ++it;
    }
    dirs.sort();
    while (!dirs.isEmpty()) {
        QString path = dirs.takeLast();
        mDirs[subPath(path)].checkState = dirCheckState(rootDirectory().absoluteFilePath(path), false, false);
    }
}

int FileSystemModel::dirCheckState(const QString &path, bool filtered, bool isConst) const
{
    if (path.startsWith("..")) return Qt::Unchecked;
    QDir dir(path);
    int flag = 0;
    const QList<QFileInfo> fiList = filtered ? visibleFileInfoList(dir)
                                             : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    if (fiList.isEmpty())
        return mSelectedFiles.contains(rootDirectory().relativeFilePath(path)) ? Qt::Checked : Qt::Unchecked;

    for (const QFileInfo &info : fiList) {
        QString relPath = rootDirectory().relativeFilePath(info.filePath());
        if (info.isDir()) {
            if (!mDirs.contains(relPath) && !isConst) {
                updateDirInfo(index(info.path()));
            }

            // TODO(JM) better check if the mDirs entry can be invalidated correctly, and reactivate the following
//            int state = mDirs.value(relPath).checkState;
//            if (state < 0) {
//                state = dirCheckState(info.filePath(), filtered, isConst);
//            }
            int state = dirCheckState(info.filePath(), filtered, isConst);

            if (state == Qt::PartiallyChecked) return Qt::PartiallyChecked;
            flag |= (state == Qt::Checked) ? 1 : 2;
        } else {
            flag |= (mSelectedFiles.contains(relPath)) ? 1 : 2;
        }
        if (flag == 3) return Qt::PartiallyChecked;
    }
    return flag == 1 ? Qt::Checked : Qt::Unchecked;
}

void FileSystemModel::updateDirInfo(const QModelIndex &idx) const
{
    if (!isDir(idx))
        return;
    QString path = subPath(idx);
    if (!mDirs.contains(path) || mDirs.value(path).childCount < 0) {
        QDir dir(filePath(idx));
        mDirs[path].childCount = int(dir.count()) - 2;
//        mDirs[path].entries.clear();
//        for (const QFileInfo &info: dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
//            Entry *e = new Entry();
//            e->isDir = info.isDir();
//            e->name = info.fileName();
//            e->absoluteFilePath = info.absoluteFilePath();
//            e->relativeFilePath = rootDirectory().relativeFilePath(e->absoluteFilePath);
//            mDirs[path].entries.append(e);
//        }
    }
}

void FileSystemModel::invalidateDirState(const QModelIndex &par)
{
    if (!par.isValid()) return;
    mDirs[subPath(par)].checkState = -1;
    if (par.parent().isValid())
        invalidateDirState(par.parent());
    emit dataChanged(par, par, QVector<int>() << Qt::CheckStateRole);
    mUpdateTimer.start();
}

void FileSystemModel::invalidateDirStates()
{
    QMap<QString,DirState>::Iterator it = mDirs.begin();
    while (it != mDirs.end()) {
        it.value().checkState = -1;
        ++it;
    }
    mUpdateTimer.start();
}

void FileSystemModel::setChildSelection(const QModelIndex &idx, bool remove)
{
    QDir dir(filePath(idx));
    const QList<QFileInfo> fiList = visibleFileInfoList(dir);
    if (fiList.isEmpty()) {
        if (remove) mSelectedFiles.remove(rootDirectory().relativeFilePath(dir.path()));
        else mSelectedFiles.insert(rootDirectory().relativeFilePath(dir.path()));
        return;
    }
    QModelIndex subIdx = QModelIndex();
    for (const QFileInfo &info : visibleFileInfoList(dir)) {
        subIdx = index(info.filePath());
        QString relPath = rootDirectory().relativeFilePath(info.filePath());
        if (info.isDir()) {
            updateDirInfo(subIdx);
            mDirs[relPath].checkState = remove ? Qt::Unchecked : Qt::Checked;
            setChildSelection(subIdx, remove);
        } else if (remove)
            mSelectedFiles.remove(relPath);
        else
            mSelectedFiles.insert(relPath);
        emit dataChanged(subIdx, subIdx, QVector<int>() << Qt::CheckStateRole);
    }
    updateDirInfo(idx);
    mDirs[rootDirectory().relativeFilePath(dir.path())].checkState = remove ? Qt::Unchecked : Qt::Checked;
}

QString FileSystemModel::subPath(const QModelIndex &idx) const
{
    return filePath(idx).remove(0, rootDirectory().canonicalPath().size()+1);
}

QString FileSystemModel::subPath(const QString &path) const
{
    return QString(path).remove(0, rootDirectory().canonicalPath().size()+1);
}

} // namespace fs
} // namespace studio
} // namespace gams
