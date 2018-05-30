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

    // TODO(JM) next step: shoft from ProjectRepo to FileMetaRepo
//    TextMarkRepo(FileMetaRepo* fileRepo, QObject *parent = nullptr);
    explicit TextMarkRepo(ProjectRepo* fileRepo, QObject *parent = nullptr);
    ~TextMarkRepo() override;

    void removeMark(TextMark* tm);
    void removeMarks(FileId fileId);
    TextMark* createMark(TextMarkData* tmData);
    QTextDocument* document(FileId fileId, NodeId groupId = -1) const;

//    FileMetaRepo *fileRepo() const { return mFileRepo; }
    ProjectRepo *fileRepo() const { return mFileRepo; }
    bool openFile(FileId fileId, bool focus = false);
    void jumpTo(FileId fileId, QTextCursor cursor, bool focus = false);
    void rehighlightAt(FileId fileId, int pos);
    FileKind fileKind(FileId fileId);
    QVector<TextMark*> marksForBlock(FileId nodeId, QTextBlock block, TextMark::Type refType = TextMark::all);
    QList<TextMark *> marks(FileId nodeId, TextMark::Type refType = TextMark::all);


private:
//    FileMetaRepo* mFileRepo = nullptr;
    ProjectRepo* mFileRepo = nullptr;
    QMultiHash<FileId, TextMark*> mMarks;

private:
    FileId ensureFileId(QString location);

};

} // namespace studio
} // namespace gams

#endif // TEXTMARKREPO_H
