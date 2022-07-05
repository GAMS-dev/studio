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
#include "filesystemwidget.h"
#include "ui_filesystemwidget.h"
#include "filesystemmodel.h"
#include "theme.h"

#include <QMessageBox>

namespace gams {
namespace studio {
namespace fs {

FileSystemWidget::FileSystemWidget(QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::FileSystemWidget)
    , mFileSystemModel(new FileSystemModel(this))
    , mFilterModel(new FilteredFileSystemModel(this))
{
    ui->setupUi(this);

    mFilterModel->setSourceModel(mFileSystemModel);
    auto oldModel = ui->directoryView->selectionModel();
    ui->directoryView->setModel(mFilterModel);
    connect(mFileSystemModel, &FileSystemModel::dataChanged, this, &FileSystemWidget::updateButtons);
    delete oldModel;
}

void FileSystemWidget::clear()
{
    mFileSystemModel->clearSelection();
}

QString FileSystemWidget::assemblyFileName() const {
    return mModelAssemblyFile;
}

void FileSystemWidget::setAssemblyFileName(const QString &file) {
    mModelAssemblyFile = file;
    QFileInfo fi(mModelAssemblyFile);
    mValidAssemblyFile = fi.exists();
    if (fi.exists()) {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Green));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText("File " + fi.fileName() + " found.");
        ui->createButton->setText("Save Changes");
    } else {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Red));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText("No file " + fi.fileName() + " found!");
        ui->createButton->setText("Create");
    }
}

bool FileSystemWidget::validAssemblyFile() const
{
    return mValidAssemblyFile;
}

QStringList FileSystemWidget::selectedFiles()
{
    if (mFileSystemModel)
        return mFileSystemModel->selectedFiles();
    return QStringList();
}

void FileSystemWidget::setSelectedFiles(const QStringList &files)
{
    mFileSystemModel->setSelectedFiles(files);
    ui->createButton->setEnabled(false);
}

void FileSystemWidget::setWorkingDirectory(const QString &workingDirectory)
{
    mWorkingDirectory = workingDirectory;
    setupViewModel();
}

QString FileSystemWidget::workingDirectory() const
{
    return mWorkingDirectory;
}

void FileSystemWidget::on_createButton_clicked()
{
    emit createButtonClicked();
    ui->createButton->setEnabled(false);
}

void FileSystemWidget::on_selectAllButton_clicked()
{
    mFileSystemModel->selectAll();
}

void FileSystemWidget::on_clearButton_clicked()
{
    mFileSystemModel->clearSelection();
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
}

void FileSystemWidget::updateButtons()
{
    ui->createButton->setEnabled(true);
    ui->clearButton->setEnabled(!selectedFiles().isEmpty());
}

void FileSystemWidget::setupViewModel()
{
    if (mWorkingDirectory.isEmpty())
        return;

    mFileSystemModel->setRootPath(mWorkingDirectory);
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
}

}
}
}
