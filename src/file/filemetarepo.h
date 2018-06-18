#ifndef FILEMETAREPO_H
#define FILEMETAREPO_H

#include <QObject>
#include <QFileSystemWatcher>
#include "filemeta.h"
#include "fileevent.h"
#include "common.h"

namespace gams {
namespace studio {

class TextMarkRepo;
class ProjectRepo;

class FileMetaRepo : public QObject
{
    Q_OBJECT
public:
    FileMetaRepo(QObject* parent, StudioSettings* settings);
    FileMeta* fileMeta(const FileId &fileId) const;
    FileMeta* fileMeta(const QString &location) const;
    FileMeta* fileMeta(QWidget * const &editor) const;
    FileMeta *findOrCreateFileMeta(QString location);
    StudioSettings *settings() const;
    void init(TextMarkRepo* textMarkRepo, ProjectRepo *projectRepo);
    TextMarkRepo *textMarkRepo() const;
    ProjectRepo *projectRepo() const;
    QVector<FileMeta*> openFiles() const;
    void unwatch(const QString &path);

signals:
    void fileEvent(FileEvent &e);

public slots:
    void openFile(FileMeta* fm, FileId runId, bool focus = true, int codecMib = -1);
    void removedFile(FileMeta* fileMeta);

private slots:
//    void dirChanged(const QString& path);
    void fileChanged(const QString& path);
    void reviewMissing();

private:
    FileId addFileMeta(FileMeta* fileMeta);

private:
    FileId mNextFileId = 0;
    StudioSettings *mSettings = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;
    ProjectRepo* mProjectRepo = nullptr;
    QHash<FileId, FileMeta*> mFiles;
    QFileSystemWatcher mWatcher;
    QStringList mCheckExistance; // List to be checked once
    QStringList mMissList; // List to be checked periodically

};

} // namespace studio
} // namespace gams

#endif // FILEMETAREPO_H
