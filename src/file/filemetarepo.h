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
    FileMetaRepo(QObject* parent);
    FileMeta* fileMeta(const FileId fileId) const;
    FileMeta* fileMeta(const QString &location) const;
    FileId addFileMeta(FileMeta* fileMeta);
    FileMeta *getOrCreateFileMeta(QString location);

signals:
    void openFile(FileMeta* fm, NodeId groupId, bool focus = true, int codecMib = -1);

private:
    static FileId mNextFileId;
    QHash<FileId, FileMeta*> mFiles;
};

} // namespace studio
} // namespace gams

#endif // FILEMETAREPO_H
