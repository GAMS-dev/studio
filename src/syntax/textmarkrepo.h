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

    inline void deleteMark(TextMark *tm);
    void removeMark(TextMark *tm);
    void removeMarks(FileId fileId, QSet<TextMark::Type> types = QSet<TextMark::Type>());
    TextMark* createMark(const FileId fileId, TextMark::Type type, int line, int column, int size = 0);
    TextMark* createMark(const FileId fileId, const FileId runId, TextMark::Type type, int value, int line, int column, int size = 0);
    QTextDocument* document(FileId fileId) const;

    FileMetaRepo *fileRepo() const { return mFileRepo; }
    void jumpTo(TextMark *mark, bool focus = false);
    void rehighlight(FileId fileId, int line);
    FileKind fileKind(FileId fileId);
    QList<TextMark *> marks(FileId nodeId, int lineNr = -1, FileId runId = -1, TextMark::Type refType = TextMark::all, int max = -1);


private:
    typedef QMultiMap<int, TextMark*> FileMarks;
    FileMetaRepo* mFileRepo = nullptr;
    ProjectRepo* mProjectRepo = nullptr;
    QHash<FileId, FileMarks*> mMarks;

private:
    FileId ensureFileId(QString location);

};

} // namespace studio
} // namespace gams

#endif // TEXTMARKREPO_H
