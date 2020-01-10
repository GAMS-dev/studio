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
#ifndef MIROMODELASSEMBLYDIALOG_H
#define MIROMODELASSEMBLYDIALOG_H

#include <QDialog>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QSet>

namespace gams {
namespace studio {
namespace miro {

namespace Ui {
class MiroModelAssemblyDialog;
}

class FilteredFileSystemModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FilteredFileSystemModel(QObject *parent = nullptr);

protected:
  bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
};

class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    FileSystemModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void selectAll();
    void clearSelection();
    QStringList selectedFiles();
    void setSelectedFiles(const QStringList &files);

private:
    QSet<QString> mCheckedFiles;
};

class MiroModelAssemblyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MiroModelAssemblyDialog(const QString &workingDirectory, QWidget *parent = nullptr);
    ~MiroModelAssemblyDialog();

    QStringList selectedFiles();
    void setSelectedFiles(const QStringList &files) {
        mFileSystemModel->setSelectedFiles(files);
    }

private slots:
    void on_createButton_clicked();
    void on_selectAllButton_clicked();
    void on_clearButton_clicked();

private:
    void showMessageBox();

private:
    Ui::MiroModelAssemblyDialog *ui;
    FileSystemModel *mFileSystemModel;
};

}
}
}

#endif // MIROMODELASSEMBLYDIALOG_H
