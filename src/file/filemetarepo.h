#ifndef FILEMETAREPO_H
#define FILEMETAREPO_H

#include <QObject>
#include <QFileSystemWatcher>
#include "filemeta.h"

namespace gams {
namespace studio {

class TextMarkRepo;

class FileMetaRepo : public QObject
{
    Q_OBJECT
public:
    FileMetaRepo(QObject* parent, StudioSettings* settings);
    FileMeta* fileMeta(const FileId &fileId) const;
    FileMeta* fileMeta(const QString &location) const;
    FileMeta *findOrCreateFileMeta(QString location);
    StudioSettings *settings() const;
    void init(TextMarkRepo* textMarkRepo);
    TextMarkRepo *textMarkRepo() const;

signals:
    void openFile(FileMeta* fm, FileId runId, bool focus = true, int codecMib = -1);
    void removedFile(FileMeta* fm);

private slots:
    void dirChanged(const QString& path);
    void fileChanged(const QString& path);

private:
    FileId addFileMeta(FileMeta* fileMeta);

private:
    FileId mNextFileId = 0;
    StudioSettings *mSettings = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;
    QHash<FileId, FileMeta*> mFiles;
    QFileSystemWatcher mWatcher;
    QStringList mMissList;


};

} // namespace studio
} // namespace gams

#endif // FILEMETAREPO_H
