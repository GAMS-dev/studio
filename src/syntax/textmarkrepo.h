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
    TextMark* firstError(const NodeId &groupId) const {
        if (isEmpty()) return nullptr;
        QList<TextMark*> marks = values(firstKey());
        TextMark* res = nullptr;
        for (TextMark* mark: std::as_const(marks)) {
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

    void removeMarks(const FileId &fileId, const NodeId &groupId, const QSet<TextMark::Type> &types = QSet<TextMark::Type>(), int lineNr = -1, int lastLine = -1);
    void removeMarks(const FileId &fileId, const QSet<TextMark::Type> &types = QSet<TextMark::Type>(), int lineNr = -1, int lastLine = -1);
    TextMark* createMark(const FileId &fileId, TextMark::Type type, int line, int column, int size = 0);
    TextMark* createMark(const FileId &fileId, const NodeId &groupId, TextMark::Type type, int value, int line, int column, int size = 0);
    bool hasBookmarks(const FileId &fileId);
    TextMark* findBookmark(const FileId &fileId, int currentLine, bool back);
    void removeBookmarks();
    void writeBookmarks(QVariantList &bookmarks);
    void readBookmarks(const QVariantList &bookmarks);
    QTextDocument* document(const FileId &fileId) const;

    FileMetaRepo *fileRepo() const { return mFileRepo; }
    void clear();
    void jumpTo(TextMark *mark, bool focus = false, bool ignoreColumn = false);
    void rehighlight(const FileId &fileId, int line);
    FileKind fileKind(const FileId &fileId);
    QList<TextMark *> marks(const FileId &fileId, int lineNr, const NodeId &groupId = -1, TextMark::Type refType = TextMark::all, int max = -1) const;
    const LineMarks *marks(const FileId &fileId);
    void shiftMarks(const FileId &fileId, int firstLine, int lineShift);

    void setDebugMode(bool debug);
    bool debugMode() const;

private:
    FileMetaRepo* mFileRepo = nullptr;
    ProjectRepo* mProjectRepo = nullptr;
    QHash<FileId, LineMarks*> mMarks;
    QList<FileId> mBookmarkedFiles;
    bool mDebug = false;

private:
    FileId ensureFileId(const QString &location);
    void removeMarks(const FileId &fileId, const NodeId &groupId, bool allGroups, const QSet<TextMark::Type> &types, int lineNr, int lastLine);
};

} // namespace studio
} // namespace gams

#endif // TEXTMARKREPO_H
