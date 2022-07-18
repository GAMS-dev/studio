#include "filesystemmodel.h"
#include "file/filetype.h"
#include "logger.h"

namespace gams {
namespace studio {
namespace fs {

bool FilteredFileSystemModel::isDir(const QModelIndex &index) const
{
    return static_cast<FileSystemModel*>(sourceModel())->isDir(mapToSource(index));
}

bool FilteredFileSystemModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)
    return source_column == 0;
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
        if (isDir(idx))
            return dirCheckState(path);
        else
            return mCheckedFiles.contains(rootDirectory().relativeFilePath(path)) ? Qt::Checked : Qt::Unchecked;
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
        QString file = rootDirectory().relativeFilePath(filePath(idx));
        if (isDir(idx) && mDirs.value(file).childCount > 0)
            setChildSelection(idx, value.toInt() == Qt::Unchecked);
        else if (value.toInt() == Qt::Unchecked)
            mCheckedFiles.remove(file);
        else
            mCheckedFiles.insert(file);
        emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
        invalidateDirState(idx.parent());
        return true;
    } else if (role == WriteBackRole && idx.column() == 0) {
        auto file = rootDirectory().relativeFilePath(filePath(idx));
        mWriteBack.insert(file, value.toBool());
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
    bool empty = true;
    QModelIndex first;
    QModelIndex idx;
    for (const QFileInfo &info : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        idx = index(info.filePath());
        if (!first.isValid()) first = idx;
        empty = false;
        if (info.isDir()) {
            selectAllFiles(QDir(info.filePath()));
        } else
            mCheckedFiles << rootDirectory().relativeFilePath(info.filePath());
    }
    if (empty) {
        mCheckedFiles << rootDirectory().relativeFilePath(dir.path());
        idx = index(dir.path());
        emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
    } else {
        emit dataChanged(first, idx, QVector<int>() << Qt::CheckStateRole);
    }
    mUpdateTimer.start();
}

void FileSystemModel::clearSelection()
{
    QSet<QModelIndex> indices;
    for (const QString &file : mCheckedFiles) {
        QModelIndex mi = index(rootDirectory().absoluteFilePath(file));
        while (mi.isValid()) {
            indices << mi;
            mi = mi.parent();
        }
    }
    mCheckedFiles.clear();
    invalidateDirStates();
    for (const QModelIndex &idx : indices)
        emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
}

QStringList FileSystemModel::selectedFiles()
{
    QStringList selection;
    for (auto file: mCheckedFiles) {
        selection << file;
    }
    selection.sort();
    return selection;
}

void FileSystemModel::setSelectedFiles(const QStringList &files)
{
    mCheckedFiles.clear();
    for (const QString &file : files)
        mCheckedFiles << file;
    invalidateDirStates();
}

bool FileSystemModel::hasSelection()
{
    return !mCheckedFiles.isEmpty();
}

void FileSystemModel::newDirectoryData(const QString &path)
{
    QString relPath = rootDirectory().relativeFilePath(path);
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
        mDirs[subPath(path)].checkState = dirCheckState(rootDirectory().absoluteFilePath(path), false);
    }
}

int FileSystemModel::dirCheckState(const QString &path, bool isConst) const
{
    QDir dir(path);
    int flag = 0;
    QList<QFileInfo> fiList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    if (fiList.isEmpty())
        return mCheckedFiles.contains(rootDirectory().relativeFilePath(path)) ? Qt::Checked : Qt::Unchecked;

    for (const QFileInfo &info : fiList) {
        QString relPath = rootDirectory().relativeFilePath(info.filePath());
        if (info.isDir()) {
            if (!mDirs.contains(relPath) && !isConst) {
                updateDirInfo(index(info.path()));
            }
            int state = mDirs.value(relPath).checkState;
            if (state < 0) {
                state = dirCheckState(info.filePath(), false);
            }
            if (state == Qt::PartiallyChecked) return Qt::PartiallyChecked;
            flag |= (state == Qt::Checked) ? 1 : 2;
        } else {
            flag |= (mCheckedFiles.contains(relPath)) ? 1 : 2;
        }
        if (flag == 3) return Qt::PartiallyChecked;
    }
    return flag == 1 ? Qt::Checked : Qt::Unchecked;
}

void FileSystemModel::updateDirInfo(const QModelIndex &idx) const
{
    if (!isDir(idx))
        return;
    auto path = subPath(idx);
    if (!mDirs.contains(path)) {
        QDir dir(filePath(idx));
        mDirs[path].childCount = int(dir.count()) - 2;
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
    QList<QFileInfo> fiList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    if (fiList.isEmpty()) {
        if (remove) mCheckedFiles.remove(rootDirectory().relativeFilePath(dir.path()));
        else mCheckedFiles.insert(rootDirectory().relativeFilePath(dir.path()));
        return;
    }

    QModelIndex startIdx = QModelIndex();
    QModelIndex subIdx = QModelIndex();
    for (const QFileInfo &info : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        subIdx = index(info.filePath());
        if (!startIdx.isValid()) startIdx = subIdx;
        QString relPath = rootDirectory().relativeFilePath(info.filePath());
        if (info.isDir()) {
            updateDirInfo(subIdx);
            mDirs[relPath].checkState = remove ? Qt::Unchecked : Qt::Checked;
            setChildSelection(subIdx, remove);
        } else if (remove)
            mCheckedFiles.remove(relPath);
        else
            mCheckedFiles.insert(relPath);
    }
    updateDirInfo(idx);
    mDirs[rootDirectory().relativeFilePath(dir.path())].checkState = remove ? Qt::Unchecked : Qt::Checked;

    if (startIdx.isValid())
        emit dataChanged(startIdx, subIdx, QVector<int>() << Qt::CheckStateRole);
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
