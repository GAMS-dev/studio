/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef TEXTMARK_H
#define TEXTMARK_H

#include <QTextDocument>
#include <QVector>
#include "file/filetype.h"
#include "common.h"

namespace gams {
namespace studio {

class TextMarkRepo;
struct TextMarkData;
class BlockData;

class TextMark
{
public:
    enum Type {none, error, link, bookmark, match, all};

    virtual ~TextMark();
    FileId fileId() const;
    FileId runId() const;
    QTextDocument* document() const;
    void setPosition(int line, int column, int size = 0);
    void jumpToRefMark(bool focus = true);
    void jumpToMark(bool focus = true);
    void setRefMark(TextMark* refMark);
    void unsetRefMark(TextMark* refMark);
    inline bool isErrorRef();
    QColor color();
    FileKind fileKind();
    FileKind refFileKind();
    int value() const;
    void setValue(int value);

    void clearBackRefs();
    void setBlockData(BlockData* blockData);

    QIcon icon();
    inline Type type() const {return mType;}
    inline Type refType() const;
    Qt::CursorShape& cursorShape(Qt::CursorShape* shape, bool inIconRegion = false);
    inline bool isValid() {return mMarkRepo && (mLine>=0) && (mColumn>=0);}
    inline bool isValidLink(bool inIconRegion = false)
    { return mReference && ((mType == error && inIconRegion) || mType == link); }
//    QTextBlock textBlock();
//    QTextCursor textCursor();
//    inline int in(int pos, int len) {
//        if (mPosition < 0) return -2;
//        return (mPosition+mSize <= pos) ? -1 : (mPosition >= pos+len) ? 1 : 0;
//    }

    inline int line() const {return document() ? qMin(mLine,document()->blockCount()-1) : mLine;}
    inline int column() const {return mColumn;}
    inline int size() const {return mSize;}
    inline bool inColumn(int col) const {return !mSize || (col >= mColumn && col < (mColumn+mSize));}
//    inline int position() const {return document() ? qMin(mPosition,document()->characterCount()-1) : mPosition;}
    inline int blockStart() const {return mColumn;}
    inline int blockEnd() const {return mColumn+mSize;}
//    inline void incSpread() {mSpread++;}
//    inline int spread() const {return mSpread;}
    void rehighlight();

//    void move(int delta);
//    void updatePos();
//    void updateLineCol();
    void flatten();

    QString dump();

protected:
    friend class TextMarkRepo;
    TextMark(TextMarkRepo* marks, FileId fileId, TextMark::Type tmType, FileId runId = -1);

private:
    static TextMarkId mNextId;
    TextMarkId mId;
    FileId mFileId;
    FileId mRunId;
    TextMarkRepo* mMarkRepo = nullptr;
//    int mPosition = -1;
    Type mType = none;
    int mLine = -1;
    int mColumn = 0;
    int mSize = 0;
    int mValue = 0;
//    int mSpread = 0;
    TextMark* mReference = nullptr;
//    TextMarkData* mRefData = nullptr;
    QVector<TextMark*> mBackRefs;

    // TODO(JM) maybe later we want to link the mark to the QTextBlock
    BlockData* mBlockData = nullptr;
};

struct TextMarkData
{
    TextMarkData(const QString& _location, TextMark::Type _type, int _line, int _column, int _size = 0)
        : location(_location), runLocation(QString()), type(_type), line(_line), column(_column), size(_size) {}
    TextMarkData(const QString& _location, const QString &_runLocation, TextMark::Type _type, int _line, int _column, int _size = 0)
        : location(_location), runLocation(_runLocation), type(_type), line(_line), column(_column), size(_size) {}
    QString location;
    QString runLocation;
    TextMark::Type type;
    int line;
    int column;
    int size;
    FileKind fileKind() {
        return FileType::from(location.right(4).toLower()).kind();
    }
};

} // namespace studio
} // namespace gams

#endif // TEXTMARK_H
