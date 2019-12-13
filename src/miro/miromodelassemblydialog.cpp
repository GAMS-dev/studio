/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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

QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::CheckStateRole) {
        if (mCheckedIndexes.contains(index)) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }
    return QFileSystemModel::data(index, role);
}

bool FileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        if (value.toBool()) {
            mCheckedIndexes.insert(index);
            emit dataChanged(index,
                             match(index, Qt::DisplayRole, "*", -1, Qt::MatchWildcard | Qt::MatchRecursive).last());
            return true;
        } else {
            auto pos = mCheckedIndexes.find(index);
            mCheckedIndexes.erase(pos);
            emit dataChanged(index,
                             match(index, Qt::DisplayRole, "*", -1, Qt::MatchWildcard | Qt::MatchRecursive).last());
            return true;
        }
    }
    return  QFileSystemModel::setData(index, value, role);
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

void FileSystemModel::selectAll()
{
    QDir dir(rootPath());
    auto entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto entry: entries) {
        auto idx = index(rootPath() + "/" + entry);
        mCheckedIndexes.insert(idx);
        emit dataChanged(idx, idx);
    }
}

void FileSystemModel::clearSelection()
{
    auto checkedIndexes = mCheckedIndexes;
    for (auto index: checkedIndexes) {
        mCheckedIndexes.remove(index);
        emit dataChanged(index, index);
    }
}

QStringList FileSystemModel::selectedFiles()
{
    QDir dir(rootPath());
    QStringList selection;
    for (auto index: mCheckedIndexes)
        selection << dir.relativeFilePath(filePath(index));
    return selection;
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
