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
#include "syntaxformats.h"
#include "syntaxreserved.h"
#include "syntaxdeclaration.h"
#include "syntaxidentifier.h"
#include "textmark.h"
#include "textmarklist.h"
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
    initState(new SyntaxStandard());
    initState(new SyntaxDirective(), Qt::darkMagenta);
    initState(new SyntaxDirectiveBody(SyntaxState::DirectiveBody), Qt::darkBlue);
    initState(new SyntaxDirectiveBody(SyntaxState::DirectiveComment), Qt::darkGreen, true, false);
    initState(new SyntaxDirectiveBody(SyntaxState::Title), Qt::darkBlue, true, true);
    initState(new SyntaxCommentLine(), Qt::darkGreen, true, false);
    initState(new SyntaxCommentBlock(), Qt::darkGreen, true, false);
    initState(new SyntaxDeclaration(), Qt::darkBlue, false, true);
    initState(new SyntaxPreDeclaration(SyntaxState::DeclarationSetType), Qt::darkBlue, false, true);
    initState(new SyntaxPreDeclaration(SyntaxState::DeclarationVariableType), Qt::darkBlue, false, true);
    initState(new SyntaxDeclarationTable(), Qt::darkBlue, false, true);
    initState(new SyntaxIdentifier(SyntaxState::Identifier), QColor(Qt::cyan).darker(), false, true, true);

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
    if (code < 0) code = 0;
    int index = 0;
    QTextBlock textBlock = currentBlock();
    int posForSyntaxState = mPositionForSyntaxState - textBlock.position();
    if (posForSyntaxState < 0) posForSyntaxState = text.length();

    while (index < text.length()) {
        QString debString = QString("  %1").arg(textBlock.blockNumber()).right(3) + ", " + QString(" %1").arg(index).right(2) + "-";
        StateCode stateCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
        SyntaxAbstract* syntax = mStates.at(stateCode.first);

         // detect end of valid trailing characters for current syntax
        SyntaxBlock tailBlock = syntax->validTail(text, index);

        // TODO(JM) For states redefined with directives:
        //   - add new Syntax to mStates
        //   - create a new full set of Syntax in mCodes with just the new one replaced
        // -> result: the top code will change from 0 to the new Standard top
        SyntaxBlock nextBlock;
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
        if (!nextBlock.isValid()) {
            if (!tailBlock.isValid()) {
                // no valid characters found, mark error
                setCombiFormat(index, text.length() - index, syntax->charFormatError(), markList);
                index = text.length();
                code = getCode(code, SyntaxStateShift::reset, stateCode.first, stateCode.first);
                continue;
            }
            nextBlock = tailBlock;
        } else if (tailBlock.isValid()) {
            if (nextBlock.start < tailBlock.end) tailBlock.end = nextBlock.start;
            if (tailBlock.isValid()) {
                if (tailBlock.syntax->state() != SyntaxState::Standard)
                    setCombiFormat(tailBlock.start, tailBlock.length(), tailBlock.syntax->charFormat(), markList);
                code = getCode(code, tailBlock.shift, getStateIdx(tailBlock.syntax->state()), getStateIdx(tailBlock.next));
            }
        }

        if (nextBlock.error && nextBlock.length() > 0)
            setCombiFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormatError(), markList);
        else if (nextBlock.syntax->state() != SyntaxState::Standard)
            setCombiFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormat(), markList);
        index = nextBlock.end;

        debString += QString(" %1").arg(index).right(2) + ": " + codeDeb(code);
        code = getCode(code, nextBlock.shift, getStateIdx(nextBlock.syntax->state()), getStateIdx(nextBlock.next));
        DEB() << debString << "   (" << codeDeb(code) << ")";

        if (posForSyntaxState <= index) {
            mLastSyntaxState = nextBlock.syntax->intSyntaxType();
            mPositionForSyntaxState = -1;
            posForSyntaxState = text.length()+1;
        }
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

QColor nextColor() {
    static int debIndex = -1;
    static QList<QColor> debColor {Qt::yellow, Qt::cyan, QColor(Qt::blue).lighter(),
                                  QColor(Qt::green).lighter()};
    debIndex = (debIndex+1) % debColor.size();
    return debColor.at(debIndex);
}

void SyntaxHighlighter::initState(SyntaxAbstract *syntax, QColor color, bool italic, bool bold, bool debug)
{
    if (debug) syntax->charFormat().setBackground(nextColor());

    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    if (color.isValid()) syntax->charFormat().setForeground(color);
    if (italic) syntax->charFormat().setFontItalic(true);
    if (bold) syntax->charFormat().setFontWeight(QFont::Bold);
    mStates << syntax;
    addCode(mStates.length()-1, 0);
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
    if (shift == SyntaxStateShift::shift)
        return addCode(state, mCodes.at(code).second);

    while (mStates.at(mCodes.at(code).first)->state() != SyntaxState::Standard) {
        code = mCodes.at(code).second;
    }
    return code;
}

QString SyntaxHighlighter::codeDeb(int code)
{
    QString res = syntaxStateName(mStates.at(mCodes.at(code).first)->state());
    while (code) {
        code = mCodes.at(code).second;
        res = syntaxStateName(mStates.at(mCodes.at(code).first)->state()) + ", " + res;
    }
    return res;
}


} // namespace studio
} // namespace gams
