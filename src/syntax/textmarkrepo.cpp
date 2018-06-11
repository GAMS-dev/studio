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

void TextMarkRepo::removeMark(TextMark *tm)
{
    mMarks.remove(tm->fileId(), tm);
}

void TextMarkRepo::removeMarks(FileId fileId)
{
    foreach (TextMark* mark, mMarks.values(fileId))
        delete mark;
    mMarks.remove(fileId);
}

TextMark *TextMarkRepo::createMark(TextMarkData *tmData)
{
    FileId fileId = ensureFileId(tmData->location);
    FileId contextId = ensureFileId(tmData->contextLocation);
    TextMark* mark = new TextMark(this, fileId, tmData->type, contextId);
    mMarks.insert(fileId, mark);
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
    emit mFileRepo->openFile(fm, mark->runId(), focus, fm->codecMib());
    if (mark->document())  {
        mark->updatePos();
        if (fm) fm->jumpTo(mark->textCursor(), mark->runId(), focus);
    }
}

void TextMarkRepo::rehighlightAt(FileId fileId, int pos)
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    if (fm) fm->rehighlightAt(pos);
}

FileKind TextMarkRepo::fileKind(FileId fileId)
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    if (fm) return fm->kind();
    return FileKind::None;
}

QVector<TextMark *> TextMarkRepo::marksForBlock(FileId nodeId, QTextBlock block, TextMark::Type refType)
{
    return marksForBlock(nodeId, block, -1, refType);
}

QVector<TextMark *> TextMarkRepo::marksForBlock(FileId nodeId, QTextBlock block, FileId runId, TextMark::Type refType)
{
    // TODO(JM) rebuild this to line/row behavior
    QVector<TextMark*> res;
    QList<TextMark*> marks = mMarks.values(nodeId);
    int i = block.blockNumber()+2 < block.document()->blockCount() ? 0 : qMax(marks.size()-4, 0);
    for (; i < marks.size(); i++) {
        TextMark* tm = marks.at(i);
        int hit = tm->in(block.position(), block.length()-1);
        if (hit == 0 && (refType == TextMark::all || refType == tm->refType())) {
            res << tm;
        }
    }
    return res;
}

QList<TextMark*> TextMarkRepo::marks(FileId fileId, FileId runId = -1, TextMark::Type refType, int max)
{
    if (refType != TextMark::all) {
        QList<TextMark*> res;
        foreach (TextMark* mark, mMarks.values(fileId)) {
            if (mark->type() == refType && (runId < 0 || mark->runId() == runId)) {
                res << mark;
                if (--max == 0) break;
            }
        }
        return res;
    }
    mMarks.values(fileId);
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
