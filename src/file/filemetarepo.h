#ifndef FILEMETAREPO_H
#define FILEMETAREPO_H

#include <QObject>
#include "filemeta.h"

namespace gams {
namespace studio {

class FileMetaRepo : public QObject
{
    Q_OBJECT
public:
    FileMetaRepo(QObject* parent, StudioSettings* settings);
    FileMeta* fileMeta(const FileId &fileId) const;
    FileMeta* fileMeta(const QString &location) const;
    FileMeta *findOrCreateFileMeta(QString location);
    StudioSettings *settings() const;

signals:
    void openFile(FileMeta* fm, FileId runId, bool focus = true, int codecMib = -1);

private:
    FileId addFileMeta(FileMeta* fileMeta);

private:
    FileId mNextFileId = 0;
    StudioSettings *mSettings = nullptr;
    QHash<FileId, FileMeta*> mFiles;


};

} // namespace studio
} // namespace gams

#endif // FILEMETAREPO_H
