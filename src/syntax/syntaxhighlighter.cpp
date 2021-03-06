/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
    mSingleLineKinds << SyntaxKind::Dco << SyntaxKind::DcoBody << SyntaxKind::SubDCO << SyntaxKind::Title
                     << SyntaxKind::CommentEndline << SyntaxKind::CommentLine << SyntaxKind::String
                     << SyntaxKind::SystemRunAttrib << SyntaxKind::SystemCompileAttrib << SyntaxKind::UserCompileAttrib;

    mPostKindBlocker << SyntaxKind::CommentLine << SyntaxKind::CommentBlock << SyntaxKind::CommentEndline
                     << SyntaxKind::CommentInline;

    SharedSyntaxData *d = new SharedSyntaxData();

    // To visualize one format in DEBUG: add color index at start e.g. initKind(1, new SyntaxReservedBody());
    initKind(new SyntaxStandard(d), Theme::Syntax_undefined);
    addCode(BlockCode(SyntaxKind::Standard, 0), 0);
    initKind(new SyntaxDco(d), Theme::Syntax_dco);
    initKind(new SyntaxDcoBody(SyntaxKind::DcoBody, d), Theme::Syntax_dcoBody);
    initKind(new SyntaxDcoBody(SyntaxKind::DcoComment, d), Theme::Syntax_comment);
    initKind(new SyntaxDcoBody(SyntaxKind::Title, d), Theme::Syntax_title);
    initKind(new SyntaxDcoBody(SyntaxKind::IgnoredHead, d), Theme::Syntax_dcoBody);
    initKind(new SyntaxSubDCO(d), Theme::Syntax_dco);

    initKind(new SyntaxFormula(SyntaxKind::Formula, d), Theme::Syntax_formula);
    initKind(new SyntaxFormula(SyntaxKind::PutFormula, d), Theme::Syntax_formula);
    initKind(new SyntaxFormula(SyntaxKind::SolveBody, d));
    initKind(new SyntaxFormula(SyntaxKind::OptionBody, d));
    initKind(new SyntaxFormula(SyntaxKind::ExecuteBody, d));

    initKind(new SyntaxSimpleKeyword(SyntaxKind::SystemRunAttrib, d), Theme::Syntax_embedded);
    initKind(new SyntaxSimpleKeyword(SyntaxKind::SystemCompileAttrib, d), Theme::Syntax_embedded);
    initKind(new SyntaxSimpleWord(d), Theme::Syntax_embedded);

    initKind(new SyntaxAssign(d), Theme::Syntax_formula);
    initKind(new SyntaxQuoted(SyntaxKind::String, d), Theme::Syntax_assignLabel);
    initKind(new SyntaxCommentLine(d), Theme::Syntax_comment);
    initKind(new SyntaxUniformBlock(SyntaxKind::CommentBlock, d), Theme::Syntax_comment);
    initKind(new SyntaxCommentEndline(d), Theme::Syntax_comment);
    initKind(new SyntaxUniformBlock(SyntaxKind::IgnoredBlock, d), Theme::Syntax_neutral);

    initKind(new SyntaxSubsetKey(SyntaxKind::SolveKey, d), Theme::Syntax_keyword);
    initKind(new SyntaxSubsetKey(SyntaxKind::OptionKey, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::ExecuteKey, d), Theme::Syntax_keyword);
    initKind(new SyntaxDelimiter(SyntaxKind::Semicolon, d));
    initKind(new SyntaxDelimiter(SyntaxKind::CommaIdent, d));

    initKind(new SyntaxReserved(SyntaxKind::Reserved, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Solve, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Option, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Execute, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Put, d), Theme::Syntax_keyword);
    initKind(new SyntaxEmbedded(SyntaxKind::Embedded, d), Theme::Syntax_keyword);
    initKind(new SyntaxEmbedded(SyntaxKind::EmbeddedEnd, d), Theme::Syntax_keyword);
    initKind(new SyntaxEmbeddedBody(d), Theme::Syntax_embedded);
    initKind(new SyntaxPreDeclaration(SyntaxKind::DeclarationSetType, d), Theme::Syntax_declaration);
    initKind(new SyntaxPreDeclaration(SyntaxKind::DeclarationVariableType, d), Theme::Syntax_declaration);
    initKind(new SyntaxDeclaration(d), Theme::Syntax_declaration);

    initKind(new SyntaxIdentifier(d), Theme::Syntax_identifier);
    initKind(new SyntaxIdentifierDim(d), Theme::Syntax_identifier);
    initKind(new SyntaxIdentifierDimEnd(d), Theme::Syntax_identifier);
    initKind(new SyntaxIdentDescript(d), Theme::Syntax_description);
    initKind(new SyntaxIdentAssign(SyntaxKind::IdentifierAssignment, d), Theme::Syntax_assignLabel);
    initKind(new AssignmentLabel(d), Theme::Syntax_assignLabel);
    initKind(new AssignmentValue(d), Theme::Syntax_assignValue);
    initKind(new SyntaxIdentAssign(SyntaxKind::IdentifierAssignmentEnd, d), Theme::Syntax_assignLabel);
    initKind(new AssignmentSystemData(d), Theme::Syntax_assignLabel);

    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentColHead, d), Theme::Syntax_tableHeader);
    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentRowHead, d), Theme::Syntax_tableHeader);
    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentRow, d), Theme::Syntax_assignValue);

    mPostSyntax << mKinds.value(SyntaxKind::SystemCompileAttrib) << mKinds.value(SyntaxKind::UserCompileAttrib);

    if (!d->isValid()) {
        EXCEPT() << "ERROR: Incomplete SharedSyntaxData";
    }
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
    QVector<QPoint> postHighlights;
    postHighlights << QPoint(0, text.length()-1);
    QTextBlock textBlock = currentBlock();
    if (!textBlock.userData()) textBlock.setUserData(new BlockData());
    BlockData* blockData = static_cast<BlockData*>(textBlock.userData());

    bool scanBlock = (textBlock.blockNumber() == mScanBlockNr);
    if (scanBlock) {
        CodeRelation codeRel = mCodes.at(cri);
        mScannedBlockSyntax.insert(0, QPair<int,int>(int(codeRel.blockCode.kind()), codeRel.blockCode.flavor()));
    }

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
        int prevFlavor = tailBlock.isValid() ? tailBlock.flavor : codeRel.blockCode.flavor();

        // HOWTO(JM) For kinds redefined with a DCO:
        //   - add new Syntax to mKinds
        //   - create a new full set of Syntax in mCodes with just the new one replaced
        // -> result: the top code will change from 0 to the new Standard top
        SyntaxBlock nextBlock;
        for (const SyntaxKind &nextKind: syntax->nextKinds(emptyLineKinds)) {
            SyntaxAbstract* testSyntax = mKinds.value(nextKind);
            if (testSyntax) {
                SyntaxBlock testBlock = testSyntax->find(syntax->kind(), prevFlavor, text, index);
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
//                                  << tailBlock.syntax->name() << " flav_" << prevFlavor << "  (tail from " << syntax->name() << ")";
                        scanParentheses(text, tailBlock, syntax->kind(), parPosList, nestingImpact);
                    }
                    cri = getCode(cri, tailBlock.shift, tailBlock, 0);
                }
            }
        }

        if (nextBlock.error && nextBlock.length() > 0) {
            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormatError());
        } else if (nextBlock.syntax->kind() != SyntaxKind::Standard) {
            if (mPostKindBlocker.contains(nextBlock.syntax->kind()) && postHighlights.last().y() > nextBlock.start) {
                // CommentLine, CommentBlock, CommentEndline, CommentInline
                postHighlights[postHighlights.size()-1].setY(nextBlock.start);
                if (nextBlock.syntax->kind() == SyntaxKind::CommentInline)
                    postHighlights << QPoint(nextBlock.end, text.length()-1);
            }

            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormat());
//            if (nextBlock.syntax)
//                DEB() << QString(nextBlock.start, ' ') << QString(nextBlock.length(), '_')
//                      << " " << nextBlock.syntax->name() << " flav_" << nextBlock.flavor << "  (next from " << syntax->name() << ")";
            if (nextBlock.syntax->kind() == SyntaxKind::Semicolon) emptyLineKinds = true;
        }
        scanParentheses(text, nextBlock, syntax->kind(), parPosList, nestingImpact);
        index = nextBlock.end;

        cri = getCode(cri, nextBlock.shift, nextBlock, 0);

        if (scanBlock) {
            QMap<int, QPair<int, int> >::Iterator it = mScannedBlockSyntax.insert(nextBlock.end,
                                         QPair<int,int>(int(nextBlock.syntax->kind()), nextBlock.flavor));
            if (it.key() > 0) {
                // adjust previous tailBlock
                --it;
                if (it.key() < nextBlock.start) {
                    mScannedBlockSyntax.insert(nextBlock.start, it.value());
                    if (it.key() > 0)
                        mScannedBlockSyntax.remove(it.key());
                }
            }
        }

        if (posForSyntaxKind <= index) {
            mLastSyntaxKind = nextBlock.syntax->intSyntaxType();
            mLastFlavor = nextBlock.flavor;
            mPositionForSyntaxKind = -1;
            posForSyntaxKind = text.length()+1;
        }
    }
    // check post highlights
    for (const QPoint &p : qAsConst(postHighlights)) {
        for (int i = p.x(); i <= p.y(); ++i) {
            if (text.at(i) == '%') {
                for (SyntaxAbstract* testSyntax : qAsConst(mPostSyntax)) {
                    if (testSyntax) {
                        SyntaxBlock nextBlock = testSyntax->find(SyntaxKind::Standard, 0, text, i);
                        if (nextBlock.isValid()) {
//                            if (nextBlock.syntax)
//                                DEB() << QString(nextBlock.start, ' ') << QString(nextBlock.length(), '_')
//                                      << " " << nextBlock.syntax->name();
                            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormat());
                            i = nextBlock.end;
                            if (scanBlock) {
                                QMap<int, QPair<int, int>>::ConstIterator it = mScannedBlockSyntax.upperBound(nextBlock.start);
                                if (it == mScannedBlockSyntax.constEnd()) {
                                    --it;
                                }
//                                QPair<int,int> currentVal = it.value();
//                                mScannedBlockSyntax.insert(nextBlock.end, QPair<int,int>(int(nextBlock.syntax->kind()), nextBlock.flavor));
                            }
                            break;
                        }
                    }
                }
            }
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

void SyntaxHighlighter::syntaxKind(int position, int &intKind, int &flavor)
{
    mPositionForSyntaxKind = position;
    mLastSyntaxKind = 0;
    mLastFlavor = 0;
    rehighlightBlock(document()->findBlock(position));
    intKind = mLastSyntaxKind;
    flavor = mLastFlavor;
    mLastSyntaxKind = 0;
}

void SyntaxHighlighter::scanSyntax(QTextBlock block, QMap<int, QPair<int, int> > &blockSyntax)
{
    mScanBlockNr = block.blockNumber();
    mScannedBlockSyntax.clear();
    rehighlightBlock(block);
    blockSyntax = mScannedBlockSyntax;
    mScannedBlockSyntax.clear();
    mScanBlockNr = -1;
}

const QVector<SyntaxKind> SyntaxHighlighter::cInvalidParenthesesSyntax = {
    SyntaxKind::Dco,
    SyntaxKind::DcoBody,
    SyntaxKind::DcoComment,
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

const QString SyntaxHighlighter::cValidParentheses("{[(}])/");
const QString SyntaxHighlighter::cSpecialBlocks("\"\'\"\'"); // ("[\"\']\"\'");
const QString SyntaxHighlighter::cFlavorChars("TtCcPpIiOoFfUu");

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
    } else if (kind == SyntaxKind::Dco && postKind == SyntaxKind::EmbeddedBody) {
        parentheses << ParenthesesPos('M', start);
        nestingImpact.addOpener();
        return;
    } else if (kind == SyntaxKind::EmbeddedEnd) {
        parentheses << ParenthesesPos('e', start);
        nestingImpact.addCloser();
        return;
    } else if (preKind == SyntaxKind::EmbeddedBody && kind == SyntaxKind::Dco) {
        parentheses << ParenthesesPos('m', start);
        nestingImpact.addCloser();
        return;
    } else if (kind == SyntaxKind::Dco) {
        if (flavor > 0 && flavor <= cFlavorChars.size()) {
            parentheses << ParenthesesPos(cFlavorChars.at(flavor-1), start);
            if (flavor%2)
                nestingImpact.addOpener();
            else
                nestingImpact.addCloser();
            return;
        }
    }
    if (cInvalidParenthesesSyntax.contains(kind)) return;
    for (int i = start; i < start+len; ++i) {
        if (i >= text.length()) break;
        int iPara = cValidParentheses.indexOf(text.at(i));
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
        int blockKind = cSpecialBlocks.indexOf(text.at(i));
        if (!inBlock && blockKind >= 0 && blockKind < cSpecialBlocks.length()/2) {
            int iEnd = text.indexOf(cSpecialBlocks.at(blockKind + cSpecialBlocks.length()/2), i+1);
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
    for (SyntaxAbstract* syntax: qAsConst(mKinds)) {
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
