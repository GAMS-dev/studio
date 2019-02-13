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
#include "syntaxdeclaration.h"
#include "syntaxidentifier.h"
#include "textmark.h"
#include "logger.h"
#include "exception.h"
#include "file.h"
#include "common.h"

namespace gams {
namespace studio {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc)
    : ErrorHighlighter(doc)
{
    QHash<ColorEnum, QColor> cl {
        {SyntaxDirex, QColor(Qt::darkMagenta).darker(120)},
        {SyntaxDiBdy, QColor(Qt::darkBlue)},
        {SyntaxComnt, QColor(80, 145, 75)},
        {SyntaxTitle, QColor(Qt::darkBlue)},
        {SyntaxKeywd, QColor(Qt::darkBlue)},
        {SyntaxDeclr, QColor(Qt::darkBlue)},
        {SyntaxIdent, QColor(Qt::black)},
        {SyntaxDescr, QColor(Qt::darkBlue).lighter(120)},
        {SyntaxAssgn, QColor(Qt::darkGreen).darker(130)},
        {SyntaxAsLab, QColor(Qt::darkGreen).lighter(110)},
        {SyntaxAsVal, QColor(Qt::darkBlue).lighter(120)},
        {SyntaxTabHd, QColor(Qt::darkGreen).darker(145)},
        {SyntaxEmbed, QColor(200, 70, 0)},
    };
    // To visualize one format: add color index at start e.g. initState(1, new SyntaxReservedBody());
    initState(new SyntaxStandard(), Qt::red);
    initState(new SyntaxDirective(), cl.value(SyntaxDirex));
    initState(new SyntaxDirectiveBody(SyntaxState::DirectiveBody), cl.value(SyntaxDiBdy));
    initState(new SyntaxDirectiveBody(SyntaxState::DirectiveComment), cl.value(SyntaxComnt), fItalic);
    initState(new SyntaxDirectiveBody(SyntaxState::Title), cl.value(SyntaxTitle), fBoldItalic);
    initState(new SyntaxCommentLine(), cl.value(SyntaxComnt), fItalic);
    initState(new SyntaxCommentBlock(), cl.value(SyntaxComnt), fItalic);

    initState(new SyntaxDelimiter(SyntaxState::Semicolon));
    initState(new SyntaxDelimiter(SyntaxState::Comma));
    initState(new SyntaxReserved(), cl.value(SyntaxKeywd), fBold);
    initState(new SyntaxReservedBody());
    initState(new SyntaxEmbedded(SyntaxState::Embedded), cl.value(SyntaxKeywd), fBold);
    initState(new SyntaxEmbedded(SyntaxState::EmbeddedEnd), cl.value(SyntaxKeywd), fBold);
    initState(new SyntaxEmbeddedBody(), cl.value(SyntaxEmbed), fNormal);
    initState(new SyntaxPreDeclaration(SyntaxState::DeclarationSetType), cl.value(SyntaxDeclr), fBold);
    initState(new SyntaxPreDeclaration(SyntaxState::DeclarationVariableType), cl.value(SyntaxDeclr), fBold);
    initState(new SyntaxDeclaration(), cl.value(SyntaxDeclr), fBold);
    initState(new SyntaxDeclarationTable(), cl.value(SyntaxDeclr), fBold);

    initState(new SyntaxIdentifier(SyntaxState::Identifier));
    initState(new SyntaxIdentDescript(SyntaxState::IdentifierDescription1), cl.value(SyntaxDescr));
    initState(new SyntaxIdentDescript(SyntaxState::IdentifierDescription2), cl.value(SyntaxDescr));
    initState(new SyntaxIdentAssign(SyntaxState::IdentifierAssignment), cl.value(SyntaxAssgn));
    initState(new AssignmentLabel(), cl.value(SyntaxAsLab));
    initState(new AssignmentValue(), cl.value(SyntaxAsVal));
    initState(new SyntaxIdentAssign(SyntaxState::IdentifierAssignmentEnd), cl.value(SyntaxAssgn));

    initState(new SyntaxIdentifier(SyntaxState::IdentifierTable));
    initState(new SyntaxIdentDescript(SyntaxState::IdentifierTableDescription1), cl.value(SyntaxDescr));
    initState(new SyntaxIdentDescript(SyntaxState::IdentifierTableDescription2), cl.value(SyntaxDescr));

    initState(new SyntaxTableAssign(SyntaxState::IdentifierTableAssignmentHead), cl.value(SyntaxTabHd), fBold);
    initState(new SyntaxTableAssign(SyntaxState::IdentifierTableAssignmentRow), cl.value(SyntaxAssgn));

}

SyntaxHighlighter::~SyntaxHighlighter()
{
    while (!mStates.isEmpty()) {
        delete mStates.takeFirst();
    }
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    QVector<ParenthesesPos> parPosList;
    QList<TextMark*> markList = marks()->values(currentBlock().blockNumber());
    setCombiFormat(0, text.length(), QTextCharFormat(), markList);
    int code = previousBlockState();
    if (code < 0) code = 0;
    int index = 0;
    QTextBlock textBlock = currentBlock();
    int posForSyntaxState = mPositionForSyntaxState - textBlock.position();
    if (posForSyntaxState < 0) posForSyntaxState = text.length();
    bool extendSearch = true;

    while (index < text.length()) {
        StateCode stateCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
        SyntaxAbstract* syntax = mStates.at(stateCode.first);
        bool stack = true;
         // detect end of valid trailing characters for current syntax
        SyntaxBlock tailBlock = syntax->validTail(text, index, stack);
        if (stack) extendSearch = false;

        // TODO(JM) For states redefined with directives:
        //   - add new Syntax to mStates
        //   - create a new full set of Syntax in mCodes with just the new one replaced
        // -> result: the top code will change from 0 to the new Standard top
        SyntaxBlock nextBlock;
        for (SyntaxState nextState: syntax->nextStates(extendSearch)) {
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
        } else {
            extendSearch = false;
            if (tailBlock.isValid()) {
                if (nextBlock.start < tailBlock.end) tailBlock.end = nextBlock.start;
                if (tailBlock.isValid()) {
                    if (tailBlock.syntax->state() != SyntaxState::Standard) {
                        setCombiFormat(tailBlock.start, tailBlock.length(), tailBlock.syntax->charFormat(), markList);
                        scanParentheses(text, tailBlock.start, tailBlock.length(), tailBlock.syntax->state(), parPosList);
                    }
                    code = getCode(code, tailBlock.shift, getStateIdx(tailBlock.syntax->state()), getStateIdx(tailBlock.next));
                }
            }
        }

        if (nextBlock.error && nextBlock.length() > 0) {
            setCombiFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormatError(), markList);
        } else if (nextBlock.syntax->state() != SyntaxState::Standard) {
            setCombiFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormat(), markList);
            if (nextBlock.syntax->state() == SyntaxState::Semicolon) extendSearch = true;
        }
        scanParentheses(text, nextBlock.start, nextBlock.length(), nextBlock.syntax->state(), parPosList);
        index = nextBlock.end;

        code = getCode(code, nextBlock.shift, getStateIdx(nextBlock.syntax->state()), getStateIdx(nextBlock.next));

        if (posForSyntaxState <= index) {
            mLastSyntaxState = nextBlock.syntax->intSyntaxType();
            mPositionForSyntaxState = -1;
            posForSyntaxState = text.length()+1;
        }
    }
    // update BlockData
    if (!parPosList.isEmpty() || textBlock.userData()) {
        BlockData* blockData = textBlock.userData() ? static_cast<BlockData*>(textBlock.userData()) : nullptr;
        if (!parPosList.isEmpty() && !blockData) {
            blockData = new BlockData();
        }
        if (blockData) blockData->setParentheses(parPosList);
        if (blockData && blockData->isEmpty())
            textBlock.setUserData(nullptr);
        else
            textBlock.setUserData(blockData);
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

const QVector<SyntaxState> validparenthesesSyntax = {
    SyntaxState::Standard,
    SyntaxState::Identifier,
    SyntaxState::IdentifierTable,
    SyntaxState::IdentifierAssignment,
    SyntaxState::IdentifierAssignmentEnd,
    SyntaxState::IdentifierTableAssignmentHead,
    SyntaxState::IdentifierTableAssignmentRow,
    SyntaxState::Reserved,
    SyntaxState::ReservedBody,
};

const QString validparentheses("{[(}])/");
const QString specialBlocks("\"\'\"\'"); // ("[\"\']\"\'");

void SyntaxHighlighter::scanParentheses(const QString &text, int start, int len, SyntaxState state,  QVector<ParenthesesPos> &parentheses)
{
    bool inBlock = false;
    if (!validparenthesesSyntax.contains(state)) return;
    for (int i = start; i < start+len; ++i) {
        int kind = validparentheses.indexOf(text.at(i));
        if (kind == 6) {
            if (state == SyntaxState::IdentifierAssignmentEnd)
                parentheses << ParenthesesPos('\\', i);
            else if (state == SyntaxState::IdentifierAssignment)
                parentheses << ParenthesesPos(text.at(i), i);
        } else if (kind >= 0) {
            parentheses << ParenthesesPos(text.at(i), i);
        }
        int blockKind = specialBlocks.indexOf(text.at(i));
        if (!inBlock && blockKind >= 0 && blockKind < specialBlocks.length()/2) {
            int iEnd = text.indexOf(specialBlocks.at(blockKind + specialBlocks.length()/2), i+1);
            i = (iEnd > 0) ? iEnd-1 : text.length()-1;
            inBlock = true;
        } else {
            inBlock = false;
        }
    }
}

void SyntaxHighlighter::addState(SyntaxAbstract* syntax, CodeIndex ci)
{
    mStates << syntax;
    addCode(mStates.length()-1, ci);
}

QColor backColor(int index) {
    static QList<QColor> debColor {Qt::yellow, Qt::cyan, QColor(Qt::blue).lighter(170),
                                  QColor(Qt::green).lighter()};
    index = (qAbs(index)-1) % debColor.size();
    return debColor.at(index);
}

void SyntaxHighlighter::initState(int debug, SyntaxAbstract *syntax, QColor color, FontModifier fMod)
{
    if (debug) syntax->charFormat().setBackground(backColor(debug));

    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    if (color.isValid()) syntax->charFormat().setForeground(color);
    if (fMod & fItalic) syntax->charFormat().setFontItalic(true);
    if (fMod & fBold) syntax->charFormat().setFontWeight(QFont::Bold);
    mStates << syntax;
    addCode(mStates.length()-1, 0);
}

void SyntaxHighlighter::initState(SyntaxAbstract *syntax, QColor color, FontModifier fMod)
{
    initState(false, syntax, color, fMod);
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
