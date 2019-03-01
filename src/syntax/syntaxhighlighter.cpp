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
    : QSyntaxHighlighter(doc)
{
    QHash<ColorEnum, QColor> cl {
        {SyntaxDirex, QColor(Qt::darkMagenta).darker(120)},
        {SyntaxDiBdy, QColor(Qt::darkBlue).lighter(170)},
        {SyntaxComnt, QColor(80, 145, 75)},
        {SyntaxTitle, QColor(Qt::darkBlue).lighter(140)},
        {SyntaxKeywd, QColor(Qt::darkBlue).lighter(140)},
        {SyntaxDeclr, QColor(Qt::darkBlue).lighter(140)},
        {SyntaxIdent, QColor(Qt::black)},
        {SyntaxDescr, QColor(Qt::darkBlue).lighter(170)},
        {SyntaxAssgn, QColor(Qt::darkGreen).darker(140)},
        {SyntaxAsLab, QColor(Qt::darkGreen).darker(110)},
//        {SyntaxAsVal, QColor(Qt::darkCyan).darker(150)},
        {SyntaxAsVal, QColor(0, 80, 120)},
        {SyntaxTabHd, QColor(Qt::darkGreen).darker(140)},
        {SyntaxEmbed, QColor(200, 70, 0)},
    };
    // To visualize one format: add color index at start e.g. initKind(1, new SyntaxReservedBody());
    initKind(new SyntaxStandard(), Qt::red);
    initKind(new SyntaxDirective(), cl.value(SyntaxDirex));
    initKind(new SyntaxDirectiveBody(SyntaxKind::DirectiveBody), cl.value(SyntaxDiBdy));
    initKind(new SyntaxDirectiveBody(SyntaxKind::DirectiveComment), cl.value(SyntaxComnt), fItalic);
    initKind(new SyntaxDirectiveBody(SyntaxKind::Title), cl.value(SyntaxTitle), fBoldItalic);
    initKind(new SyntaxCommentLine(), cl.value(SyntaxComnt), fItalic);
    initKind(new SyntaxCommentBlock(), cl.value(SyntaxComnt), fItalic);

    initKind(new SyntaxDelimiter(SyntaxKind::Semicolon));
    initKind(new SyntaxDelimiter(SyntaxKind::Comma));
    initKind(new SyntaxReserved(), cl.value(SyntaxKeywd), fBold);
    initKind(new SyntaxReservedBody());
    initKind(new SyntaxEmbedded(SyntaxKind::Embedded), cl.value(SyntaxKeywd), fBold);
    initKind(new SyntaxEmbedded(SyntaxKind::EmbeddedEnd), cl.value(SyntaxKeywd), fBold);
    initKind(new SyntaxEmbeddedBody(), cl.value(SyntaxEmbed), fNormal);
    initKind(new SyntaxPreDeclaration(SyntaxKind::DeclarationSetType), cl.value(SyntaxDeclr), fBold);
    initKind(new SyntaxPreDeclaration(SyntaxKind::DeclarationVariableType), cl.value(SyntaxDeclr), fBold);
    initKind(new SyntaxDeclaration(), cl.value(SyntaxDeclr), fBold);
    initKind(new SyntaxDeclarationTable(), cl.value(SyntaxDeclr), fBold);

    initKind(new SyntaxIdentifier(SyntaxKind::Identifier));
    initKind(new SyntaxIdentDescript(SyntaxKind::IdentifierDescription1), cl.value(SyntaxDescr));
    initKind(new SyntaxIdentDescript(SyntaxKind::IdentifierDescription2), cl.value(SyntaxDescr));
    initKind(new SyntaxIdentAssign(SyntaxKind::IdentifierAssignment), cl.value(SyntaxAssgn));
    initKind(new AssignmentLabel(), cl.value(SyntaxAsLab));
    initKind(new AssignmentValue(), cl.value(SyntaxAsVal));
    initKind(new SyntaxIdentAssign(SyntaxKind::IdentifierAssignmentEnd), cl.value(SyntaxAssgn));

    initKind(new SyntaxIdentifier(SyntaxKind::IdentifierTable));
    initKind(new SyntaxIdentDescript(SyntaxKind::IdentifierTableDescription1), cl.value(SyntaxDescr));
    initKind(new SyntaxIdentDescript(SyntaxKind::IdentifierTableDescription2), cl.value(SyntaxDescr));

    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentHead), cl.value(SyntaxTabHd), fBold);
    initKind(new SyntaxTableAssign(SyntaxKind::IdentifierTableAssignmentRow), cl.value(SyntaxAssgn));

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
    int code = previousBlockState();
    if (code < 0) code = 0;
    int index = 0;
    QTextBlock textBlock = currentBlock();
    int posForSyntaxKind = mPositionForSyntaxKind - textBlock.position();
    if (posForSyntaxKind < 0) posForSyntaxKind = text.length();
    bool extendSearch = true;

    while (index < text.length()) {
        KindCode kindCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
        SyntaxAbstract* syntax = mKinds.at(kindCode.first);
        bool stack = true;
         // detect end of valid trailing characters for current syntax
        SyntaxBlock tailBlock = syntax->validTail(text, index, stack);
        if (stack) extendSearch = false;

        // HOWTO(JM) For kinds redefined with directives:
        //   - add new Syntax to mKinds
        //   - create a new full set of Syntax in mCodes with just the new one replaced
        // -> result: the top code will change from 0 to the new Standard top
        SyntaxBlock nextBlock;
        for (SyntaxKind nextKind: syntax->nextKinds(extendSearch)) {
            SyntaxAbstract* testSyntax = getSyntax(nextKind);
            if (testSyntax) {
                SyntaxBlock testBlock = testSyntax->find(syntax->kind(), text, index);
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
            extendSearch = false;
            if (tailBlock.isValid()) {
                if (nextBlock.start < tailBlock.end) tailBlock.end = nextBlock.start;
                if (tailBlock.isValid()) {
                    if (tailBlock.syntax->kind() != SyntaxKind::Standard) {
                        setFormat(tailBlock.start, tailBlock.length(), tailBlock.syntax->charFormat());
                        scanParentheses(text, tailBlock.start, tailBlock.length(), tailBlock.syntax->kind(), parPosList);
                    }
                    code = getCode(code, tailBlock.shift, getKindIdx(tailBlock.syntax->kind()), getKindIdx(tailBlock.next));
                }
            }
        }

        if (nextBlock.error && nextBlock.length() > 0) {
            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormatError());
        } else if (nextBlock.syntax->kind() != SyntaxKind::Standard) {
            setFormat(nextBlock.start, nextBlock.length(), nextBlock.syntax->charFormat());
            if (nextBlock.syntax->kind() == SyntaxKind::Semicolon) extendSearch = true;
        }
        scanParentheses(text, nextBlock.start, nextBlock.length(), nextBlock.syntax->kind(), parPosList);
        index = nextBlock.end;

        code = getCode(code, nextBlock.shift, getKindIdx(nextBlock.syntax->kind()), getKindIdx(nextBlock.next));

        if (posForSyntaxKind <= index) {
            mLastSyntaxKind = nextBlock.syntax->intSyntaxType();
            mPositionForSyntaxKind = -1;
            posForSyntaxKind = text.length()+1;
        }
    }
    // update BlockData
    if (!parPosList.isEmpty() || textBlock.userData()) {
        parPosList.squeeze();
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
    KindCode kindCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
    if (mKinds.at(kindCode.first)->kind() != SyntaxKind::Standard) {
        setCurrentBlockState(code);
    } else if (currentBlockState() != -1) {
        setCurrentBlockState(-1);
    }
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

const QVector<SyntaxKind> validParenthesesSyntax = {
    SyntaxKind::Standard,
    SyntaxKind::Identifier,
    SyntaxKind::IdentifierTable,
    SyntaxKind::IdentifierAssignment,
    SyntaxKind::IdentifierAssignmentEnd,
    SyntaxKind::IdentifierTableAssignmentHead,
    SyntaxKind::IdentifierTableAssignmentRow,
    SyntaxKind::Reserved,
    SyntaxKind::ReservedBody,
    SyntaxKind::EmbeddedBody,
};

const QString validParentheses("{[(}])/");
const QString specialBlocks("\"\'\"\'"); // ("[\"\']\"\'");

void SyntaxHighlighter::scanParentheses(const QString &text, int start, int len, SyntaxKind kind,  QVector<ParenthesesPos> &parentheses)
{
    bool inBlock = false;
    if (kind == SyntaxKind::Embedded) {
        parentheses << ParenthesesPos('E', start);
        return;
    }
    if (kind == SyntaxKind::EmbeddedEnd) {
        parentheses << ParenthesesPos('e', start);
        return;
    }
    if (!validParenthesesSyntax.contains(kind)) return;
    for (int i = start; i < start+len; ++i) {
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

void SyntaxHighlighter::initKind(int debug, SyntaxAbstract *syntax, QColor color, FontModifier fMod)
{
    if (debug) syntax->charFormat().setBackground(backColor(debug));

    syntax->charFormat().setProperty(QTextFormat::UserProperty, syntax->intSyntaxType());
    if (color.isValid()) syntax->charFormat().setForeground(color);
    if (fMod & fItalic) syntax->charFormat().setFontItalic(true);
    if (fMod & fBold) syntax->charFormat().setFontWeight(QFont::Bold);
    mKinds << syntax;
    addCode(mKinds.length()-1, 0);
}

void SyntaxHighlighter::initKind(SyntaxAbstract *syntax, QColor color, FontModifier fMod)
{
    initKind(false, syntax, color, fMod);
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

int SyntaxHighlighter::getCode(CodeIndex code, SyntaxShift shift, KindIndex kind, KindIndex kindNext)
{
    if (code < 0) code = 0;
    if (shift == SyntaxShift::skip)
        return code;
    if (shift == SyntaxShift::out)
        return mCodes.at(code).second;
    if (shift == SyntaxShift::in)
        return addCode(kindNext, code);
    if (shift == SyntaxShift::shift)
        return addCode(kind, mCodes.at(code).second);

    while (mKinds.at(mCodes.at(code).first)->kind() != SyntaxKind::Standard) {
        code = mCodes.at(code).second;
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


} // namespace studio
} // namespace gams
