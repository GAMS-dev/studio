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

bool TextMarkRepo::openFile(FileId fileId, FileId runId, bool focus)
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    if (fm) {
        emit mFileRepo->openFile(fm, runId, focus, fm->codecMib());
        return true;
    }
    return false;
}

void TextMarkRepo::jumpTo(FileId fileId, QTextCursor cursor, bool focus)
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    if (fm) fm->jumpTo(cursor, focus);
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

QList<TextMark*> TextMarkRepo::marks(FileId nodeId, TextMark::Type refType)
{
    if (refType != TextMark::all) {
        QList<TextMark*> res;
        foreach (TextMark* mark, mMarks.values(nodeId))
            if (mark->type() == refType) res << mark;
        return res;
    }
    mMarks.values(nodeId);
}

FileId TextMarkRepo::ensureFileId(QString location)
{
    if (location.isEmpty()) return -1;
    FileMeta* fm = nullptr;
    mFileRepo->findOrCreateFileNode(location, fm, nullptr);
    if (fm) return fm->id();
    return -1;
}

} // namespace studio
} // namespace gams
