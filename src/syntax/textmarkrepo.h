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

class LineMarks: public QMultiMap<int, TextMark*>
{
public:
    LineMarks();
    bool hasVisibleMarks() const;
    TextMark* firstError(NodeId groupId) const {
        if (isEmpty()) return nullptr;
        QList<TextMark*> marks = values(firstKey());
        TextMark* res = nullptr;
        for (TextMark* mark: marks) {
            if (mark->type() != TextMark::error) continue;
            if (groupId.isValid() && groupId != mark->groupId()) continue;
            if (!res || res->column() > mark->column())
                res = mark;
        }
        return res;
    }
};

class TextMarkRepo: public QObject
{
    Q_OBJECT
public:
    explicit TextMarkRepo(QObject *parent = nullptr);
    ~TextMarkRepo() override;
    void init(FileMetaRepo *fileRepo, ProjectRepo *projectRepo);

    void removeMarks(FileId fileId, NodeId groupId, QSet<TextMark::Type> types = QSet<TextMark::Type>(), int lineNr = -1, int lastLine = -1);
    void removeMarks(FileId fileId, QSet<TextMark::Type> types = QSet<TextMark::Type>(), int lineNr = -1, int lastLine = -1);
    TextMark* createMark(const FileId fileId, TextMark::Type type, int line, int column, int size = 0);
    TextMark* createMark(const FileId fileId, const NodeId groupId, TextMark::Type type, int value, int line, int column, int size = 0);
    bool hasBookmarks(FileId fileId);
    TextMark* findBookmark(FileId fileId, int currentLine, bool back);
    void removeBookmarks();
    QTextDocument* document(FileId fileId) const;

    FileMetaRepo *fileRepo() const { return mFileRepo; }
    void clear();
    void jumpTo(TextMark *mark, bool focus = false, bool ignoreColumn = false);
    void rehighlight(FileId fileId, int line);
    FileKind fileKind(FileId fileId);
    QList<TextMark *> marks(FileId fileId, int lineNr, NodeId groupId = -1, TextMark::Type refType = TextMark::all, int max = -1) const;
    const LineMarks *marks(FileId fileId);
    void shiftMarks(FileId fileId, int firstLine, int lineShift);

    void setDebugMode(bool debug);
    bool debugMode() const;

private:
    FileMetaRepo* mFileRepo = nullptr;
    ProjectRepo* mProjectRepo = nullptr;
    QHash<FileId, LineMarks*> mMarks; // TODO(JM) LineMarks should be protected by smartpointer
    QVector<FileId> mBookmarkedFiles;
    bool mDebug = false;

private:
    FileId ensureFileId(QString location);
    void removeMarks(FileId fileId, NodeId groupId, bool allGroups, QSet<TextMark::Type> types, int lineNr, int lastLine);

};

} // namespace studio
} // namespace gams

#endif // TEXTMARKREPO_H
