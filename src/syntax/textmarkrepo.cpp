/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "textmarkrepo.h"
#include "file/filemetarepo.h"
#include "file/filemeta.h"
#include "file/projectrepo.h"
#include "logger.h"
#include <QMultiHash>
#include <QMutableMapIterator>

namespace gams {
namespace studio {

TextMarkRepo::TextMarkRepo(QObject *parent)
    : QObject(parent)
{
}

TextMarkRepo::~TextMarkRepo()
{
}

void TextMarkRepo::init(FileMetaRepo *fileRepo, ProjectRepo *projectRepo)
{
    mFileRepo = fileRepo;
    mProjectRepo = projectRepo;
}

void TextMarkRepo::removeMarks(const FileId &fileId, const NodeId &groupId, const QSet<TextMark::Type> &types, int lineNr, int lastLine)
{
    removeMarks(fileId, groupId, false, types, lineNr, (lastLine < 0 ? lineNr : lastLine));
}

void TextMarkRepo::removeMarks(const FileId &fileId, const QSet<TextMark::Type> &types, int lineNr, int lastLine)
{
    removeMarks(fileId, NodeId(), true, types, lineNr, (lastLine < 0 ? lineNr : lastLine));
}

void TextMarkRepo::removeMarks(const FileId &fileId, const NodeId &groupId, bool allGroups, const QSet<TextMark::Type> &types, int lineNr, int lastLine)
{
    LineMarks* marks = mMarks.value(fileId);
    if (!marks) return;
    bool remainingBookmarks = false;
    QSet<NodeId> groups;
    QSet<int> changedLines;
    LineMarks::iterator it = marks->begin();
    if ((types.isEmpty() || types.contains(TextMark::all)) && lineNr == -1 && allGroups) {
        // delete all
        while (it != marks->end()) {
            changedLines << it.value()->line();
            delete *it;
            ++it;
        }
        marks->clear();
    } else {
        // delete conditionally
        while (it != marks->end()) {
            TextMark* mark = (*it);
            ++it;
            if ((types.isEmpty() || types.contains(TextMark::all) || types.contains(mark->type()))
                    && (allGroups || mark->groupId() == groupId)
                    && (lineNr == -1 || (lineNr <= mark->line() && lastLine >= mark->line())) ) {
                groups << mark->groupId();
                marks->remove(mark->line(), mark);
                changedLines << mark->line();
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
    if (fm) fm->marksChanged(changedLines);
}

TextMark *TextMarkRepo::createMark(const FileId &fileId, TextMark::Type type, int line, int column, int size)
{
    return createMark(fileId, NodeId(), type, 0, line, column, size);
}

TextMark *TextMarkRepo::createMark(const FileId &fileId, const NodeId &groupId, TextMark::Type type, int value
                                   , int line, int column, int size)
{
    Q_UNUSED(value)
//    if (groupId < 0) {
//        DEB() << "No valid groupId to create a TextMark";
//        return nullptr;
//    }
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
    if (fm) {
        fm->marksChanged(QSet<int>() << line);
    }
    return mark;
}

bool TextMarkRepo::hasBookmarks(const FileId &fileId)
{
    return mBookmarkedFiles.contains(fileId);
}

TextMark *TextMarkRepo::findBookmark(const FileId &fileId, int currentLine, bool back)
{
    const LineMarks *lm = marks(fileId);
    if (lm->cbegin() == lm->cend()) return nullptr;
    if (back) {
        LineMarks::const_iterator it = currentLine < 0 ? lm->cend() : lm->lowerBound(currentLine);
        while (it-- != lm->begin()) {
            if ((currentLine < 0 || it.value()->line() < currentLine) && (it.value()->type() == TextMark::bookmark))
                return it.value();
        }
    } else {
        LineMarks::const_iterator it = lm->lowerBound(currentLine);
        while (it != lm->end()) {
            if ((it.value()->line() > currentLine) && (it.value()->type() == TextMark::bookmark))
                return it.value();
            ++it;
        }
    }
    return nullptr;


//    TextMark* res = nullptr;
//    QList<TextMark*> bookmarks = marks(fileId, -1, groupId, TextMark::bookmark);
//    for (TextMark *mark: bookmarks) {
//        if (back) {
//            if ((currentLine < 0 || mark->line() < currentLine) && (!res || res->line() < mark->line())) res = mark;
//        } else {
//            if (mark->line() > currentLine && (!res || res->line() > mark->line())) res = mark;
//        }
//    }
//    return res;
}

void TextMarkRepo::removeBookmarks()
{
    QVector<FileId> files = mBookmarkedFiles;
    for (const FileId &fileId: files) {
        removeMarks(fileId, QSet<TextMark::Type>() << TextMark::bookmark);
    }
}

QTextDocument *TextMarkRepo::document(const FileId &fileId) const
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    return fm ? fm->document() : nullptr;
}

void TextMarkRepo::clear()
{
    while (!mMarks.isEmpty()) {
        QHash<FileId, LineMarks*>::iterator it = mMarks.begin();
        LineMarks *marks = *it;
        const FileId &fileId = it.key();
        removeMarks(fileId);
        mMarks.remove(fileId);
        delete marks;
    }
    mBookmarkedFiles.clear();
}

void TextMarkRepo::jumpTo(TextMark *mark, bool focus, bool ignoreColumn)
{
    FileMeta* fm = mFileRepo->fileMeta(mark->fileId());
    mProjectRepo->findOrCreateFileNode(fm, mProjectRepo->findProject(mark->groupId()));

    if (fm) {
        if (mark->blockEnd() < 0) ignoreColumn = true;
        fm->jumpTo(mark->groupId(), focus, mark->line(), ignoreColumn ? 0 : mark->blockEnd());
    }
}

void TextMarkRepo::rehighlight(const FileId &fileId, int line)
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    if (fm) fm->rehighlight(line);
}

FileKind TextMarkRepo::fileKind(const FileId &fileId)
{
    FileMeta* fm = mFileRepo->fileMeta(fileId);
    if (fm) return fm->kind();
    return FileKind::None;
}

QList<TextMark*> TextMarkRepo::marks(const FileId &fileId, int lineNr, const NodeId &groupId, TextMark::Type refType, int max) const
{
    QList<TextMark*> res;
    if (!mMarks.contains(fileId)) return res;

    LineMarks *lMarks = mMarks.value(fileId);
    if (lMarks->isEmpty() || (lineNr >= 0 && !lMarks->contains(lineNr)) ) return res;

    res.reserve(lMarks->size());
    QPair<LineMarks::const_iterator, LineMarks::const_iterator> interval;
    if (lineNr < 0) {
        interval.first = lMarks->constBegin();
        interval.second = lMarks->constEnd();
    } else {
        interval = lMarks->equal_range(lineNr);
    }

    LineMarks::const_iterator it = interval.first;
    while (true) {
        if (it == interval.second) break;
        TextMark *tm = it.value();
        if (refType == TextMark::all || refType == tm->type()) {
            if (!groupId.isValid() || !tm->groupId().isValid() || groupId == tm->groupId()) {
                res << tm;
            }
        }
        if (res.size() == max) break;
        if (it == interval.second) break;
        ++it;
    }

    return res;
}

const LineMarks *TextMarkRepo::marks(const FileId &fileId)
{
    if (!mMarks.contains(fileId)) {
        mMarks.insert(fileId, new LineMarks());
    }
    return mMarks.value(fileId, nullptr);
}

void TextMarkRepo::shiftMarks(const FileId &fileId, int firstLine, int lineShift)
{
    LineMarks *marks = mMarks.value(fileId);
    if (!marks || !marks->size() || !lineShift) return;
    QSet<int> changedLines;
    changedLines.reserve(marks->size()*2);
    QMutableMultiMapIterator<int, TextMark*> it(*marks);
    QVector<TextMark*> parked;
    if (lineShift < 0) {
        while (it.hasNext()) {
            it.next();
            if (it.key() < firstLine) continue;
            changedLines << it.value()->line() << (it.value()->line()+lineShift);
            parked << it.value();
            it.remove();
        }
    } else {
        it.toBack();
        while (it.hasPrevious()) {
            it.previous();
            if (it.key() < firstLine) break;
            changedLines << it.value()->line() << (it.value()->line()+lineShift);
            parked << it.value();
            it.remove();
        }
    }
    for (TextMark *mark: std::as_const(parked)) {
        mark->setLine(lineShift<0 ? qMax(mark->line()+lineShift, firstLine): mark->line()+lineShift);
        marks->insert(mark->line(), mark);
    }
    FileMeta *fm = mFileRepo->fileMeta(fileId);
    if (fm) fm->marksChanged(changedLines);
}

void TextMarkRepo::setDebugMode(bool debug)
{
    mDebug = debug;
    if (!debug) return;
    DEB() << "\n--------------- TextMarks ---------------";
    QList<int> keys;
    for (auto it = mMarks.constBegin() ; it != mMarks.constEnd() ; ++it) {
        keys << int(it.key());
    }
    std::sort(keys.begin(), keys.end());
    for (int key: std::as_const(keys)) {
        DEB() << key << ": " << mMarks.value(FileId(key))->size();
    }
}

bool TextMarkRepo::debugMode() const
{
    return mDebug;
}

FileId TextMarkRepo::ensureFileId(const QString &location)
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
    QMultiMap<int, TextMark*>::const_iterator it;
    for (it = begin() ; it != end() ; ++it) {
        if ((it.value()->type() == TextMark::link) || (it.value()->type() == TextMark::error)
                || (it.value()->type() == TextMark::bookmark))
            return true;
    }
    return false;
}

} // namespace studio
} // namespace gams
