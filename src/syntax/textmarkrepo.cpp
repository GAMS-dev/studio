#include "textmarkrepo.h"
#include "file/filemetarepo.h"
#include "file/filemeta.h"
#include "file/projectrepo.h"
#include "logger.h"
#include <QMultiHash>
#include <QMutableMapIterator>

namespace gams {
namespace studio {

TextMarkRepo::TextMarkRepo(FileMetaRepo *fileRepo, ProjectRepo *projectRepo, QObject *parent)
    : QObject(parent), mFileRepo(fileRepo), mProjectRepo(projectRepo)
{
}

TextMarkRepo::~TextMarkRepo()
{
}

void TextMarkRepo::removeMarks(FileId fileId, NodeId groupId, QSet<TextMark::Type> types, int lineNr)
{
    removeMarks(fileId, groupId, false, types, lineNr);
}

void TextMarkRepo::removeMarks(FileId fileId, QSet<TextMark::Type> types, int lineNr)
{
    removeMarks(fileId, NodeId(), true, types, lineNr);
}

void TextMarkRepo::removeMarks(FileId fileId, NodeId groupId, bool allGroups, QSet<TextMark::Type> types, int lineNr)
{
    LineMarks* marks = mMarks.value(fileId);
    if (!marks) return;
    bool remainingBookmarks = false;
    QSet<NodeId> groups;
    LineMarks::iterator it = marks->begin();
    if ((types.isEmpty() || types.contains(TextMark::all)) && lineNr == -1 && allGroups) {
        // delete all
        while (it != marks->end()) {
            delete *it;
            ++it;
        }
        marks->clear();
    } else {
        // delete conditionally
        while (it != marks->end()) {
            TextMark* mark = (*it);
            ++it;
            if (types.contains(mark->type())
                    && (allGroups || mark->groupId() == groupId)
                    && (lineNr == -1 || lineNr == mark->line()) ) {
                groups << mark->groupId();
                marks->remove(mark->line(), mark);
                delete mark;
            } else {
                if (!remainingBookmarks && mark->type() == TextMark::bookmark)
                    remainingBookmarks = true;
            }
        }
    }
    if (!remainingBookmarks) mBookmarkedFiles.removeAll(fileId);
    if (groups.isEmpty()) return;
    FileMeta *fm = mFileRepo->fileMeta(fileId);
    if (fm) fm->marksChanged();
}

TextMark *TextMarkRepo::createMark(const FileId fileId, TextMark::Type type, int line, int column, int size)
{
    return createMark(fileId, NodeId(), type, 0, line, column, size);
}

TextMark *TextMarkRepo::createMark(const FileId fileId, const NodeId groupId, TextMark::Type type, int value
                                   , int line, int column, int size)
{
    Q_UNUSED(value)
    if (groupId < 0) {
        DEB() << "No valid groupId to create a TextMark";
        return nullptr;
    }
    if (!fileId.isValid()) {
        DEB() << "No valid fileId to create a TextMark";
        return nullptr;
    }
    if (!mMarks.contains(fileId)) {
        mMarks.insert(fileId, new LineMarks());
    }
    TextMark* mark = new TextMark(this, fileId, type, groupId);
    mark->setPosition(line, column, size);
    LineMarks *marks = mMarks.value(fileId);
    marks->insert(mark->line(), mark);
    if (mark->type() == TextMark::bookmark && !mBookmarkedFiles.contains(fileId))
        mBookmarkedFiles << fileId;
    FileMeta *fm = mFileRepo->fileMeta(fileId);
    if (fm) fm->marksChanged();
    return mark;
}

bool TextMarkRepo::hasBookmarks(FileId fileId, NodeId groupId)
{
    return mBookmarkedFiles.contains(fileId);
}

TextMark *TextMarkRepo::findBookmark(FileId fileId, NodeId groupId, int currentLine, bool back)
{
    TextMark* res = nullptr;
    QList<TextMark*> bookmarks = marks(fileId, -1, groupId, TextMark::bookmark);
    for (TextMark *mark: bookmarks) {
        if (back) {
            if ((currentLine < 0 || mark->line() < currentLine) && (!res || res->line() < mark->line())) res = mark;
        } else {
            if (mark->line() > currentLine && (!res || res->line() > mark->line())) res = mark;
        }
    }
    return res;
}

QTextDocument *TextMarkRepo::document(FileId fileId) const
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    return fm ? fm->document() : nullptr;
}

void TextMarkRepo::clear()
{
    while (!mMarks.isEmpty()) {
        QHash<FileId, LineMarks*>::iterator it = mMarks.begin();
        LineMarks *marks = *it;
        FileId fileId = it.key();
        removeMarks(fileId);
        mMarks.remove(fileId);
        delete marks;
    }
    mBookmarkedFiles.clear();
}

void TextMarkRepo::jumpTo(TextMark *mark, bool focus)
{
    FileMeta* fm = mFileRepo->fileMeta(mark->fileId());
    mProjectRepo->findOrCreateFileNode(fm, mProjectRepo->findRunGroup(mark->groupId()));

    if (fm) fm->jumpTo(mark->groupId(), focus, mark->line(), mark->blockEnd());
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

QList<TextMark*> TextMarkRepo::marks(FileId fileId, int lineNr, NodeId groupId, TextMark::Type refType, int max) const
{
    QList<TextMark*> res;
    if (!mMarks.contains(fileId)) return res;
    QList<TextMark*> marks = (lineNr < 0) ? mMarks.value(fileId)->values() : mMarks.value(fileId)->values(lineNr);
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

const LineMarks *TextMarkRepo::marks(FileId fileId)
{
    if (!mMarks.contains(fileId)) {
        mMarks.insert(fileId, new LineMarks());
    }
    return mMarks.value(fileId, nullptr);
}

void TextMarkRepo::shiftMarks(FileId fileId, int firstLine, int lineShift)
{
    LineMarks *marks = mMarks.value(fileId);
    if (!marks->size() || !lineShift) return;
    QMutableMapIterator<int, TextMark*> it(*marks);
    QVector<TextMark*> parked;
    if (lineShift < 0) {
        while (it.hasNext()) {
            it.next();
            if (it.key() < firstLine) continue;
            parked << it.value();
            it.remove();
        }
    } else {
        it.toBack();
        while (it.hasPrevious()) {
            it.previous();
            if (it.key() < firstLine) break;
            parked << it.value();
            it.remove();
        }
    }
    for (TextMark *mark: parked) {
        mark->setLine(mark->line()+lineShift);
        marks->insert(mark->line(), mark);
    }
    FileMeta *fm = mFileRepo->fileMeta(fileId);
    if (fm) fm->marksChanged();
}

void TextMarkRepo::setDebugMode(bool debug)
{
    mDebug = debug;
    if (!debug) return;
    DEB() << "\n--------------- TextMarks ---------------";
    QList<int> keys;
    for (FileId id: mMarks.keys()) {
        keys << int(id);
    }
    std::sort(keys.begin(), keys.end());
    for (int key: keys) {
        DEB() << key << ": " << mMarks.value(FileId(key))->size();
    }
}

bool TextMarkRepo::debugMode() const
{
    return mDebug;
}

FileId TextMarkRepo::ensureFileId(QString location)
{
    if (location.isEmpty()) return -1;
    FileMeta* fm = mFileRepo->findOrCreateFileMeta(location);
    if (fm) return fm->id();
    return -1;
}

LineMarks::LineMarks() : QMultiMap<int, TextMark *>()
{
}

bool LineMarks::hasVisibleMarks() const
{
    QList<TextMark*> tm = values();
    for (TextMark* t : tm) {
        if ((t->type() == TextMark::link) || (t->type() == TextMark::error)
                || (t->type() == TextMark::bookmark))
            return true;
    }
    return false;
}

} // namespace studio
} // namespace gams
