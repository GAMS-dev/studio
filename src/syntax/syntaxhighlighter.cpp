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
#include "syntaxhighlighter.h"
#include "logger.h"
#include "exception.h"
#include "filecontext.h"
#include "tool.h"

namespace gams {
namespace studio {

ErrorHighlighter::ErrorHighlighter(FileContext* context)
    : QSyntaxHighlighter(context->document()), mContext(context)
{
}

void ErrorHighlighter::setDocAndConnect(QTextDocument* doc)
{
    setDocument(doc);
}

TextMarkList* ErrorHighlighter::marks()
{
    return (mContext) ? mContext->marks() : nullptr;
}

void ErrorHighlighter::syntaxState(int position, int &intState)
{
    mPositionForSyntaxState = position;
    mLastSyntaxState = 0;
    rehighlightBlock(document()->findBlock(position));
    intState = mLastSyntaxState;
    mLastSyntaxState = 0;
}

void ErrorHighlighter::highlightBlock(const QString& text)
{
    if (!marks()) {
        DEB() << "trying to highlight without marks!";
        return;
    }
    QList<TextMark*> markList = marks()->marksForBlock(currentBlock());
    setCombiFormat(0, text.length(), QTextCharFormat(), markList);
}

void ErrorHighlighter::setCombiFormat(int start, int len, const QTextCharFormat &charFormat, QList<TextMark*> markList)
{
    int end = start+len;
    int marksStart = end;
    int marksEnd = start;
    for (TextMark* mark: markList) {
        if (mark->blockStart() < marksStart) marksStart = qMax(start, mark->blockStart());
        if (mark->blockEnd() > marksEnd) marksEnd = qMin(end, mark->blockEnd());
    }
    if (marksStart == end) {
        setFormat(start, len, charFormat);
        return;
    }
    if (marksStart > start) setFormat(start, marksStart-start, charFormat);
    if (marksEnd < end) setFormat(marksEnd, end-marksEnd, charFormat);
    start = marksStart;
    end = marksEnd;

    for (TextMark* mark: markList) {
        if (mark->blockStart() >= end || mark->blockEnd() < start)
            continue;
        QTextCharFormat combinedFormat(charFormat);
        marksStart = qMax(mark->blockStart(), start);
        marksEnd = qMin(mark->blockEnd(), end);
        if (mark->type() == TextMark::error) {
            combinedFormat.setUnderlineColor(Qt::red);
            combinedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            combinedFormat.setAnchorName(QString::number(mark->line()));
            setFormat(marksStart, marksEnd-marksStart, combinedFormat);
            if (marksEnd == mark->blockEnd()) {
                combinedFormat.setBackground(QColor(225,200,255));
                combinedFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                setFormat(marksEnd, 1, combinedFormat);
            }
        }
        if (mark->type() == TextMark::link) {
            combinedFormat.setForeground(mark->color());
            combinedFormat.setUnderlineColor(mark->color());
            combinedFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            combinedFormat.setAnchor(true);
            combinedFormat.setAnchorName(QString::number(mark->line()));
            setFormat(marksStart, marksEnd-marksStart, combinedFormat);
        }
        if (mark->type() == TextMark::match) {
            combinedFormat.setBackground(mark->color());
            setFormat(marksStart, marksEnd - marksStart, combinedFormat);
        }
    }
}


SyntaxHighlighter::SyntaxHighlighter(FileContext* context)
    : ErrorHighlighter(context)
{
    SyntaxAbstract* syntax = new SyntaxStandard();
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);

    syntax = new SyntaxDirective();
    syntax->charFormat().setForeground(Qt::darkMagenta);
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);

    syntax = new SyntaxDirectiveBody(SyntaxState::DirectiveBody);
    syntax->charFormat().setForeground(Qt::darkBlue);
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);

    syntax = new SyntaxDirectiveBody(SyntaxState::DirectiveComment);
    syntax->charFormat().setForeground(Qt::darkGreen);
    syntax->charFormat().setFontItalic(true);
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);

    syntax = new SyntaxDirectiveBody(SyntaxState::Title);
    syntax->charFormat().setForeground(Qt::darkBlue);
    syntax->charFormat().setFontWeight(QFont::Bold);
    syntax->charFormat().setFontItalic(true);
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);

    syntax = new SyntaxCommentLine();
    syntax->charFormat().setForeground(Qt::darkGreen);
    syntax->charFormat().setFontItalic(true);
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);

    syntax = new SyntaxCommentBlock();
    syntax->copyCharFormat(mStates.last()->charFormat());
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);

    syntax = new SyntaxDeclaration(SyntaxState::Declaration);
    syntax->charFormat().setForeground(Qt::darkBlue);
    syntax->charFormat().setFontWeight(QFont::Bold);
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);
    SyntaxAbstract *sd_dec = syntax;

    syntax = new SyntaxDeclaration(SyntaxState::DeclarationSetType);
    syntax->copyCharFormat(mStates.last()->charFormat());
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);
    SyntaxAbstract *sd_decS = syntax;

    syntax = new SyntaxDeclaration(SyntaxState::DeclarationVariableType);
    syntax->copyCharFormat(mStates.last()->charFormat());
    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    addState(syntax);
    SyntaxAbstract *sd_decV = syntax;

    // TODO(JM) just for debugging
    sd_dec->charFormat().setBackground(Qt::cyan);
    sd_decS->charFormat().setBackground(Qt::yellow);
    sd_decV->charFormat().setBackground(Qt::yellow);
}

SyntaxHighlighter::~SyntaxHighlighter()
{
    while (!mStates.isEmpty()) {
        delete mStates.takeFirst();
    }
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    ErrorHighlighter::highlightBlock(text);
    QList<TextMark*> markList = marks() ? marks()->marksForBlock(currentBlock()) : QList<TextMark*>();
    int code = previousBlockState();
    int index = 0;
    int posForSyntaxState = mPositionForSyntaxState - currentBlock().position();
    if (posForSyntaxState < 0) posForSyntaxState = text.length();

    while (index < text.length()) {
        StateCode stateCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
        SyntaxAbstract* syntax = mStates.at(stateCode.first);

        SyntaxBlock thisBlock = syntax->find(syntax->state(), text, index);
        if (!thisBlock.isValid()) {
            DEB() << "Syntax not found for type " << int(syntax->state());
            index++;
            continue;
        }
        SyntaxBlock nextBlock;
            // TODO(JM) store somehow when states have been redefined with directives
            // (e.g. StdState with indexes to changed subsetStates)
            // other approach: add a maxFactor to code whenever a directive changed the syntax
        for (SyntaxState nextState: syntax->nextStates()) {
            SyntaxAbstract* testSyntax = getSyntax(nextState);
            if (testSyntax) {
                SyntaxBlock testBlock = testSyntax->find(syntax->state(), text, index);
                if (testBlock.isValid()) {
                    if (!nextBlock.isValid() || nextBlock.start > testBlock.start) {
                        nextBlock = testBlock;

                    }
                }
            }
        }
        if (nextBlock.isValid()) {
            // new state inside current block -> shorten end
            /*if (nextBlock.start < thisBlock.end) */thisBlock.end = nextBlock.start;
            // current block has zero size
            if (thisBlock.length() < 1) thisBlock = nextBlock;
        }
        if (thisBlock.isValid() && thisBlock.length() > 0) {
            if (thisBlock.syntax->state() != SyntaxState::Standard)
                setCombiFormat(thisBlock.start, thisBlock.length(), thisBlock.syntax->charFormat(), markList);

            if (thisBlock.error && thisBlock.length() > 0)
                setCombiFormat(thisBlock.start, thisBlock.length(), thisBlock.syntax->charFormatError(), markList);

            index = thisBlock.end;

            code = getCode(code, thisBlock.shift, getStateIdx(thisBlock.syntax->state()), getStateIdx(thisBlock.next));
            if (posForSyntaxState <= index) {
                mLastSyntaxState = thisBlock.syntax->intSyntaxType();
                mPositionForSyntaxState = -1;
                posForSyntaxState = text.length()+1;
            }
        } else {
            index++;
        }
//        if (thisBlock.isValid()) DEB() << "Line " << currentBlock().blockNumber()+1
//                                       << " Syntax " << syntaxStateName(thisBlock.syntax->state());
    }
    StateCode stateCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
    if (mStates.at(stateCode.first)->state() != SyntaxState::Standard) {
        setCurrentBlockState(code);
    } else if (currentBlockState() != -1) {
        setCurrentBlockState(-1);
    }
}

SyntaxAbstract*SyntaxHighlighter::getSyntax(SyntaxState state) const
{
    int i = mStates.length();
    while (i > 0) {
        --i;
        if (mStates.at(i)->state() == state) return mStates.at(i);
    }
    return nullptr;
}

int SyntaxHighlighter::getStateIdx(SyntaxState state) const
{
    int i = mStates.length();
    while (i > 0) {
        --i;
        if (mStates.at(i)->state() == state) return i;
    }
    return -1;
}

void SyntaxHighlighter::addState(SyntaxAbstract* syntax, CodeIndex ci)
{
    mStates << syntax;
    addCode(mStates.length()-1, ci);
}

int SyntaxHighlighter::addCode(StateIndex si, CodeIndex ci)
{
    StateCode sc(si, ci);
    if (si < 0)
        EXCEPT() << "Can't generate code for invalid StateIndex";
    int index = mCodes.indexOf(sc);
    if (index >= 0)
        return index;
    mCodes << sc;
    return mCodes.length()-1;
}

int SyntaxHighlighter::getCode(CodeIndex code, SyntaxStateShift shift, StateIndex state, StateIndex stateNext)
{
    if (code < 0) code = 0;
    if (shift == SyntaxStateShift::skip)
        return code;
    if (shift == SyntaxStateShift::out)
        return mCodes.at(code).second;
    if (shift == SyntaxStateShift::in)
        return addCode(stateNext, code);
    if (shift == SyntaxStateShift::stay)
        return addCode(state, mCodes.at(code).second);

    while (mStates.at(mCodes.at(code).first)->state() != SyntaxState::Standard) {
        code = mCodes.at(code).second;
    }
    return code;
}


} // namespace studio
} // namespace gams
