/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "miromodelassemblydialog.h"
#include "ui_miromodelassemblydialog.h"

#include <QDir>
#include <QMessageBox>

namespace gams {
namespace studio {
namespace miro {

FilteredFileSystemModel::FilteredFileSystemModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

bool FilteredFileSystemModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)
    return source_column == 0;
}

FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    connect(this, &QFileSystemModel::directoryLoaded, this, &FileSystemModel::newDirectoryData);
}

QVariant FileSystemModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    if (role == Qt::CheckStateRole) {
        auto path = rootDirectory().relativeFilePath(filePath(idx));
        if (isDir(idx)) {
            int state = subdirectoryCheckState(path);
            if (state == 1)
                return Qt::PartiallyChecked;
            if (state == 2)
                return Qt::Checked;
            return Qt::Unchecked;
        } else if (mCheckedFiles.contains(path)) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }
    return QFileSystemModel::data(idx, role);
}

bool FileSystemModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && idx.column() == 0) {
        updateChildDirInfo(idx);
        auto file = rootDirectory().relativeFilePath(filePath(idx));
        if (value.toBool()) {
            mCheckedFiles.insert(file);
            emit dataChanged(idx, idx);
            if (isDir(idx))
                updateChildSelection(idx);
            addParentSelection(idx.parent());
            return true;
        } else {
            mCheckedFiles.erase(mCheckedFiles.find(file));
            emit dataChanged(idx, idx);
            if (isDir(idx))
                updateChildSelection(idx, true);
            removeParentSelection(idx.parent());
            return true;
        }
    }
    return  QFileSystemModel::setData(idx, value, role);
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

void FileSystemModel::selectAll()
{
    auto entries = rootDirectory().entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto entry: entries) {
        auto idx = index(rootDirectory().absoluteFilePath(entry));
        setData(idx, true, Qt::CheckStateRole);
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
    for (auto file: files) {
        auto idx = index(rootDirectory().absoluteFilePath(file));
        if (idx.isValid())
            setData(idx, true, Qt::CheckStateRole);
    }
}

void FileSystemModel::newDirectoryData(const QString &path)
{
    auto checked = rootDirectory().relativeFilePath(path);
    if (mCheckedFiles.contains(checked))
        setData(index(path), true, Qt::CheckStateRole);
}

int FileSystemModel::checkedChilds(const QString &path) const
{
    int checkedChilds = 0;
    if (isDir(index(path))) {
        QDir dir(rootDirectory().absoluteFilePath(path));
        for (auto info: dir.entryInfoList()) {
            auto child = subPath(info.absoluteFilePath());
            if (child == path)
                continue;
            if (!child.startsWith(path))
                continue;
            if (mCheckedFiles.contains(child))
                ++checkedChilds;
        }
    }
    return checkedChilds;
}

int FileSystemModel::directroyCheckState(const QString &path) const
{
    if (!mDirChilds.contains(path))
        return 0;

    int childs = mDirChilds[path];
    int checked = checkedChilds(path);
    if (checked>0 && checked<childs)
        return 1; // partial
    if (checked == childs && mCheckedFiles.contains(path))
        return 2; // full
    return 0; // unchecked
}

int FileSystemModel::subdirectoryCheckState(const QString &path) const
{
    QDirIterator iter(rootDirectory().absoluteFilePath(path),
                      QDir::Dirs | QDir::NoDotAndDotDot);
    while (iter.hasNext()) {
        auto next = iter.next();
        if (directroyCheckState(subPath(next)) == 1)
            return 1; // partial
    }
    return directroyCheckState(path);
}

void FileSystemModel::updateChildDirInfo(const QModelIndex &idx)
{
    if (!isDir(idx))
        return;

    while (canFetchMore(idx))
        fetchMore(idx);
    auto path = subPath(idx);
    mDirChilds[path] = rowCount(idx);
}

void FileSystemModel::updateParentDirInfo(const QModelIndex &parent)
{
    if (!parent.isValid())
        return;

    while (canFetchMore(parent))
        fetchMore(parent);
    auto path = subPath(parent);
    mDirChilds[path] = rowCount(parent);

    updateParentDirInfo(parent.parent());
}

void FileSystemModel::updateChildSelection(const QModelIndex &idx, bool remove)
{
    while (canFetchMore(idx))
        fetchMore(idx);

    for (int r=0; r<rowCount(idx); ++r) {
        auto subIdx = index(r, 0, idx);
        updateChildDirInfo(subIdx);

        auto idxPath = rootDirectory().relativeFilePath(filePath(subIdx));
        if (remove)
            mCheckedFiles.remove(idxPath);
        else
            mCheckedFiles.insert(idxPath);

        if (isDir(subIdx))
            updateChildSelection(subIdx, remove);

        emit dataChanged(subIdx, subIdx);
    }
}

void FileSystemModel::addParentSelection(const QModelIndex &idx)
{
    if (!idx.isValid())
        return;

    updateParentDirInfo(idx);

    auto path = subPath(idx);
    if (path.isEmpty())
        return;

    mCheckedFiles.insert(path);
    emit dataChanged(idx, idx);

    addParentSelection(idx.parent());
}

void FileSystemModel::removeParentSelection(const QModelIndex &idx)
{
    if (!idx.isValid())
        return;

    auto path = rootDirectory().relativeFilePath(filePath(idx));
    mCheckedFiles.remove(path);
    removeParentSelection(idx.parent());
    emit dataChanged(idx, idx);
}

QString FileSystemModel::subPath(const QModelIndex &idx) const
{
    return filePath(idx).remove(0, rootDirectory().canonicalPath().size()+1);
}

QString FileSystemModel::subPath(const QString &path) const
{
    return QString(path).remove(0, rootDirectory().canonicalPath().size()+1);
}

MiroModelAssemblyDialog::MiroModelAssemblyDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MiroModelAssemblyDialog)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

MiroModelAssemblyDialog::~MiroModelAssemblyDialog()
{
    delete ui;
}

QStringList MiroModelAssemblyDialog::selectedFiles()
{
    if (mFileSystemModel)
        return mFileSystemModel->selectedFiles();
    return QStringList();
}

void MiroModelAssemblyDialog::setWorkingDirectory(const QString &workingDirectory)
{
    mWorkingDirectory = workingDirectory;
    setupViewModel();
}

void MiroModelAssemblyDialog::on_createButton_clicked()
{
    if (!mFileSystemModel)
        return;
    if (selectedFiles().isEmpty())
        showMessageBox(this);
    else
        accept();
}

void MiroModelAssemblyDialog::on_selectAllButton_clicked()
{
    if (mFileSystemModel)
        mFileSystemModel->selectAll();
}

void MiroModelAssemblyDialog::on_clearButton_clicked()
{
    if (mFileSystemModel) {
        mFileSystemModel->clearSelection();
        auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
        ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
    }
}

void MiroModelAssemblyDialog::setupViewModel()
{
    if (mWorkingDirectory.isEmpty())
        return;

    mFileSystemModel = new FileSystemModel(this);
    mFileSystemModel->setRootPath(mWorkingDirectory);
    mFilterModel = new FilteredFileSystemModel(this);
    mFilterModel->setSourceModel(mFileSystemModel);

    auto oldModel = ui->directoryView->selectionModel();
    ui->directoryView->setModel(mFilterModel);
    delete oldModel;

    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
    ui->directoryView->expandAll();
}

void MiroModelAssemblyDialog::showMessageBox(QWidget *parent)
{
    QMessageBox::critical(parent, "No deployment files!", "Please select the files for your MIRO deployment.");
}

}
}
}
