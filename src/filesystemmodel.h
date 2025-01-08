/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_FILESYSTEMMODEL_H
#define GAMS_STUDIO_FILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QSet>
#include <QTimer>

namespace gams {
namespace studio {
namespace fs {

enum FileSystemRole {
    WriteBackRole = Qt::UserRole + 5
};

class FilteredFileSystemModel : public QSortFilterProxyModel
{
    bool mHideUncommon = true;
    QRegularExpression mUncommonRegEx;
    Q_OBJECT
public:
    FilteredFileSystemModel(QObject *parent = nullptr);
    bool isDir(const QModelIndex &index) const;
    void setHideUncommonFiles(bool hide) { mHideUncommon = hide; invalidateFilter(); }
    void setUncommonRegExp(const QRegularExpression &rex) { mUncommonRegEx = rex; invalidateFilter(); }
    void setSourceModel(QAbstractItemModel *sourceModel) override;
protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

class FileSystemModel : public QFileSystemModel
{
//    struct Entry {
//        bool isDir = false;
//        QString name;
//        QString absoluteFilePath;
//        QString relativeFilePath;
//    };

    struct DirState {
        DirState() {}
        int childCount = -1;
        int checkState = -1; // Qt::CheckState plus invalid state (-1)
//        QList<Entry*> entries;
    };

    Q_OBJECT
public:
    FileSystemModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void selectAll();
    void clearSelection();
    QStringList selectedFiles(bool addWriteBackState = false);
    void setSelectedFiles(const QStringList &files);
    bool hasSelection();
    int selectionCount();

signals:
    void selectionCountChanged(int count);
    void missingFiles(QStringList files);
    void isFiltered(QModelIndex source_index, bool &filtered) const;

private slots:
    void newDirectoryData(const QString &path);
    void updateDirCheckStates();

private:
    int dirCheckState(const QString &path, bool filtered, bool isConst = true) const;
    void updateDirInfo(const QModelIndex &idx) const;
    void invalidateDirState(const QModelIndex &par);
    void invalidateDirStates();

    void setChildSelection(const QModelIndex &idx, bool remove);
    void selectAllFiles(const QDir &dir);
    const QList<QFileInfo> visibleFileInfoList(const QDir &dir) const;

    QString subPath(const QModelIndex &idx) const;
    QString subPath(const QString &path) const;

private:
    QTimer mUpdateTimer;
    mutable QMap<QString, DirState> mDirs;
    QMap<QString, bool> mWriteBack;
    QSet<QString> mSelectedFiles;
};

} // namespace fs
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FILESYSTEMMODEL_H
