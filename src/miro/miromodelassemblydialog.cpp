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
#include <QDebug>

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

    if (source_column == 0)
        return true;
    return false;
}

FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
}

QVariant FileSystemModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    if (role == Qt::CheckStateRole) {
        auto file = rootDirectory().relativeFilePath(filePath(idx));
        if (mCheckedFiles.contains(file)) {
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
        auto file = rootDirectory().relativeFilePath(filePath(idx));
        if (value.toBool()) {
            mCheckedFiles.insert(file);
            emit dataChanged(idx, idx);
            return true;
        } else {
            auto pos = mCheckedFiles.find(file);
            mCheckedFiles.erase(pos);
            emit dataChanged(idx, idx);
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
        mCheckedFiles.insert(idx.data().toString());
        emit dataChanged(idx, idx);
    }
}

void FileSystemModel::clearSelection()
{
    auto checkedFiles = mCheckedFiles;
    for (auto idx: checkedFiles) {
        mCheckedFiles.remove(idx);
        emit dataChanged(index(rootDirectory().absoluteFilePath(idx)),
                         index(rootDirectory().absoluteFilePath(idx)));
    }
}

QStringList FileSystemModel::selectedFiles()
{
    QStringList selection;
    for (auto file: mCheckedFiles)
        selection << file;
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

MiroModelAssemblyDialog::MiroModelAssemblyDialog(const QString &workingDirectory, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MiroModelAssemblyDialog),
      mFileSystemModel(new FileSystemModel(this))
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mFileSystemModel->setRootPath(workingDirectory);
    auto filterModel = new FilteredFileSystemModel(this);
    filterModel->setSourceModel(mFileSystemModel);

    auto oldModel = ui->directoryView->selectionModel();
    ui->directoryView->setModel(filterModel);
    delete oldModel;

    auto rootIndex = mFileSystemModel->index(workingDirectory);
    ui->directoryView->setRootIndex(filterModel->mapFromSource(rootIndex));
    ui->directoryView->expandAll();
}

MiroModelAssemblyDialog::~MiroModelAssemblyDialog()
{
    delete ui;
}

QStringList MiroModelAssemblyDialog::selectedFiles()
{
    return mFileSystemModel->selectedFiles();
}

void MiroModelAssemblyDialog::on_createButton_clicked()
{
    if (selectedFiles().isEmpty()) {
        showMessageBox();
        return;
    }
    accept();
}

void MiroModelAssemblyDialog::on_selectAllButton_clicked()
{
    mFileSystemModel->selectAll();
}

void MiroModelAssemblyDialog::on_clearButton_clicked()
{
    mFileSystemModel->clearSelection();
}

void MiroModelAssemblyDialog::showMessageBox()
{
    QMessageBox::critical(this, "No deployment files!", "Please select the files for your MIRO deployment.");
}

}
}
}
