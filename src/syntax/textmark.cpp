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
#include "textmark.h"
#include "textmarkrepo.h"
#include "file.h"
#include "logger.h"
#include "exception.h"

namespace gams {
namespace studio {

TextMarkId TextMark::mNextId = 0;

TextMark::TextMark(TextMarkRepo *marks, FileId fileId, Type tmType, NodeId groupId)
    : mId(mNextId++), mFileId(fileId), mGroupId(groupId), mMarkRepo(marks), mType(tmType)
{
    if (!mMarkRepo) FATAL() << "The TextMarkRepo must be a valid instance.";
}

void TextMark::setLine(int lineNr)
{
    mLine = lineNr;
}

TextMark *TextMark::refMark() const
{
    return mReference;
}

QVector<TextMark *> TextMark::backRefs(FileId fileId) const
{
    if (!fileId.isValid()) return mBackRefs;
    QVector<TextMark *> res;
    for (TextMark* mark: mBackRefs) {
        if (mark->fileId() == fileId) res << mark;
    }
    return res;
}

TextMark::~TextMark()
{
    clearBackRefs();
}

void TextMark::setPosition(int line, int column, int size)
{
    mLine = line;
    mSize = (size<0) ? -size : size;
    mColumn = (size<0) ? column-mSize : column;
}

void TextMark::jumpToRefMark(bool focus, bool ignoreColumn)
{
    if (mReference)
        mReference->jumpToMark(focus, ignoreColumn);
    else
        DEB() << "No TextMark reference to jump to";
}

void TextMark::jumpToMark(bool focus, bool ignoreColumn)
{
    mMarkRepo->jumpTo(this, focus, ignoreColumn);
}

void TextMark::setRefMark(TextMark* refMark)
{
    mReference = refMark;
    if (mReference)
        mReference->mBackRefs << this;
}

void TextMark::unsetRefMark(TextMark* refMark)
{
    if (mReference == refMark) mReference = nullptr;
    mBackRefs.removeAll(refMark);
}

void TextMark::clearBackRefs()
{
    if (mReference) mReference->unsetRefMark(this);
    for (TextMark* backRef: mBackRefs) {
        backRef->unsetRefMark(this);
    }
    mBackRefs.clear();
    if (mBlockData) mBlockData->removeTextMark(this);
    mBlockData = nullptr;
}

void TextMark::setBlockData(BlockData *blockData)
{
    // recent value of mBlockData is commonly invalid
    mBlockData = blockData;
    if (mBlockData) mBlockData->addTextMark(this);
}

QColor TextMark::color()
{
    if (mReference) {
        if (mReference->type() == TextMark::error) return Qt::darkRed;
        if (mReference->fileKind() == FileKind::Lst) return Qt::blue;
    } else {
        return Qt::darkRed;
    }
    return Qt::darkGreen;
}

FileKind TextMark::fileKind()
{
    return mMarkRepo->fileKind(mFileId);
}

FileKind TextMark::refFileKind()
{
    return mReference ? mReference->fileKind() : FileKind::None;
}

QIcon TextMark::icon()
{
    switch (mType) { // TODO(JM) hold ref to TextMark instead of icon
    case error:
        return QIcon(":/img/exclam-circle-r");
    case link:
        return mReference ? QIcon(":/img/err-ref") : QIcon(":/img/err-ref-missing");
    case bookmark: {
        QIcon ico(":/img/bookmark");
        // TODO(JM) insert bookmark-number from value (0-9)
        return ico;
    }
    default:
        break;
    }
    return QIcon();
}

inline TextMark::Type TextMark::refType() const
{
    return (mReference) ? mReference->type() : none;
}

Qt::CursorShape& TextMark::cursorShape(Qt::CursorShape* shape, bool inIconRegion)
{
    if (shape && ((mType == error && inIconRegion) || mType == link))
        *shape = mReference ? Qt::PointingHandCursor : Qt::ForbiddenCursor;
    return *shape;
}

//QTextBlock TextMark::textBlock()
//{
//    if (!document())
//        return QTextBlock();
//    return document()->findBlock(qMin(mPosition, document()->characterCount()-1));
//}

//QTextCursor TextMark::textCursor()
//{
//    if (!document())
//        return QTextCursor();
//    QTextCursor cursor(document());
//    int pos = qMin(mPosition+mSize, document()->characterCount()-1);
//    cursor.setPosition(pos);
//    return cursor;
//}

void TextMark::rehighlight()
{
    mMarkRepo->rehighlight(mFileId, mLine);
}

//void TextMark::move(int delta)
//{
//    if (mPosition < 0)
//        EXCEPT() << "Can't move an uninitialized position";

//    mPosition += delta;
//    updateLineCol();
//    mMarks->rehighlight(mFileId, qMin(mPosition-delta+1, document()->characterCount()-1));
//    rehighlight();
//}

//void TextMark::updatePos()
//{
//    if (document()) {
//        QTextBlock block = document()->findBlockByNumber(mLine);
//        if (block.blockNumber() != mLine) block = document()->lastBlock();
//        int col = (mColumn>=0 ? mColumn : 0);
//        mPosition = block.position() + col;
//        if (mSize <= 0) {
//            mSize = block.next().text().indexOf('$')+1;
//            if (mSize <= 0) mSize = block.length()-col-1;
//        } else {
//            QString str = block.text();
//            for (int i = col; i < qMin(col+mSize, str.length()); ++i)
//                if (str.at(i)=='\t') mSize -= (7 - i%8);
//        }
//    }
//}

//void TextMark::updateLineCol()
//{
//    if (document()) {
//        QTextCursor cursor(document());
//        cursor.setPosition(qMin(mPosition, document()->characterCount()-1));
//        mLine = cursor.blockNumber();
//        if (mColumn >= 0) mColumn = cursor.positionInBlock();
//    }
//}

void TextMark::flatten()
{
    mSize = 0;
    mColumn = -1;
    rehighlight();
}

QString TextMark::dump()
{
    QStringList refs;
    for (TextMark* mark: mBackRefs) {
        refs << QString::number(mark->mId);
    }
    return QString("(%3,%4,%5)[%1%2] ").arg(mId)
            .arg(mReference ? "->"+QString::number(mReference->mId) : "")
            .arg(mLine).arg(mColumn).arg(mSize);
}

FileId TextMark::fileId() const
{
    return mFileId;
}

NodeId TextMark::groupId() const
{
    return mGroupId;
}

int TextMark::value() const
{
    return mValue;
}

void TextMark::setValue(int value)
{
    mValue = value;
}

} // namespace studio
} // namespace gams
