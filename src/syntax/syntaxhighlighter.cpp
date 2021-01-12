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
                     << SyntaxKind::Call << SyntaxKind::CommentLine << SyntaxKind::Title;

    // To visualize one format in DEBUG: add color index at start e.g. initKind(1, new SyntaxReservedBody());
    initKind(new SyntaxStandard(), Theme::Syntax_undefined);
    addCode(BlockCode(SyntaxKind::Standard, 0), 0);
    SyntaxDirective *syntaxDirective = new SyntaxDirective();
    initKind(syntaxDirective, Theme::Syntax_directive);
    SyntaxDirectiveBody *syntaxDirectiveBody = new SyntaxDirectiveBody(SyntaxKind::DirectiveBody);
    initKind(syntaxDirectiveBody, Theme::Syntax_directiveBody);
    syntaxDirective->setDirectiveBody(syntaxDirectiveBody);
    initKind(new SyntaxDirectiveBody(SyntaxKind::DirectiveComment), Theme::Syntax_comment);
    initKind(new SyntaxDirectiveBody(SyntaxKind::Title), Theme::Syntax_title);
    initKind(new SyntaxDirectiveBody(SyntaxKind::IgnoredHead), Theme::Syntax_directiveBody);
    initKind(new SyntaxCall(), Theme::Syntax_directive);

    SyntaxFormula * syntaxFormula = new SyntaxFormula(SyntaxKind::Formula);
    initKind(syntaxFormula, Theme::Syntax_formula);
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

    initKind(new SyntaxAssign(), Theme::Syntax_formula);
    initKind(new SyntaxString(), Theme::Syntax_neutral);
    initKind(new SyntaxCommentLine(), Theme::Syntax_comment);
    initKind(new SyntaxUniformBlock(SyntaxKind::CommentBlock), Theme::Syntax_comment);
    SyntaxCommentEndline *syntaxCommentEndline = new SyntaxCommentEndline();
    initKind(syntaxCommentEndline, Theme::Syntax_comment);
    syntaxDirective->setSyntaxCommentEndline(syntaxCommentEndline);
    initKind(new SyntaxUniformBlock(SyntaxKind::IgnoredBlock), Theme::Syntax_neutral);

    initKind(new SyntaxSubsetKey(SyntaxKind::SolveKey), Theme::Syntax_keyword);
    initKind(new SyntaxSubsetKey(SyntaxKind::OptionKey), Theme::Syntax_keyword);
    initKind(new SyntaxSubsetKey(SyntaxKind::ExecuteKey), Theme::Syntax_keyword);
    initKind(new SyntaxDelimiter(SyntaxKind::Semicolon));
    initKind(new SyntaxDelimiter(SyntaxKind::CommaIdent));

    initKind(new SyntaxReserved(SyntaxKind::Reserved), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Solve), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Option), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Execute), Theme::Syntax_keyword);
    initKind(new SyntaxEmbedded(SyntaxKind::Embedded), Theme::Syntax_keyword);
    initKind(new SyntaxEmbedded(SyntaxKind::EmbeddedEnd), Theme::Syntax_keyword);
    initKind(new SyntaxEmbeddedBody(), Theme::Syntax_embedded);
    initKind(new SyntaxPreDeclaration(SyntaxKind::DeclarationSetType), Theme::Syntax_declaration);
    initKind(new SyntaxPreDeclaration(SyntaxKind::DeclarationVariableType), Theme::Syntax_declaration);
    initKind(new SyntaxDeclaration(), Theme::Syntax_declaration);

    initKind(new SyntaxIdentifier(), Theme::Syntax_identifier);
    initKind(new SyntaxIdentifierDim(), Theme::Syntax_identifier);
    initKind(new SyntaxIdentifierDimEnd(), Theme::Syntax_identifier);
    initKind(new SyntaxIdentDescript(), Theme::Syntax_description);
    initKind(new SyntaxIdentAssign(SyntaxKind::IdentifierAssignment), Theme::Syntax_assignLabel);
    initKind(new AssignmentLabel(), Theme::Syntax_assignLabel);
    initKind(new AssignmentValue(), Theme::Syntax_assignValue);
    initKind(new SyntaxIdentAssign(SyntaxKind::IdentifierAssignmentEnd), Theme::Syntax_assignLabel);

    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentColHead), Theme::Syntax_assignLabel);
    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentRowHead), Theme::Syntax_assignLabel);
    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentRow), Theme::Syntax_assignValue);
}

SyntaxHighlighter::~SyntaxHighlighter()
{
    Kinds::iterator it = mKinds.begin();
    while (it != mKinds.end()) {
        SyntaxAbstract *syn = it.value();
        it = mKinds.erase(it);
        delete syn;
    }
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    QVector<ParenthesesPos> parPosList;
    parPosList.reserve(20);
    CodeRelationIndex cri = previousBlockState();
    if (cri < 0) cri = 0;
    int index = 0;
    QTextBlock textBlock = currentBlock();
    if (!textBlock.userData()) textBlock.setUserData(new BlockData());
    BlockData* blockData = static_cast<BlockData*>(textBlock.userData());

    int posForSyntaxKind = mPositionForSyntaxKind - textBlock.position();
    if (posForSyntaxKind < 0) posForSyntaxKind = text.length();
    bool emptyLineKinds = true;
//    DEB() << text;

    NestingImpact nestingImpact;
    while (index < text.length()) {
        CodeRelation codeRel = mCodes.at(cri);
        SyntaxAbstract* syntax = mKinds.value(codeRel.blockCode.kind());
        if (!syntax) {
            DEB() << "no Syntax for " << syntaxKindName(codeRel.blockCode.kind());
            return;
        }
        bool stack = true;
         // detect end of valid trailing characters for current syntax
        SyntaxBlock tailBlock = syntax->validTail(text, index, codeRel.blockCode.flavor(), stack);
        if (stack) emptyLineKinds = false;

        // HOWTO(JM) For kinds redefined with directives:
        //   - add new Syntax to mKinds
        //   - create a new full set of Syntax in mCodes with just the new one replaced
        // -> result: the top code will change from 0 to the new Standard top
        SyntaxBlock nextBlock;
        for (SyntaxKind nextKind: syntax->nextKinds(emptyLineKinds)) {
            SyntaxAbstract* testSyntax = mKinds.value(nextKind);
            if (testSyntax) {
                SyntaxBlock testBlock = testSyntax->find(syntax->kind(), tailBlock.flavor, text, index);
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
                cri = getCode(cri, SyntaxShift::reset, tailBlock);
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
//                                  << tailBlock.syntax->kind() << " flav_" << tailBlock.flavor << "  (tail from " << syntax->kind() << ")";
                        scanParentheses(text, tailBlock, syntax->kind(), parPosList, nestingImpact);
                    }
                    cri = getCode(cri, tailBlock.shift, tailBlock, 0);
                }
            }
        }

        if (nextBlock.error && nextBlock.length() > 0) {
            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormatError());
        } else if (nextBlock.syntax->kind() != SyntaxKind::Standard) {
            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormat());
//            if (nextBlock.syntax)
//                DEB() << QString(nextBlock.start, ' ') << QString(nextBlock.length(), '_')
//                      << " " << nextBlock.syntax->kind() << " flav_" << nextBlock.flavor << "  (next from " << syntax->kind() << ")";
            if (nextBlock.syntax->kind() == SyntaxKind::Semicolon) emptyLineKinds = true;
        }
        scanParentheses(text, nextBlock, syntax->kind(), parPosList, nestingImpact);
        index = nextBlock.end;

        cri = getCode(cri, nextBlock.shift, nextBlock, 0);

        if (posForSyntaxKind <= index) {
            mLastSyntaxKind = nextBlock.syntax->intSyntaxType();
            mPositionForSyntaxKind = -1;
            posForSyntaxKind = text.length()+1;
        }
    }
    if (blockData->foldCount()) {
        QVector<ParenthesesPos> blockPars = blockData->parentheses();
        bool same = (parPosList.size() == blockPars.size());
        for (int i = 0; i < parPosList.size() && same; ++i) {
            same = (parPosList.at(i).character == blockPars.at(i).character);
        }
        if (!same) emit needUnfold(textBlock);
    }
    blockData->setParentheses(parPosList, nestingImpact);
    if (blockData && blockData->isEmpty()) {
        textBlock.setUserData(nullptr);
    } else
        textBlock.setUserData(blockData);
    setCurrentBlockState(purgeCode(cri));
//    DEB() << text << "      _" << codeDeb(cri) << " [nesting " << nestingImpact.impact() << "]";
}

void SyntaxHighlighter::syntaxKind(int position, int &intKind)
{
    mPositionForSyntaxKind = position;
    mLastSyntaxKind = 0;
    rehighlightBlock(document()->findBlock(position));
    intKind = mLastSyntaxKind;
    mLastSyntaxKind = 0;
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
    SyntaxKind::IgnoredBlock,
    SyntaxKind::DeclarationSetType,
    SyntaxKind::DeclarationVariableType,
    SyntaxKind::Declaration,
    SyntaxKind::IdentifierDescription,
    SyntaxKind::Embedded,
    SyntaxKind::EmbeddedEnd,
};

const QString validParentheses("{[(}])/");
const QString specialBlocks("\"\'\"\'"); // ("[\"\']\"\'");
const QString flavorChars("TtCcPpIiOoFfUu");

void SyntaxHighlighter::scanParentheses(const QString &text, SyntaxBlock block, SyntaxKind preKind,
                                        QVector<ParenthesesPos> &parentheses, NestingImpact &nestingImpact)
{
    int start = block.start;
    int len = block.length();
    int flavor = block.flavor;
    SyntaxKind kind = block.syntax->kind();
    SyntaxKind postKind = block.next;

    bool inBlock = false;
    if (kind == SyntaxKind::Embedded) {
        parentheses << ParenthesesPos('E', start);
        nestingImpact.addOpener();
        return;
    } else if (kind == SyntaxKind::Directive && postKind == SyntaxKind::EmbeddedBody) {
        parentheses << ParenthesesPos('M', start);
        nestingImpact.addOpener();
        return;
    } else if (kind == SyntaxKind::EmbeddedEnd) {
        parentheses << ParenthesesPos('e', start);
        nestingImpact.addCloser();
        return;
    } else if (preKind == SyntaxKind::EmbeddedBody && kind == SyntaxKind::Directive) {
        parentheses << ParenthesesPos('m', start);
        nestingImpact.addCloser();
        return;
    } else if (kind == SyntaxKind::Directive) {
        if (flavor > 0 && flavor <= flavorChars.size()) {
            parentheses << ParenthesesPos(flavorChars.at(flavor-1), start);
            if (flavor%2)
                nestingImpact.addOpener();
            else
                nestingImpact.addCloser();
            return;
        }
    }
    if (invalidParenthesesSyntax.contains(kind)) return;
    for (int i = start; i < start+len; ++i) {
        if (i >= text.length()) break;
        int iPara = validParentheses.indexOf(text.at(i));
        if (iPara == 6) {
            if (kind == SyntaxKind::IdentifierAssignmentEnd) {
                parentheses << ParenthesesPos('\\', i);
                nestingImpact.addCloser();
            } else if (kind == SyntaxKind::IdentifierAssignment) {
                parentheses << ParenthesesPos('/', i);
                nestingImpact.addOpener();
            }
        } else if (iPara >= 0) {
            parentheses << ParenthesesPos(text.at(i), i);
            if (iPara<3)
                nestingImpact.addOpener();
            else
                nestingImpact.addCloser();

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
    return;
}

QColor backColor(int index) {
    static QList<QColor> debColor { QColor(Qt::yellow).darker(105), QColor(Qt::cyan).lighter(170),
                                    QColor(Qt::blue).lighter(180), QColor(Qt::green).lighter(170) };
    index = (qAbs(index)-1) % debColor.size();
    return debColor.at(index);
}

void SyntaxHighlighter::initKind(int debug, SyntaxAbstract *syntax, Theme::ColorSlot slot)
{
    if (debug) syntax->charFormat().setBackground(backColor(debug));
    syntax->assignColorSlot(slot);

    // TODO(JM) check if mSingleLineKinds can be left out of mKinds because the code won't be passed to the next line
//    if (!mSingleLineKinds.contains(syntax->kind())) {}
    mKinds.insert(syntax->kind(), syntax);
//    addCode(mKinds.length()-1, 0, 0);
}

void SyntaxHighlighter::initKind(SyntaxAbstract *syntax, Theme::ColorSlot slot)
{
    initKind(false, syntax, slot);
}

void SyntaxHighlighter::reloadColors()
{
    for (SyntaxAbstract* syntax: mKinds) {
        syntax->assignColorSlot(syntax->colorSlot());
    }
}

int SyntaxHighlighter::addCode(BlockCode code, CodeRelationIndex parentIndex)
{
    CodeRelation sc(code, parentIndex);
    if (code.code() < 0)
        EXCEPT() << "Can't generate code for invalid BlockCode";
    int index = mCodes.indexOf(sc);
    if (index >= 0)
        return index;
    mCodes << sc;
    return mCodes.length()-1;
}

CodeRelationIndex SyntaxHighlighter::getCode(CodeRelationIndex cri, SyntaxShift shift, SyntaxBlock block, int nest)
{
    Q_UNUSED(nest)
    cri = qBound(0, cri, mCodes.size());
    if (shift == SyntaxShift::skip) {
        return cri;
    } else if (shift == SyntaxShift::out) {
        return mCodes.at(cri).prevCodeRelIndex;
    } else if (shift == SyntaxShift::in) {
        return addCode(BlockCode(block.next, block.flavor), cri);
    } else if (shift == SyntaxShift::shift) {
        return addCode(BlockCode(block.syntax->kind(), block.flavor), mCodes.at(cri).prevCodeRelIndex);
    }

    // SyntaxShift::reset
    while (mCodes.at(cri).blockCode.kind() != SyntaxKind::Standard) {
        cri = mCodes.at(cri).prevCodeRelIndex;
    }
    return cri;
}

int SyntaxHighlighter::purgeCode(CodeRelationIndex cri)
{
    if (cri < 0) return 0;
    SyntaxKind kind = mCodes.at(cri).blockCode.kind();
    while (mSingleLineKinds.contains(kind) && kind != SyntaxKind::Standard) {
        cri = mCodes.at(cri).prevCodeRelIndex;
        kind = mCodes.at(cri).blockCode.kind();
    }
    return cri;
}

QString SyntaxHighlighter::codeDeb(CodeRelationIndex cri)
{
    QString res = syntaxKindName(mCodes.at(cri).blockCode.kind());
    while (cri > 0 && cri != mCodes.at(cri).prevCodeRelIndex) {
        cri = mCodes.at(cri).prevCodeRelIndex;
        res = syntaxKindName(mCodes.at(cri).blockCode.kind()) + "["
                + QString::number(mCodes.at(cri).blockCode.flavor()) + "], " + res;
    }
    return res;
}


} // namespace syntax
} // namespace studio
} // namespace gams
