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
#include "filegroupcontext.h"
#include "logger.h"
#include "exception.h"

namespace gams {
namespace studio {

int TextMark::mNextId = 0;

TextMark::TextMark(Type tmType): mId(mNextId++), mType(tmType)
{}

TextMark::~TextMark()
{}

void TextMark::ensureFileContext()
{
    if (!mFileContext && mGroup) {
        FileSystemContext *fsc = mGroup->findFile(mFileName);
        if (!fsc) {
            mGroup->attachFile(mFileName);
            fsc = mGroup->findFile(mFileName);
        }
        if (fsc && fsc->type() == FileSystemContext::File) {
            mFileContext = static_cast<FileContext*>(fsc);
            updateCursor();
        }
    } else if (!mFileContext) {
        EXCEPT() << "Invalid TextMark found: neither linked to FileContext nor FileName " << static_cast<void*>(this);
    }
}

void TextMark::unbindFileContext()
{
    if (mFileContext) {
        if (!mGroup) {
            mGroup = mFileContext->parentEntry();
            mFileName = mFileContext->location();
        }
        mFileContext = nullptr;
    }
}

void TextMark::setPosition(FileContext* fileContext, int line, int column, int size)
{
    if (!fileContext)
        EXCEPT() << "FileContext must not be null.";
    mFileContext = fileContext;
    mGroup = nullptr;
    mFileName = "";
    mLine = line;
    mSize = size;
    mColumn = column;
    updateCursor();
}

void TextMark::setPosition(QString fileName, FileGroupContext* group, int line, int column, int size)
{
    if (!group)
        EXCEPT() << "FileGroupContext must not be null.";
    mFileContext = nullptr;
    mGroup = group;
    mFileName = fileName;
    mLine = line;
    mSize = size;
    mColumn = column;
}

void TextMark::updateCursor()
{
    if (mFileContext && mFileContext->document()) {
        QTextBlock block = mFileContext->document()->findBlockByNumber(mLine);
        mCursor = QTextCursor(block);
        if (mSize <= 0) {
            int end = block.next().text().indexOf('$');
            if (end == 0) end = block.next().length();
            if (end < 0) end = 0;
            if (end > 0) mCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, end+1);
            mSize = qAbs(mCursor.selectionEnd()-mCursor.selectionStart());
        } else {
            QString str = block.text();
            for (int i = mColumn; i < mColumn+mSize; ++i)
                if (str.at(i)=='\t') mSize -= (7 - i%8);
            mCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, mColumn);
            mCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, mSize);
        }
    } else {
        mCursor = QTextCursor();
    }
}

void TextMark::jumpToRefMark(bool focus)
{
    if (mReference)
        mReference->jumpToMark(focus);
}

void TextMark::jumpToMark(bool focus)
{
    ensureFileContext();
    if (mFileContext) {
        if (mCursor.isNull()) {
            if (mFileContext->metrics().fileType() == FileType::Gdx)
                mFileContext->openFileContext(mFileContext, focus);
            else
                mFileContext->jumpTo(mCursor, focus, mLine, mColumn);
        } else {
            mFileContext->jumpTo(mCursor, focus);
        }
    }
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
    foreach (TextMark* backRef, mBackRefs) {
        backRef->unsetRefMark(this);
    }
}

QColor TextMark::color()
{
    if (type() == TextMark::result)
        return Qt::yellow;

    if (!mReference) return Qt::white;
    if (mReference->type() == TextMark::error)
        return Qt::darkRed;
    if (mReference->fileKind() == FileType::Lst)
        return Qt::blue;

    return Qt::darkGreen;
}

FileType::Kind TextMark::fileKind()
{
    return (!mFileContext) ? FileType::None : mFileContext->metrics().fileType().kind();
}

FileType::Kind TextMark::refFileKind()
{
    return !mReference ? FileType::None : mReference->fileKind();
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

Qt::CursorShape& TextMark::cursorShape(Qt::CursorShape* shape, bool inIconRegion)
{
    if (shape && ((mType == error && inIconRegion) || mType == link))
        *shape = mReference ? Qt::PointingHandCursor : Qt::ForbiddenCursor;
    return *shape;
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

void TextMark::rehighlight()
{
    if (mFileContext) mFileContext->rehighlightAt(position());
}

void TextMark::modified()
{
    mSize = 0;
    mColumn = 0;
    mCursor.setPosition(mCursor.position());
    rehighlight();
}

QString TextMark::dump()
{
    QStringList refs;
    for (TextMark* mark: mBackRefs) {
        refs << QString::number(mark->mId);
    }
    return QString("[%1->%2]  %3").arg(mId).arg(mReference?QString::number(mReference->mId):"#").arg(refs.join(", "));
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
