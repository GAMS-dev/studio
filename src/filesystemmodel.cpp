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
            if (!filterRegExp().isEmpty()) {
                QDir dir(srcModel->filePath(idx));
                for (QFileInfo info : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
                    QModelIndex child = srcModel->index(info.filePath());
                    if (filterAcceptsRow(child.row(), idx)) return true;
                }
                return false;
            }
        } else {
            QString text = sourceModel()->data(idx).toString();
            if (mHideUncommon && mUncommonRegEx.isValid()) {
                if (mUncommonRegEx.exactMatch(text)) {
                    return false;
                }
            }
            return text.contains(filterRegExp());
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
        if (isDir(idx))
            return dirCheckState(path);
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
        QString file = rootDirectory().relativeFilePath(filePath(idx));
        if (isDir(idx) && mDirs.value(file).childCount > 0)
            setChildSelection(idx, value.toInt() == Qt::Unchecked);
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
    bool empty = true;
    QModelIndex first;
    QModelIndex idx;
    for (const QFileInfo &info : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        idx = index(info.filePath());
        if (!first.isValid()) first = idx;
        empty = false;
        if (info.isDir()) {
            selectAllFiles(QDir(info.filePath()));
        } else {
            mSelectedFiles << rootDirectory().relativeFilePath(info.filePath());
            emit selectionCountChanged(mSelectedFiles.count());
        }
    }
    if (empty) {
        mSelectedFiles << rootDirectory().relativeFilePath(dir.path());
        idx = index(dir.path());
        emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
        emit selectionCountChanged(mSelectedFiles.count());
    } else {
        emit dataChanged(first, idx, QVector<int>() << Qt::CheckStateRole);
    }
    mUpdateTimer.start();
}

void FileSystemModel::clearSelection()
{
    QSet<QModelIndex> indices;
    for (const QString &file : mSelectedFiles) {
        QModelIndex mi = index(rootDirectory().absoluteFilePath(file));
        while (mi.isValid()) {
            indices << mi;
            mi = mi.parent();
        }
    }
    mSelectedFiles.clear();
    invalidateDirStates();
    for (const QModelIndex &idx : indices)
        emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
    emit selectionCountChanged(mSelectedFiles.count());
}

QStringList FileSystemModel::selectedFiles(bool addWriteBackState)
{
    QStringList selection;
    for (auto file: mSelectedFiles)
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
    for (const QString &file : mSelectedFiles) {
        if (!QFileInfo::exists(rootDirectory().absoluteFilePath(file)))
            missFiles << file;
    }
    for (const QString &file: missFiles) {
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
    if (path.startsWith("..")) return Qt::Unchecked;
    QDir dir(path);
    int flag = 0;
    QList<QFileInfo> fiList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    if (fiList.isEmpty())
        return mSelectedFiles.contains(rootDirectory().relativeFilePath(path)) ? Qt::Checked : Qt::Unchecked;

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
        if (remove) mSelectedFiles.remove(rootDirectory().relativeFilePath(dir.path()));
        else mSelectedFiles.insert(rootDirectory().relativeFilePath(dir.path()));
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
            mSelectedFiles.remove(relPath);
        else
            mSelectedFiles.insert(relPath);
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
