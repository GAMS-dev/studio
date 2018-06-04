#ifndef TEXTMARKREPO_H
#define TEXTMARKREPO_H

#include <QObject>
#include <QMultiHash>
#include <QTextBlock>
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
    explicit TextMarkRepo(FileMetaRepo* fileRepo, ProjectRepo *projectRepo, QObject *parent = nullptr);
    ~TextMarkRepo() override;

    void removeMark(TextMark* tm);
    void removeMarks(FileId fileId);
    TextMark* createMark(TextMarkData* tmData);
    QTextDocument* document(FileId fileId) const;

//    FileMetaRepo *fileRepo() const { return mFileRepo; }
    FileMetaRepo *fileRepo() const { return mFileRepo; }
    bool openFile(FileId fileId, FileId runId, bool focus = false);
    void jumpTo(FileId fileId, QTextCursor cursor, bool focus = false);
    void rehighlightAt(FileId fileId, int pos);
    FileKind fileKind(FileId fileId);
    QVector<TextMark*> marksForBlock(FileId nodeId, QTextBlock block, TextMark::Type refType = TextMark::all);
    QList<TextMark *> marks(FileId nodeId, TextMark::Type refType = TextMark::all);


private:
    FileMetaRepo* mFileRepo = nullptr;
    ProjectRepo* mProjectRepo = nullptr;
    QMultiHash<FileId, TextMark*> mMarks;

private:
    FileId ensureFileId(QString location);

};

} // namespace studio
} // namespace gams

#endif // TEXTMARKREPO_H
