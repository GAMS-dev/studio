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
#ifndef FILESYSTEMWIDGET_H
#define FILESYSTEMWIDGET_H

#include <QGroupBox>

namespace gams {
namespace studio {
namespace fs {

namespace Ui {
class FileSystemWidget;
}

class FileSystemModel;
class FilteredFileSystemModel;

class FileSystemWidget : public QGroupBox
{
    Q_OBJECT
public:
    FileSystemWidget(QWidget *parent = nullptr);
    void clear();
    QString assemblyFileName() const;
    void setAssemblyFileName(const QString &file);
    bool validAssemblyFile() const;
    QStringList selectedFiles();
    void setSelectedFiles(const QStringList &files);
    void setWorkingDirectory(const QString &workingDirectory);
    QString workingDirectory() const;

signals:
    void createButtonClicked();

private slots:
    void on_createButton_clicked();
    void on_selectAllButton_clicked();
    void on_clearButton_clicked();
    void updateButtons();

private:
    void setupViewModel();

private:
    Ui::FileSystemWidget *ui;
    QString mModelAssemblyFile;
    bool mValidAssemblyFile;
    QString mWorkingDirectory;

    FileSystemModel *mFileSystemModel;
    FilteredFileSystemModel *mFilterModel;
};

}
}
}

#endif // FILESYSTEMWIDGET_H
