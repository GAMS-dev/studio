/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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

#ifndef SYNTAXDEBUG
//#define SYNTAXDEBUG
#endif

#ifndef TDEB
#define TDEB(check, text) if (check) syntaxDebug(text);
#endif

namespace gams {
namespace studio {
namespace syntax {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc)
    : BaseHighlighter(doc)
{
    // TODO(JM) Check what additional kinds belong here too (kinds that won't be passed to the next line)
    mSingleLineKinds << SyntaxKind::Dco << SyntaxKind::DcoBody << SyntaxKind::SubDCO << SyntaxKind::Title
                     << SyntaxKind::CommentEndline << SyntaxKind::CommentLine << SyntaxKind::String
                     << SyntaxKind::SystemRunAttrib << SyntaxKind::SystemCompileAttrib << SyntaxKind::SystemCompileAttribR
                     << SyntaxKind::UserCompileAttrib;

    mPostKindBlocker << SyntaxKind::CommentLine << SyntaxKind::CommentBlock << SyntaxKind::CommentEndline
                     << SyntaxKind::CommentInline;

    SharedSyntaxData *d = new SharedSyntaxData();

    // To visualize one format in DEBUG: add color index at start e.g. initKind(1, new SyntaxReservedBody());
    initKind(new SyntaxStandard(d), Theme::Syntax_undefined);
    addCode(BlockCode(SyntaxKind::Standard, 0), SyntaxFlags(), 0);
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
    initKind(new SyntaxSimpleKeyword(SyntaxKind::SystemCompileAttribR, d), Theme::Syntax_embedded);
    initKind(new SyntaxSimpleWord(d), Theme::Syntax_embedded);

    initKind(new SyntaxAssign(d), Theme::Syntax_formula);
    initKind(new SyntaxQuoted(SyntaxKind::String, d), Theme::Syntax_assignLabel);
    initKind(new SyntaxCommentLine(d), Theme::Syntax_comment);
    initKind(new SyntaxUniformBlock(SyntaxKind::CommentBlock, d), Theme::Syntax_comment);
    initKind(new SyntaxCommentEndline(d), Theme::Syntax_comment);
    initKind(new SyntaxUniformBlock(SyntaxKind::IgnoredBlock, d), Theme::Syntax_neutral);

    initKind(new SyntaxReserved(SyntaxKind::AbortKey, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::ExecuteKey, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::ExecuteToolKey, d), Theme::Syntax_keyword);
    initKind(new SyntaxSubsetKey(SyntaxKind::SolveKey, d), Theme::Syntax_keyword);
    initKind(new SyntaxSubsetKey(SyntaxKind::OptionKey, d), Theme::Syntax_keyword);
    initKind(new SyntaxDelimiter(SyntaxKind::Semicolon, d));
    initKind(new SyntaxDelimiter(SyntaxKind::CommaIdent, d));

    initKind(new SyntaxReserved(SyntaxKind::Reserved, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Abort, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Execute, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::ExecuteTool, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Solve, d), Theme::Syntax_keyword);
    initKind(new SyntaxReserved(SyntaxKind::Option, d), Theme::Syntax_keyword);
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

    mPostSyntax << mKinds.value(SyntaxKind::SystemCompileAttrib)  << mKinds.value(SyntaxKind::SystemCompileAttribR)
                << mKinds.value(SyntaxKind::UserCompileAttrib);

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
    if (isUnformatted()) {
        // this removes a previously assigned format at the border to max-highlight
        setFormat(0, text.length(), QTextCharFormat());
        setCurrentBlockCRIndex(0);
        return;
    }
    CodeRelationIndex cri = previousBlockCRIndex();
    if (cri < 0) cri = 0;
    int index = 0;
    QVector<QPoint> postHighlights;
    postHighlights << QPoint(0, text.length()-1);
    QTextBlock textBlock = currentBlock();
    if (!textBlock.userData()) textBlock.setUserData(new BlockData());
    BlockData* blockData = static_cast<BlockData*>(textBlock.userData());

    if (maxLineLength() >= 0 && text.length() >= maxLineLength()) {
        setCurrentBlockCRIndex(purgeCode(cri));
        return;
    }

    bool scanBlock = (textBlock.blockNumber() == mScanBlockNr);
    if (scanBlock) {
        CodeRelation codeRel = mCodes.at(cri);
        mScannedBlockSyntax.insert(0, QPair<int,int>(int(codeRel.blockCode.kind()), codeRel.blockCode.flavor()));
    }

    int posForSyntaxKind = mPositionForSyntaxKind - textBlock.position();
    if (posForSyntaxKind < 0) posForSyntaxKind = text.length();
    bool emptyLineKinds = true;
    TDEB(true, text)

    NestingData nestingData;
    while (index < text.length()) {
        CodeRelation codeRel = mCodes.at(cri);
        SyntaxAbstract* syntax = mKinds.value(codeRel.blockCode.kind());
        if (!syntax) {
            DEB() << "no Syntax for " << syntaxKindName(codeRel.blockCode.kind());
            return;
        }
        SyntaxState state;
        state.flavor = codeRel.blockCode.flavor();
        state.setSyntaxFlags(codeRel.syntaxFlags);
        bool stack = true;
         // detect end of valid trailing characters for current syntax
        SyntaxBlock tailBlock = syntax->validTail(text, index, state, stack);
        if (stack) emptyLineKinds = false;
        if (tailBlock.isValid())
            state = tailBlock.state;

        // HOWTO(JM) For kinds redefined with a DCO:
        //   - add new Syntax to mKinds
        //   - create a new full set of Syntax in mCodes with just the new one replaced
        // -> result: the top code will change from 0 to the new Standard top
        SyntaxBlock nextBlock;
        for (const SyntaxKind &nextKind: syntax->nextKinds(emptyLineKinds)) {
            SyntaxAbstract* testSyntax = mKinds.value(nextKind);
            if (testSyntax) {
                SyntaxBlock testBlock = testSyntax->find(syntax->kind(), state, text, index);
                if (testBlock.isValid()) {
                    if (!nextBlock.isValid() || nextBlock.start > testBlock.start) {
                        nextBlock = testBlock;
                        if (scanBlock && testBlock.start <= mScanPosInBlock && testBlock.end >= mScanPosInBlock) {
                            mScannedPosDoc = testBlock.syntax->docForLastRequest();
                        }
                    }
                }
            }
        }
        if (!nextBlock.isValid()) {
            if (!tailBlock.isValid()) {
                // no valid characters found, mark error
                index = text.length();
                cri = getCode(cri, SyntaxShift::reset, tailBlock, tailBlock.state.syntaxFlags());
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
                        TDEB(tailBlock.syntax, QString(tailBlock.start, ' ') + QString(tailBlock.length(), '.')
                              + " " + tailBlock.syntax->name() + " flav_" + QString::number(state.flavor)
                              + "  (tail from " + syntax->name() + ")")
                        scanParentheses(text, tailBlock, syntax->kind(), nestingData);
                    }
                    cri = getCode(cri, tailBlock.shift, tailBlock, tailBlock.state.syntaxFlags(), 0);
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
            TDEB(nextBlock.syntax, QString(nextBlock.start, ' ') + QString(nextBlock.length(), '_')
                  + " " + nextBlock.syntax->name() + " flav_" + QString::number(nextBlock.state.flavor)
                  + "  (next from " + syntax->name() + ")")
            if (nextBlock.syntax->kind() == SyntaxKind::Semicolon) emptyLineKinds = true;
        }
        scanParentheses(text, nextBlock, syntax->kind(), nestingData);
        index = nextBlock.end;
        cri = getCode(cri, nextBlock.shift, nextBlock, nextBlock.state.syntaxFlags(), 0);

        if (scanBlock) {
            QMap<int, QPair<int, int> >::Iterator it = mScannedBlockSyntax.insert(nextBlock.end,
                                         QPair<int,int>(int(nextBlock.syntax->kind()), nextBlock.state.flavor));
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
            mLastFlavor = nextBlock.state.flavor;
            mPositionForSyntaxKind = -1;
            posForSyntaxKind = text.length()+1;
        }
    }

    // check post highlights
    for (const QPoint &p : std::as_const(postHighlights)) {
        for (int i = p.x(); i <= p.y(); ++i) {
            if (text.at(i) == '%') {
                for (SyntaxAbstract* testSyntax : std::as_const(mPostSyntax)) {
                    if (testSyntax) {
                        SyntaxBlock nextBlock = testSyntax->find(SyntaxKind::Standard, SyntaxState(), text, i);
                        if (nextBlock.isValid()) {
                            TDEB(nextBlock.syntax, QString(nextBlock.start, ' ') + QString(nextBlock.length(), '_')
                                  + " " + nextBlock.syntax->name())
                            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormat());
                            if (scanBlock) {
                                QMap<int, QPair<int, int>>::Iterator it = mScannedBlockSyntax.upperBound(nextBlock.start);
                                while (it != mScannedBlockSyntax.end() && it.key() < nextBlock.end) {
                                    int key = it.key();
                                    ++it;
                                    mScannedBlockSyntax.remove(key);
                                }
                                mScannedBlockSyntax.insert(nextBlock.end, QPair<int,int>(int(nextBlock.syntax->kind()), nextBlock.state.flavor));
                            }
                            i = nextBlock.end-1;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (blockData->foldCount()) {
        QVector<ParenthesesPos> blockPars = blockData->parentheses();
        bool same = (nestingData.parentheses().size() == blockPars.size());
        for (int i = 0; i < nestingData.parentheses().size() && same; ++i) {
            same = (nestingData.parentheses().at(i).character == blockPars.at(i).character);
        }
        if (!same) emit needUnfold(textBlock);
    }
    blockData->setParentheses(nestingData);
    textBlock.setUserData(blockData);
    setCurrentBlockCRIndex(purgeCode(cri));

    TDEB(true, text + "      _" + codeDeb(cri) + " [nesting " + QString::number(nestingData.impact()) + "]"
         + (mCodes.at(cri).syntaxFlags.isNull() ? "" : " @" + mCodes.at(cri).syntaxFlags->value(flagSuffixName)))
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

void SyntaxHighlighter::scanSyntax(const QTextBlock &block, QMap<int, QPair<int, int> > &blockSyntax, int pos)
{
    mScanBlockNr = block.blockNumber();
    mScanPosInBlock = pos;
    mScannedBlockSyntax.clear();
    rehighlightBlock(block);
    blockSyntax = mScannedBlockSyntax;
    mScannedBlockSyntax.clear();
    mScanBlockNr = -1;
    mScanPosInBlock = -1;
}

void SyntaxHighlighter::syntaxDocAt(const QTextBlock &block, int pos, QStringList &syntaxDoc)
{
    QMap<int, QPair<int, int>> blockSyntax;
    mScannedPosDoc = QStringList();
    scanSyntax(block, blockSyntax, pos);
    syntaxDoc = mScannedPosDoc;
    mScannedPosDoc = QStringList();
    for (QMap<int,QPair<int, int>>::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
        if (it.key() > pos)
            break;
    }
}

void SyntaxHighlighter::syntaxFlagData(const QTextBlock &block, SyntaxFlag flag, QString &value)
{
    if (block.userState() < 0) return;
    SyntaxFlagData *data = mCodes.at(block.userState()).syntaxFlags.get();
    if (!data) return;
    value = data->value(flag);
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

void SyntaxHighlighter::scanParentheses(const QString &text, SyntaxBlock block, SyntaxKind preKind, NestingData &nestingData)
{
    int start = block.start;
    int len = block.length();
    int flavor = block.state.flavor;
    SyntaxKind kind = block.syntax->kind();
    SyntaxKind postKind = block.next;

    bool inBlock = false;
    if (kind == SyntaxKind::Embedded) {
        nestingData.addOpener('E', start);
        return;
    } else if (kind == SyntaxKind::Dco && postKind == SyntaxKind::EmbeddedBody) {
        nestingData.addOpener('M', start);
        return;
    } else if (kind == SyntaxKind::EmbeddedEnd) {
        nestingData.addCloser('e', start);
        return;
    } else if (preKind == SyntaxKind::EmbeddedBody && kind == SyntaxKind::Dco) {
        nestingData.addCloser('m', start);
        return;
    } else if (kind == SyntaxKind::Dco) {
        if (flavor > 0 && flavor <= cFlavorChars.size()) {
            if (flavor%2)
                nestingData.addOpener(cFlavorChars.at(flavor-1), start);
            else
                nestingData.addCloser(cFlavorChars.at(flavor-1), start);
            return;
        }
    }
    if (cInvalidParenthesesSyntax.contains(kind)) return;
    for (int i = start; i < start+len; ++i) {
        if (i >= text.length()) break;
        int iPara = cValidParentheses.indexOf(text.at(i));
        if (iPara == 6) {
            if (kind == SyntaxKind::IdentifierAssignmentEnd) {
                nestingData.addCloser('\\', i);
            } else if (kind == SyntaxKind::IdentifierAssignment) {
                nestingData.addOpener('/', i);
            }
        } else if (iPara >= 0) {
            if (iPara<3)
                nestingData.addOpener(text.at(i), i);
            else
                nestingData.addCloser(text.at(i), i);

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

const QMap<Theme::ColorSlot, Theme::ColorSlot> SyntaxHighlighter::cForeToBackground = {
    {Theme::Syntax_assignLabel, Theme::Syntax_assignLabel_bg},
    {Theme::Syntax_assignValue, Theme::Syntax_assignValue_bg},
    {Theme::Syntax_comment, Theme::Syntax_comment_bg},
    {Theme::Syntax_dco, Theme::Syntax_dco_bg},
    {Theme::Syntax_dcoBody, Theme::Syntax_dcoBody_bg},
    {Theme::Syntax_declaration, Theme::Syntax_declaration_bg},
    {Theme::Syntax_description, Theme::Syntax_description_bg},
    {Theme::Syntax_embedded, Theme::Syntax_embedded_bg},
    {Theme::Syntax_identifier, Theme::Syntax_identifier_bg},
    {Theme::Syntax_keyword, Theme::Syntax_keyword_bg},
    {Theme::Syntax_tableHeader, Theme::Syntax_tableHeader_bg},
};

QColor backColor(int index) {
    static QList<QColor> debColor { QColor(Qt::yellow).darker(105), QColor(Qt::cyan).lighter(170),
                                    QColor(Qt::blue).lighter(180), QColor(Qt::green).lighter(170) };
    index = (qAbs(index)-1) % debColor.size();
    return debColor.at(index);
}

void SyntaxHighlighter::initKind(int debug, SyntaxAbstract *syntax, Theme::ColorSlot slot)
{
    if (debug) syntax->charFormat().setBackground(backColor(debug));
    else if (cForeToBackground.contains(slot)) {
        QColor bkColor = Theme::color(cForeToBackground.value(slot));
        if (bkColor != Theme::CAutoBackground) {
            syntax->charFormat().setBackground(bkColor);
        }
    }
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

bool SyntaxHighlighter::reloadColors()
{
    bool changed = false;
    for (SyntaxAbstract* syntax: std::as_const(mKinds)) {
        if (syntax->assignColorSlot(syntax->colorSlot()))
            changed = true;
    }
    return changed;
}

int SyntaxHighlighter::addCode(BlockCode code, const SyntaxFlags &synFlags, CodeRelationIndex parentIndex)
{
    CodeRelation sc(code, synFlags, parentIndex);
    if (code.code() < 0)
        EXCEPT() << "Can't generate code for invalid BlockCode";
    int index = mCodes.indexOf(sc);
    if (index >= 0)
        return index;
    mCodes << sc;
    return mCodes.length()-1;
}

CodeRelationIndex SyntaxHighlighter::getCode(CodeRelationIndex cri, SyntaxShift shift, const SyntaxBlock &block, const SyntaxFlags &synFlags, int nest)
{
    Q_UNUSED(nest)
    cri = qBound(0, cri, mCodes.size());
    if (shift == SyntaxShift::skip) {
        return cri;
    } else if (shift == SyntaxShift::out) {
        return mCodes.at(cri).prevCodeRelIndex;
    } else if (shift == SyntaxShift::in) {
        return addCode(BlockCode(block.next, block.state.flavor), synFlags, cri);
    } else if (shift == SyntaxShift::shift) {
        return addCode(BlockCode(block.syntax->kind(), block.state.flavor), synFlags, mCodes.at(cri).prevCodeRelIndex);
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
    QString res = QString("%1|").arg(cri) + syntaxKindName(mCodes.at(cri).blockCode.kind());
    while (cri > 0 && cri != mCodes.at(cri).prevCodeRelIndex) {
        cri = mCodes.at(cri).prevCodeRelIndex;
        res = syntaxKindName(mCodes.at(cri).blockCode.kind()) + "["
                + QString::number(mCodes.at(cri).blockCode.flavor()) + "], " + res;
    }
    return res;
}

void SyntaxHighlighter::syntaxDebug(const SyntaxBlock &syntaxBlock, const QString &syntaxName, int prevFlavor)
{
#ifdef SYNTAXDEBUG
    if (syntaxBlock.syntax)
        DEB() << QString(syntaxBlock.start, ' ') << QString(syntaxBlock.length(), '.') << " "
              << syntaxBlock.syntax->name() << " flav_" << prevFlavor << "  (tail from " << syntaxName << ")";
#else
    Q_UNUSED(syntaxBlock)
    Q_UNUSED(syntaxName)
    Q_UNUSED(prevFlavor)
#endif
}

void SyntaxHighlighter::syntaxDebug(const QString &text)
{
#ifdef SYNTAXDEBUG
    DEB() << text;
#else
    Q_UNUSED(text)
#endif
}


} // namespace syntax
} // namespace studio
} // namespace gams
