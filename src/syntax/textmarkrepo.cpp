#include "textmarkrepo.h"
#include "file/filemetarepo.h"
#include "file/filemeta.h"
#include "file/projectrepo.h"
#include <QMultiHash>

namespace gams {
namespace studio {

TextMarkRepo::TextMarkRepo(FileMetaRepo *fileRepo, ProjectRepo *projectRepo, QObject *parent)
    : QObject(parent), mFileRepo(fileRepo), mProjectRepo(projectRepo)
{
}

TextMarkRepo::~TextMarkRepo()
{
    while (!mMarks.isEmpty()) {
        int fileId = mMarks.begin().key();
        removeMarks(fileId);
    }
}

inline void TextMarkRepo::deleteMark(TextMark *tm)
{
    delete tm; //  ~TextMark() triggers removeMark()
}

void TextMarkRepo::removeMark(TextMark *tm)
{
    FileMarks *marks = mMarks.value(tm->fileId());
    marks->remove(tm->mId, tm);
    if (marks->isEmpty()) {
        mMarks.remove(tm->fileId());
        delete marks;
    }
}

void TextMarkRepo::removeMarks(FileId fileId, QSet<TextMark::Type> types)
{
    FileMarks *marks = mMarks.value(fileId);
    if (!marks || marks->isEmpty()) return;
    FileMarks::iterator it = marks->begin();
    if (types.isEmpty() || types.contains(TextMark::all)) {
        // delete all
        while (it != marks->end()) {
            delete *it;
            it = marks->erase(it);
        }
    } else {
        // delete conditionally
        while (it != marks->end()) {
            if (types.contains(it.value()->type())) {
                delete *it;
                it = marks->erase(it);
            } else {
                ++it;
            }
        }
    }
    if (marks->isEmpty()) {
        mMarks.remove(fileId);
        delete marks;
    }
}

TextMark *TextMarkRepo::createMark(const FileId fileId, TextMark::Type type, int line, int column, int size)
{
    return createMark(fileId, NodeId(), type, 0, line, column, size);
}

TextMark *TextMarkRepo::createMark(const FileId fileId, const NodeId groupId, TextMark::Type type, int value
                                   , int line, int column, int size)
{
    Q_UNUSED(value)
    if (!fileId.isValid()) return nullptr;
    TextMark* mark = new TextMark(this, fileId, type, groupId);
    mark->setPosition(line, column, size);
    if (!mMarks.contains(fileId)) mMarks.insert(fileId, new FileMarks());
    FileMarks *marks = mMarks.value(fileId);
    marks->insert(mark->line(), mark);
    return mark;
}

QTextDocument *TextMarkRepo::document(FileId fileId) const
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    return fm ? fm->document() : nullptr;
}

void TextMarkRepo::jumpTo(TextMark *mark, bool focus)
{
    FileMeta* fm = mFileRepo->fileMeta(mark->fileId());
    if (fm) fm->jumpTo(mark->groupId(), focus, mark->line(), mark->column());
}

void TextMarkRepo::rehighlight(FileId fileId, int line)
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    if (fm) fm->rehighlight(line);
}

FileKind TextMarkRepo::fileKind(FileId fileId)
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    if (fm) return fm->kind();
    return FileKind::None;
}

QList<TextMark*> TextMarkRepo::marks(FileId nodeId, int lineNr, NodeId groupId, TextMark::Type refType, int max) const
{
    QList<TextMark*> res;
    if (!mMarks.contains(nodeId)) return res;
    QList<TextMark*> marks = (lineNr < 0) ? mMarks.value(nodeId)->values() : mMarks.value(nodeId)->values(lineNr);
    if (groupId < 0 && refType == TextMark::all) return marks;
    int i = 0;
    for (TextMark* mark: marks) {
        if (refType != TextMark::all && refType != mark->type()) continue;
        if (groupId.isValid() && mark->groupId().isValid() && groupId != mark->groupId()) continue;
        res << mark;
        i++;
        if (i == max) break;
    }
    return res;
}

const FileMarks TextMarkRepo::marks(FileId nodeId) const
{
    if (mMarks.contains(nodeId)) return *mMarks.value(nodeId);
    return FileMarks();
}

FileId TextMarkRepo::ensureFileId(QString location)
{
    if (location.isEmpty()) return -1;
    FileMeta* fm = mFileRepo->findOrCreateFileMeta(location);
    if (fm) return fm->id();
    return -1;
}

} // namespace studio
} // namespace gams
