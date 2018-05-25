#include "textmarkrepo.h"
#include "file/projectrepo.h"
#include <QMultiHash>

namespace gams {
namespace studio {

TextMarkRepo::TextMarkRepo(ProjectRepo *fileRepo, QObject *parent)
    : QObject(parent), mFileRepo(fileRepo)
{
}

void TextMarkRepo::remove(TextMark *tm)
{
    mMarks.remove(tm->mFileId, tm);
}

TextMark *TextMarkRepo::create(TextMarkData *tmData)
{
    FileId fileId = ensureFileId(tmData->location, tmData->contextLocation);
    ProjectFileNode* fn = mFileRepo->fileNode(fileId);
    // TODO (JM) to be completed
}

QTextDocument *TextMarkRepo::document(FileId fileId) const
{
    ProjectFileNode* fn = mFileRepo->fileNode(fileId);
    if (fn) return fn->document();
    return nullptr;
}

bool TextMarkRepo::openFile(FileId fileId, bool focus)
{
    ProjectFileNode* fn = mFileRepo->fileNode(fileId);
    if (fn) {
        emit mFileRepo->openFile(fn, focus, fn->codecMib());
        return true;
    }
    return false;
}

void TextMarkRepo::jumpTo(FileId fileId, QTextCursor cursor, bool focus)
{
    ProjectFileNode* fn = mFileRepo->fileNode(fileId);
    if (fn) fn->jumpTo(cursor, focus);
}

void TextMarkRepo::rehighlightAt(FileId fileId, int pos)
{
    ProjectFileNode* fn = mFileRepo->fileNode(fileId);
    if (fn) fn->rehighlightAt(pos);
}

FileType::Kind TextMarkRepo::fileKind(FileId fileId)
{
    ProjectFileNode* fn = mFileRepo->fileNode(fileId);
    if (fn) return fn->metrics().fileType().kind();
    return FileType::None;
}

FileId TextMarkRepo::ensureFileId(QString location, QString contextLocation)
{
    ProjectGroupNode* gn = nullptr;
    // TODO (JM) to be completed
    if (!contextLocation.isEmpty()) {
        mFileRepo->findFile();
        ProjectGroupNode* gn = mFileRepo->fileNode(contextLocation);
    }
    ProjectFileNode* fn = nullptr;
    mFileRepo->findOrCreateFileNode(location, fn, gn);
    if (fn) return fn->id();
    return -1;
}

} // namespace studio
} // namespace gams
