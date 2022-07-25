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
    QRegExp mUncommonRegEx;
    Q_OBJECT
public:
    FilteredFileSystemModel(QObject *parent = nullptr) { Q_UNUSED(parent) }
    bool isDir(const QModelIndex &index) const;
    void setHideUncommonFiles(bool hide) { mHideUncommon = hide; invalidateFilter(); }
    void setUncommonRegExp(QRegExp rex) { mUncommonRegEx = rex; invalidateFilter(); }
protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
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
    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void selectAll();
    void clearSelection();
    QStringList selectedFiles(bool addWriteBackState = false);
    void setSelectedFiles(const QStringList &files);
    bool hasSelection();
    void setRootDir(const QDir &dir);
    int selectionCount();

signals:
    void selectionCountChanged(int count);
    void missingFiles(QStringList files);

private slots:
    void newDirectoryData(const QString &path);
    void updateDirCheckStates();

private:
    int dirCheckState(const QString &path, bool isConst = true) const;
    void updateDirInfo(const QModelIndex &idx) const;
    void invalidateDirState(const QModelIndex &par);
    void invalidateDirStates();

    void setChildSelection(const QModelIndex &idx, bool remove);
    void selectAllFiles(const QDir &dir);

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
