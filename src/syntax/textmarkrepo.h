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

typedef QMultiMap<int, TextMark*> LineMarks;

class TextMarkRepo: public QObject
{
    Q_OBJECT
public:
    // TODO(JM) initialize fileRepo and projectRepo later (not in constructor)
    explicit TextMarkRepo(FileMetaRepo* fileRepo, ProjectRepo *projectRepo, QObject *parent = nullptr);
    ~TextMarkRepo() override;

    void removeMarks(FileId fileId, NodeId groupId, QSet<TextMark::Type> types = QSet<TextMark::Type>());
    void removeMarks(FileId fileId, QSet<TextMark::Type> types = QSet<TextMark::Type>());
    TextMark* createMark(const FileId fileId, TextMark::Type type, int line, int column, int size = 0);
    TextMark* createMark(const FileId fileId, const NodeId groupId, TextMark::Type type, int value, int line, int column, int size = 0);
    QTextDocument* document(FileId fileId) const;

    FileMetaRepo *fileRepo() const { return mFileRepo; }
    void clear();
    void jumpTo(TextMark *mark, bool focus = false);
    void rehighlight(FileId fileId, int line);
    FileKind fileKind(FileId fileId);
    QList<TextMark *> marks(FileId nodeId, int lineNr, NodeId groupId = -1, TextMark::Type refType = TextMark::all, int max = -1) const;
    const LineMarks *marks(FileId fileId);
    void shiftMarks(FileId fileId, int firstLine, int lineShift);

    void setDebugMode(bool debug);
    bool debugMode() const;

private:
    FileMetaRepo* mFileRepo = nullptr;
    ProjectRepo* mProjectRepo = nullptr;
    QHash<FileId, LineMarks*> mMarks;
    bool mDebug = false;

private:
    FileId ensureFileId(QString location);
    void removeMarks(FileId fileId, NodeId groupId, bool allGroups, QSet<TextMark::Type> types);

};

} // namespace studio
} // namespace gams

#endif // TEXTMARKREPO_H
