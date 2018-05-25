#ifndef TEXTMARKREPO_H
#define TEXTMARKREPO_H

#include <QObject>
#include <QMultiHash>
#include "textmark.h"
#include "common.h"

namespace gams {
namespace studio {

class FileMetaRepo;
class ProjectRepo;

class TextMarkRepo: public QObject
{
    Q_OBJECT
public:
    //
//    TextMarkRepo(FileMetaRepo* fileRepo, QObject *parent = nullptr);

    TextMarkRepo(ProjectRepo* fileRepo, QObject *parent = nullptr);
    void remove(TextMark* tm);
    TextMark* create(TextMarkData* tmData);
    QTextDocument* document(FileId fileId) const;

//    FileMetaRepo *fileRepo() const { return mFileRepo; }
    ProjectRepo *fileRepo() const { return mFileRepo; }
    bool openFile(FileId fileId, bool focus = false);
    void jumpTo(FileId fileId, QTextCursor cursor, bool focus = false);
    void rehighlightAt(FileId fileId, int pos);
    FileType::Kind fileKind(FileId fileId);


private:
//    FileMetaRepo* mFileRepo = nullptr;
    ProjectRepo* mFileRepo = nullptr;
    QMultiHash<FileId, TextMark*> mMarks;

private:
    FileId ensureFileId(QString location, QString contextLocation);

};

} // namespace studio
} // namespace gams

#endif // TEXTMARKREPO_H
