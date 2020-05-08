/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "settings.h"

namespace gams {
namespace studio {
namespace syntax {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc)
    : BaseHighlighter(doc)
{
    // TODO(JM) Check what additional kinds belong here too (kinds that won't be passed to the next line)
    mSingleLineKinds << SyntaxKind::Directive << SyntaxKind::DirectiveBody << SyntaxKind::CommentEndline
                     << SyntaxKind::CommentLine << SyntaxKind::Title;

    // To visualize one format in DEBUG: add color index at start e.g. initKind(1, new SyntaxReservedBody());
    initKind(new SyntaxStandard(), Scheme::Syntax_undefined);
    SyntaxDirective *syntaxDirective = new SyntaxDirective();
    initKind(syntaxDirective, Scheme::Syntax_directive);
    SyntaxDirectiveBody *syntaxDirectiveBody = new SyntaxDirectiveBody(SyntaxKind::DirectiveBody);
    initKind(syntaxDirectiveBody, Scheme::Syntax_directiveBody);
    syntaxDirective->setDirectiveBody(syntaxDirectiveBody);
    initKind(new SyntaxDirectiveBody(SyntaxKind::DirectiveComment), Scheme::Syntax_comment);
    initKind(new SyntaxDirectiveBody(SyntaxKind::Title), Scheme::Syntax_title);

    SyntaxFormula * syntaxFormula = new SyntaxFormula(SyntaxKind::Formula);
    initKind(syntaxFormula);
    syntaxDirective->addSubBody(syntaxFormula);
    SyntaxFormula * syntaxSolveBody = new SyntaxFormula(SyntaxKind::SolveBody);
    initKind(syntaxSolveBody);
    syntaxDirective->addSubBody(syntaxSolveBody);
    SyntaxFormula * syntaxOptionBody = new SyntaxFormula(SyntaxKind::OptionBody);
    initKind(syntaxOptionBody);
    syntaxDirective->addSubBody(syntaxOptionBody);
    SyntaxFormula * syntaxExecuteBody = new SyntaxFormula(SyntaxKind::ExecuteBody);
    initKind(syntaxExecuteBody);
    syntaxDirective->addSubBody(syntaxExecuteBody);

    initKind(new SyntaxAssign(), Scheme::Syntax_assign);
    initKind(new SyntaxString(), Scheme::Syntax_neutral);
    initKind(new SyntaxCommentLine(), Scheme::Syntax_comment);
    initKind(new SyntaxCommentBlock(), Scheme::Syntax_comment);
    SyntaxCommentEndline *syntaxCommentEndline = new SyntaxCommentEndline();
    initKind(syntaxCommentEndline, Scheme::Syntax_comment);
    syntaxDirective->setSyntaxCommentEndline(syntaxCommentEndline);

    initKind(new SyntaxSubsetKey(SyntaxKind::SolveKey), Scheme::Syntax_keyword);
    initKind(new SyntaxSubsetKey(SyntaxKind::OptionKey), Scheme::Syntax_keyword);
    initKind(new SyntaxSubsetKey(SyntaxKind::ExecuteKey), Scheme::Syntax_keyword);
    initKind(new SyntaxDelimiter(SyntaxKind::Semicolon));
    initKind(new SyntaxDelimiter(SyntaxKind::CommaIdent));
    initKind(new SyntaxDelimiter(SyntaxKind::CommaTable));

    initKind(new SyntaxReserved(SyntaxKind::Reserved), Scheme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Solve), Scheme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Option), Scheme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Execute), Scheme::Syntax_keyword);
    initKind(new SyntaxEmbedded(SyntaxKind::Embedded), Scheme::Syntax_keyword);
    initKind(new SyntaxEmbedded(SyntaxKind::EmbeddedEnd), Scheme::Syntax_keyword);
    initKind(new SyntaxEmbeddedBody(), Scheme::Syntax_embedded);
    initKind(new SyntaxPreDeclaration(SyntaxKind::DeclarationSetType), Scheme::Syntax_declaration);
    initKind(new SyntaxPreDeclaration(SyntaxKind::DeclarationVariableType), Scheme::Syntax_declaration);
    initKind(new SyntaxDeclaration(), Scheme::Syntax_declaration);
    initKind(new SyntaxDeclarationTable(), Scheme::Syntax_declaration);

    initKind(new SyntaxIdentifier(SyntaxKind::Identifier), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentifierDim(SyntaxKind::IdentifierDim1), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentifierDim(SyntaxKind::IdentifierDim2), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentifierDimEnd(SyntaxKind::IdentifierDimEnd1), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentifierDimEnd(SyntaxKind::IdentifierDimEnd2), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentDescript(SyntaxKind::IdentifierDescription), Scheme::Syntax_description);
    initKind(new SyntaxIdentAssign(SyntaxKind::IdentifierAssignment), Scheme::Syntax_identifierAssign);
    initKind(new AssignmentLabel(), Scheme::Syntax_assignLabel);
    initKind(new AssignmentValue(), Scheme::Syntax_assignValue);
    initKind(new SyntaxIdentAssign(SyntaxKind::IdentifierAssignmentEnd), Scheme::Syntax_identifierAssign);

    initKind(new SyntaxIdentifier(SyntaxKind::IdentifierTable), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentifierDim(SyntaxKind::IdentifierTableDim1), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentifierDim(SyntaxKind::IdentifierTableDim2), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentifierDimEnd(SyntaxKind::IdentifierTableDimEnd1), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentifierDimEnd(SyntaxKind::IdentifierTableDimEnd2), Scheme::Syntax_identifier);
    initKind(new SyntaxIdentDescript(SyntaxKind::IdentifierTableDescription), Scheme::Syntax_description);

    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentHead), Scheme::Syntax_tableHeader);
    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentRow), Scheme::Syntax_identifierAssign);
}

SyntaxHighlighter::~SyntaxHighlighter()
{
    while (!mKinds.isEmpty()) {
        delete mKinds.takeFirst();
    }
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    QVector<ParenthesesPos> parPosList;
    parPosList.reserve(20);
    BlockCode code = previousBlockState();
    if (!code.isValid()) code = 0;
    int index = 0;
    QTextBlock textBlock = currentBlock();
    if (!textBlock.userData()) textBlock.setUserData(new BlockData());
    BlockData* blockData = static_cast<BlockData*>(textBlock.userData());

    int posForSyntaxKind = mPositionForSyntaxKind - textBlock.position();
    if (posForSyntaxKind < 0) posForSyntaxKind = text.length();
    bool emptyLineKinds = true;
//    DEB() << text;

    while (index < text.length()) {
        KindCode kindCode = (!code.isValid()) ? mCodes.at(0) : mCodes.at(code.kind());
        SyntaxAbstract* syntax = mKinds.at(kindCode.first);
        bool stack = true;
         // detect end of valid trailing characters for current syntax
        SyntaxBlock tailBlock = syntax->validTail(text, index, stack);
        if (stack) emptyLineKinds = false;

        // HOWTO(JM) For kinds redefined with directives:
        //   - add new Syntax to mKinds
        //   - create a new full set of Syntax in mCodes with just the new one replaced
        // -> result: the top code will change from 0 to the new Standard top
        SyntaxBlock nextBlock;
        for (SyntaxKind nextKind: syntax->nextKinds(emptyLineKinds)) {
            SyntaxAbstract* testSyntax = getSyntax(nextKind);
            if (testSyntax) {
                SyntaxBlock testBlock = testSyntax->find(syntax->kind(), 0, text, index);
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
                index = text.length();
                code = getCode(code, SyntaxShift::reset, kindCode.first, kindCode.first);
                continue;
            }
            nextBlock = tailBlock;
        } else {
            emptyLineKinds = false;
            if (tailBlock.isValid()) {
                if (nextBlock.start < tailBlock.end) tailBlock.end = nextBlock.start;
                if (tailBlock.isValid()) {
                    if (tailBlock.syntax->kind() != SyntaxKind::Standard) {
                        setFormat(tailBlock.start, tailBlock.length(), tailBlock.syntax->charFormat());
//                        if (tailBlock.syntax)
//                            DEB() << QString(tailBlock.start, ' ') << QString(tailBlock.length(), '.') << " "
//                                  << tailBlock.syntax->kind() << "  (tail from " << syntax->kind() << ")";
                        scanParentheses(text, tailBlock.start, tailBlock.length(), syntax->kind(),
                                        tailBlock.syntax->kind(), tailBlock.next, parPosList);
                    }
                    code = getCode(code, tailBlock.shift, getKindIdx(tailBlock.syntax->kind()), getKindIdx(tailBlock.next));
                }
            }
        }

        if (nextBlock.error && nextBlock.length() > 0) {
            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormatError());
        } else if (nextBlock.syntax->kind() != SyntaxKind::Standard) {
            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormat());
//            if (nextBlock.syntax)
//                DEB() << QString(nextBlock.start, ' ') << QString(nextBlock.length(), '_')
//                      << " " << nextBlock.syntax->kind() << "  (next from " << syntax->kind() << ")";
            if (nextBlock.syntax->kind() == SyntaxKind::Semicolon) emptyLineKinds = true;
        }
        scanParentheses(text, nextBlock.start, nextBlock.length(), syntax->kind(),
                        nextBlock.syntax->kind(), nextBlock.next, parPosList);
        index = nextBlock.end;

        code = getCode(code, nextBlock.shift, getKindIdx(nextBlock.syntax->kind()), getKindIdx(nextBlock.next));

        if (posForSyntaxKind <= index) {
            mLastSyntaxKind = nextBlock.syntax->intSyntaxType();
            mPositionForSyntaxKind = -1;
            posForSyntaxKind = text.length()+1;
        }
    }
    // update BlockData
    if (!parPosList.isEmpty() && !blockData) {
        blockData = new BlockData();
    }
    if (blockData) blockData->setParentheses(parPosList);
    if (blockData && blockData->isEmpty())
        textBlock.setUserData(nullptr);
    else
        textBlock.setUserData(blockData);
    setCurrentBlockState(purgeCode(code.code()));
//    DEB() << text << "      _" << codeDeb(code.code());
}

void SyntaxHighlighter::syntaxKind(int position, int &intKind)
{
    mPositionForSyntaxKind = position;
    mLastSyntaxKind = 0;
    rehighlightBlock(document()->findBlock(position));
    intKind = mLastSyntaxKind;
    mLastSyntaxKind = 0;
}

SyntaxAbstract*SyntaxHighlighter::getSyntax(SyntaxKind kind) const
{
    int i = mKinds.length();
    while (i > 0) {
        --i;
        if (mKinds.at(i)->kind() == kind) return mKinds.at(i);
    }
    return nullptr;
}

int SyntaxHighlighter::getKindIdx(SyntaxKind kind) const
{
    int i = mKinds.length();
    while (i > 0) {
        --i;
        if (mKinds.at(i)->kind() == kind) return i;
    }
    return -1;
}

const QVector<SyntaxKind> invalidParenthesesSyntax = {
    SyntaxKind::Directive,
    SyntaxKind::DirectiveBody,
    SyntaxKind::DirectiveComment,
    SyntaxKind::Title,
    SyntaxKind::String,
    SyntaxKind::Assignment,
    SyntaxKind::CommentLine,
    SyntaxKind::CommentBlock,
    SyntaxKind::CommentEndline,
    SyntaxKind::CommentInline,
    SyntaxKind::DeclarationSetType,
    SyntaxKind::DeclarationVariableType,
    SyntaxKind::Declaration,
    SyntaxKind::DeclarationTable,
    SyntaxKind::IdentifierDescription,
    SyntaxKind::Embedded,
    SyntaxKind::EmbeddedEnd,
};

const QString validParentheses("{[(}])/");
const QString specialBlocks("\"\'\"\'"); // ("[\"\']\"\'");

void SyntaxHighlighter::scanParentheses(const QString &text, int start, int len, SyntaxKind preKind, SyntaxKind kind, SyntaxKind postKind,  QVector<ParenthesesPos> &parentheses)
{
    bool inBlock = false;
    if (kind == SyntaxKind::Embedded || (kind == SyntaxKind::Directive && postKind == SyntaxKind::EmbeddedBody)) {
        parentheses << ParenthesesPos('E', start);
        return;
    } else if (kind == SyntaxKind::EmbeddedEnd || (preKind == SyntaxKind::EmbeddedBody && kind == SyntaxKind::Directive)) {
        parentheses << ParenthesesPos('e', start);
        return;
    }
    if (invalidParenthesesSyntax.contains(kind)) return;
    for (int i = start; i < start+len; ++i) {
        if (i >= text.length()) break;
        int iPara = validParentheses.indexOf(text.at(i));
        if (iPara == 6) {
            if (kind == SyntaxKind::IdentifierAssignmentEnd) {
                parentheses << ParenthesesPos('\\', i);
            } else if (kind == SyntaxKind::IdentifierAssignment) {
                parentheses << ParenthesesPos('/', i);
            }
        } else if (iPara >= 0) {
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

void SyntaxHighlighter::addKind(SyntaxAbstract* syntax, CodeIndex ci)
{
    mKinds << syntax;
    addCode(mKinds.length()-1, ci);
}

QColor backColor(int index) {
    static QList<QColor> debColor { QColor(Qt::yellow).darker(105), QColor(Qt::cyan).lighter(170),
                                    QColor(Qt::blue).lighter(180), QColor(Qt::green).lighter(170) };
    index = (qAbs(index)-1) % debColor.size();
    return debColor.at(index);
}

void SyntaxHighlighter::initKind(int debug, SyntaxAbstract *syntax, Scheme::ColorSlot slot)
{
    if (debug) syntax->charFormat().setBackground(backColor(debug));
    syntax->assignColorSlot(slot);

    // TODO(JM) check if mSingleLineKinds can be left out of mKinds because the code won't be passed to the next line
//    if (!mSingleLineKinds.contains(syntax->kind())) {}
    mKinds << syntax;
    addCode(mKinds.length()-1, 0);
}

void SyntaxHighlighter::initKind(SyntaxAbstract *syntax, Scheme::ColorSlot slot)
{
    initKind(false, syntax, slot);
}

void SyntaxHighlighter::reloadColors()
{
    for (SyntaxAbstract* syntax: mKinds) {
        syntax->assignColorSlot(syntax->colorSlot());
    }
}

int SyntaxHighlighter::addCode(KindIndex si, CodeIndex ci)
{
    KindCode sc(si, ci);
    if (si < 0)
        EXCEPT() << "Can't generate code for invalid KindIndex";
    int index = mCodes.indexOf(sc);
    if (index >= 0)
        return index;
    mCodes << sc;
    return mCodes.length()-1;
}

BlockCode SyntaxHighlighter::getCode(BlockCode code, SyntaxShift shift, KindIndex kind, KindIndex kindNext, int nest)
{
    if (!code.isValid()) code = 0; // default to syntax gams-standard
    if (nest) {
        code.setDepth(code.depth() + nest);
        if (code.depth() > 0) return code;
    }
    if (shift == SyntaxShift::skip) {
        return code;
    } else if (shift == SyntaxShift::out) {
        code.setKind(mCodes.at(code.kind()).second);
        return code;
    } else if (shift == SyntaxShift::in) {
        code.setKind(addCode(kindNext, code.kind()));
        return code;
    } else if (shift == SyntaxShift::shift) {
        code.setKind(addCode(kind, mCodes.at(code.kind()).second));
        return code;
    }

    while (mKinds.at(mCodes.at(code.kind()).first)->kind() != SyntaxKind::Standard) {
        code.setKind(mCodes.at(code.kind()).second);
    }
    return code;
}

int SyntaxHighlighter::purgeCode(int code)
{
    SyntaxKind kind = mKinds.at(mCodes.at(code).first)->kind();
    while (mSingleLineKinds.contains(kind)) {
        code = mCodes.at(code).second;
        kind = mKinds.at(mCodes.at(code).first)->kind();
    }
    return code;
}

QString SyntaxHighlighter::codeDeb(int code)
{
    QString res = syntaxKindName(mKinds.at(mCodes.at(code).first)->kind());
    while (code) {
        code = mCodes.at(code).second;
        res = syntaxKindName(mKinds.at(mCodes.at(code).first)->kind()) + ", " + res;
    }
    return res;
}


} // namespace syntax
} // namespace studio
} // namespace gams
