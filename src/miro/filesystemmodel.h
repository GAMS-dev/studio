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
#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QDialog>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QSet>
#include <QTimer>

namespace gams {
namespace studio {
namespace miro {

class FilteredFileSystemModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FilteredFileSystemModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsColumn(int source_column,
                             const QModelIndex& source_parent) const override;
};

class FileSystemModel : public QFileSystemModel
{
    struct DirState {
        DirState() {}
        int childCount = 0;
        int checkState = -1; // Qt::CheckState plus invalid state (-1)
    };

    Q_OBJECT
public:
    FileSystemModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void selectAll();
    void clearSelection();
    QStringList selectedFiles();
    void setSelectedFiles(const QStringList &files);
    void setRootDir(const QDir &dir);

private slots:
    void newDirectoryData(const QString &path);
    void updateDirCheckStates();

private:
    Qt::CheckState dirCheckState(const QString &file, bool isConst = true) const;
    void updateDirInfo(const QModelIndex &idx) const;
    void invalidateDirState(const QModelIndex &par);
    void invalidateDirStates();

    void setChildSelection(const QModelIndex &idx, bool remove);
    void selectAllFiles(const QDir &dir);
    QString subPath(const QModelIndex &idx) const;
    QString subPath(const QString &path) const;

private:
    QTimer mUpdateTimer;
    mutable QMap<QString,DirState> mDirs;
    QSet<QString> mCheckedFiles;
};

}
}
}

#endif // FILESYSTEMMODEL_H
