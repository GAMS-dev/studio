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
#include "file.h"
#include "exception.h"
#include "logger.h"

namespace gams {
namespace studio {

TextMarkList::TextMarkList(ProjectGroupNode* group, const QString& fileName)
    : QObject(), mGroupNode(group), mFileName(fileName)
{}

void TextMarkList::unbind()
{
    if (mFileNode) {
        mFileNode->unbindMarks();
        mFileNode = nullptr;
    }
}

void TextMarkList::bind(ProjectFileNode* fc)
{
    if (mFileNode)
        EXCEPT() << "TextMarks are already bound to FileNode " << mFileNode->location();
    mFileNode = fc;
    mGroupNode = fc->parentNode();
    mFileName = fc->location();
    if (fc->document() && fc->metrics().fileType().kind() == FileKind::Gms) connectDoc();
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

void TextMarkList::shareMarkHash(QHash<int, TextMark*>* marks, TextMark::Type filter)
{
    for (TextMark* mark: mMarks) {
        if ((mark->type() == filter) || filter == TextMark::all)
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
    if (mFileNode && mFileNode->metrics().fileType().kind() == FileKind::Gms)
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
            if (fileNode()) fileNode()->rehighlightAt(pos);
        } else if (compare > 0 && !mFileNode->isReadOnly()) {
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
    if (document()) fileNode()->rehighlightAt(res->position());
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

ProjectFileNode*TextMarkList::fileNode()
{
    if (mFileNode) return mFileNode;
    // TODO(JM) find fileNode in groupNode
    return mFileNode;
}

QTextDocument*TextMarkList::document() const
{
    return mFileNode ? mFileNode->document() : nullptr;
}

ProjectFileNode* TextMarkList::openFileNode()
{
    if (!mFileNode) {
        DEB() << "Creating FileNode for missing " << mFileName;
        emit getFileNode(mFileName, mFileNode, mGroupNode);
        if (!mFileNode) EXCEPT() << "Error creating FileNode " << mFileName;
    }
    if (!mFileNode->document()) {
        emit mFileNode->openFileNode(mFileNode, true);
    }
    return mFileNode;
}

void TextMarkList::removeTextMarks(QSet<TextMark::Type> tmTypes, bool doRehighlight)
{
    int i = mMarks.size();
    while (i > 0) {
        --i;
        TextMark* tm = mMarks.at(i);
        if (tmTypes.isEmpty() || tmTypes.contains(tm->type()) || tmTypes.contains(TextMark::all)) {
            int pos = tm->position();
            TextMark* mark = mMarks.takeAt(i);
            delete mark;
            if (doRehighlight && fileNode() && fileNode()->document())
                fileNode()->rehighlightAt(pos);
        }
    }
}

void TextMarkList::removeTextMark(TextMark* mark)
{
    mMarks.removeAll(mark);
}

QVector<TextMark*> TextMarkList::findMarks(const QTextCursor& cursor)
{
    QVector<TextMark*> res;
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
    if (mFileNode && mFileNode->document() && !mFileNode->isReadOnly()) {
        connect(mFileNode->document(), &QTextDocument::contentsChange, this, &TextMarkList::documentChanged);
        updateMarks();
    }
}

QVector<TextMark*> TextMarkList::marksForBlock(QTextBlock block, TextMark::Type refType)
{
    QVector<TextMark*> marks;
    int i = block.blockNumber()+2 < block.document()->blockCount() ? 0 : qMax(mMarks.size()-4, 0);
    for (; i < mMarks.size(); i++) {
        TextMark* tm = mMarks.at(i);
        int hit = tm->in(block.position(), block.length()-1);
        if (hit == 0 && (refType == TextMark::all || refType == tm->refType())) {
            marks << tm;
        }
    }
    return marks;
}


} // namespace source
} // namespace gams
