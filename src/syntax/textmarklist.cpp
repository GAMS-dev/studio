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
#include "textmarklist.h"
#include "filecontext.h"
#include "filegroupcontext.h"
#include "exception.h"
#include "logger.h"

namespace gams {
namespace studio {

TextMarkList::TextMarkList(FileGroupContext* group, const QString& fileName)
    : QObject(), mGroupContext(group), mFileName(fileName)
{}

void TextMarkList::unbind()
{
    if (mFileContext) {
        mFileContext->unbindMarks();
        mFileContext = nullptr;
    }
}

void TextMarkList::bind(FileContext* fc)
{
    if (mFileContext)
        EXCEPT() << "TextMarks are already bound to FileContext " << mFileContext->location();
    mFileContext = fc;
    mGroupContext = fc->parentEntry();
    mFileName = fc->location();
    if (fc->document() && fc->metrics().fileType().kind() == FileType::Gms) connectDoc();
}

void TextMarkList::updateMarks()
{
    for (TextMark* mark: mMarks) {
        mark->updatePos();
        mark->rehighlight();
    }
}

void TextMarkList::rehighlight()
{
    for (TextMark* mark: mMarks) {
        mark->rehighlight();
    }
}

void TextMarkList::shareMarkHash(QHash<int, TextMark*>* marks)
{
    for (TextMark* mark: mMarks) {
        marks->insert(mark->line(), mark);
    }
}

void TextMarkList::textMarkIconsEmpty(bool* noIcons)
{
    int tms = textMarkCount(QSet<TextMark::Type>() << TextMark::error << TextMark::link << TextMark::bookmark);
    *noIcons = !(tms > 0);
}

void TextMarkList::documentOpened()
{
    if (mFileContext && mFileContext->metrics().fileType().kind() == FileType::Gms)
        connectDoc();
}

void TextMarkList::documentChanged(int pos, int charsRemoved, int charsAdded)
{
    int i = mMarks.size()-1;
    while (i >= 0) {
        TextMark* mark = mMarks.at(i);
        int compare = mark->in(pos, charsRemoved);
        if (!compare) {
            int pos = mark->position();
            if (mark->type() == TextMark::error || mark->type() == TextMark::link) mark->flatten();
            else mMarks.removeAt(i);
            if (fileContext()) fileContext()->rehighlightAt(pos);
        } else if (compare > 0 && !mFileContext->isReadOnly()) {
            mark->move(charsAdded-charsRemoved);
        }
        i--;
    }
}

TextMark*TextMarkList::generateTextMark(TextMark::Type tmType, int value, int line, int column, int size)
{
    TextMark* res = new TextMark(this, tmType);
    res->setPosition(line, column, size);
    res->setValue(value);
    mMarks << res;
    if (document()) fileContext()->rehighlightAt(res->position());
    return res;
}

int TextMarkList::textMarkCount(QSet<TextMark::Type> tmTypes)
{
    int i = mMarks.size();
    int res = 0;
    while (i > 0) {
        --i;
        TextMark* tm = mMarks.at(i);
        if (tmTypes.contains(tm->type()) || tmTypes.contains(TextMark::all)) {
            res++;
        }
    }

    return res;
}

FileContext*TextMarkList::fileContext()
{
    if (mFileContext) return mFileContext;
    // TODO(JM) find file-context in group
    return mFileContext;
}

QTextDocument*TextMarkList::document() const
{
    return mFileContext ? mFileContext->document() : nullptr;
}

FileContext* TextMarkList::openFileContext()
{
    if (!mFileContext) {
        DEB() << "Creating FileContext for missing " << mFileName;
        emit getFileContext(mFileName, mFileContext, mGroupContext);
        if (!mFileContext) EXCEPT() << "Error creating FileContext " << mFileName;
    }
    if (!mFileContext->document()) {
        emit mFileContext->openFileContext(mFileContext, true);
    }
    return mFileContext;
}

void TextMarkList::removeTextMarks(QSet<TextMark::Type> tmTypes)
{
    int i = mMarks.size();
    while (i > 0) {
        --i;
        TextMark* tm = mMarks.at(i);
        if (tmTypes.isEmpty() || tmTypes.contains(tm->type()) || tmTypes.contains(TextMark::all)) {
            int pos = tm->position();
            TextMark* mark = mMarks.takeAt(i);
            mark->clearBackRefs();
            // TODO(JM) Somehow this cannot be deleted, as if it's already done
            delete mark;
            if (fileContext() && fileContext()->document()) fileContext()->rehighlightAt(pos);
        }
    }
}

void TextMarkList::removeTextMark(TextMark* mark)
{
    mMarks.removeAll(mark);
}

QList<TextMark*> TextMarkList::findMarks(const QTextCursor& cursor)
{
    QList<TextMark*> res;
    for (TextMark* mark: mMarks) {
        QTextCursor tc = mark->textCursor();
        if (tc.isNull()) break;
        if (tc.blockNumber() > cursor.blockNumber()) break;
        if (tc.blockNumber() < cursor.blockNumber()) continue;
        if (cursor.atBlockStart())
            res << mark;

        int a = tc.block().position() + mark->column();
        int b = a + (mark->size() ? mark->size() : tc.block().length());
        if (cursor.position() >= b) continue;
        if (cursor.position() >= a && (cursor.selectionEnd() < b))
            res << mark;
    }
    return res;
}

TextMark*TextMarkList::firstErrorMark()
{
    for (TextMark* mark: mMarks)
        if (mark->isErrorRef()) return mark;
    return nullptr;
}

void TextMarkList::connectDoc()
{
    if (mFileContext && mFileContext->document() && !mFileContext->isReadOnly()) {
        connect(mFileContext->document(), &QTextDocument::contentsChange, this, &TextMarkList::documentChanged);
        updateMarks();
    }
}

QList<TextMark*> TextMarkList::marksForBlock(QTextBlock block, TextMark::Type refType)
{
    QList<TextMark*> marks;
    for (TextMark* tm: mMarks) {
        int hit = tm->in(block.position(), block.length()-1);
        if (hit == 0 && (refType == TextMark::all || refType == tm->refType())) {
            marks << tm;
        }
    }
//    if (marks.size() && mFileName.isEmpty()) {
//        QString res;
//        for (TextMark *mark: marks) {
//            res += "  " + mark->dump();
//        }
//        DEB() << "at " << block.position() << ":  line " << block.blockNumber() << "(" << block.length() << ")  -- MARKS FOR LOG: " << res;
//    }
    return marks;
}


} // namespace source
} // namespace gams
