/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "filecontext.h"
#include "logger.h"

namespace gams {
namespace studio {

TextMark::TextMark(Type tmType): mType(tmType)
{

}

void TextMark::mark(FileContext* fileContext, int line, int column, int size)
{
    mFileContext = fileContext;
    mLine = line;
    mSize = size;
    mColumn = column;
    updateCursor();
}

void TextMark::updateCursor()
{
    if (mFileContext && mFileContext->document()) {
        QTextBlock block = mFileContext->document()->findBlockByNumber(mLine);
        mCursor = QTextCursor(block);
        if (mSize <= 0) {
//            mCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            mCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        } else {
            mCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, mColumn);
            mCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, mSize);
        }
    } else {
        mCursor = QTextCursor();
    }
}

void TextMark::setRefMark(TextMark* refMark)
{
    mReference = refMark;
}

void TextMark::jumpToRefMark()
{
    if (mReference)
        mReference->jumpToMark();
}

void TextMark::jumpToMark()
{
    if (mFileContext && !mCursor.isNull())
        mFileContext->jumpTo(mCursor);
}

void TextMark::showToolTip()
{
    if (mFileContext)
        mFileContext->showToolTip(*this);
}

QIcon TextMark::icon()
{
    switch (mType) { // TODO(JM) hold ref to TextMark instead of icon
    case error:
        return QIcon(":/img/exclam-circle-r");
        break;
    case link:
        return mReference ? QIcon(":/img/err-ref") : QIcon(":/img/err-ref-missing");
        break;
    case bookmark: {
        QIcon ico(":/img/bookmark");
        // TODO(JM) insert bookmark-number from value (0-9)
        return ico;
        break;
    }
    default:
        break;
    }
    return QIcon();
}

TextMark::Type TextMark::type() const
{
    return mType;
}

bool TextMark::isValid()
{
    return mFileContext && (mLine>=0) && (mColumn>=0);
}

int TextMark::line() const
{
    if (mCursor.isNull())
        return mLine;
    return mCursor.block().userState();
}

QTextBlock TextMark::textBlock()
{
    if (mCursor.isNull())
        return QTextBlock();
    return mCursor.block();
}

QTextCursor TextMark::textCursor() const
{
    return mCursor;
}

int TextMark::column() const
{
    return mColumn;
}

int TextMark::size() const
{
    return mSize;
}

bool TextMark::inColumn(int col) const
{
    return !mSize || (col >= mColumn && col < (mColumn+mSize));
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
