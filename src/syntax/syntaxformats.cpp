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
#include "syntaxformats.h"
#include "logger.h"
#include "syntaxdata.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace syntax {

QString syntaxKindName(SyntaxKind kind)
{
    return QVariant::fromValue(kind).toString();
//    if (!res.isEmpty()) return res+"_auto";


//    static const QHash<int, QString> syntaxKindName {
//        {static_cast<int>(SyntaxKind::Standard), "Standard"},
//        {static_cast<int>(SyntaxKind::Directive), "Directive"},
//        {static_cast<int>(SyntaxKind::DirectiveBody), "DirectiveBody"},
//        {static_cast<int>(SyntaxKind::DirectiveComment), "DirectiveComment"},
//        {static_cast<int>(SyntaxKind::Title), "Title"},

//        {static_cast<int>(SyntaxKind::CommentLine), "CommentLine"},
//        {static_cast<int>(SyntaxKind::CommentBlock), "CommentBlock"},
//        {static_cast<int>(SyntaxKind::CommentEndline), "CommentEndline"},
//        {static_cast<int>(SyntaxKind::CommentInline), "CommentInline"},

//        {static_cast<int>(SyntaxKind::Semicolon), "Semicolon"},
//        {static_cast<int>(SyntaxKind::Comma), "Comma"},
//        {static_cast<int>(SyntaxKind::DeclarationSetType), "DeclarationSetType"},
//        {static_cast<int>(SyntaxKind::DeclarationVariableType), "DeclarationVariableType"},
//        {static_cast<int>(SyntaxKind::Declaration), "Declaration"},
//        {static_cast<int>(SyntaxKind::DeclarationTable), "DeclarationTable"},

//        {static_cast<int>(SyntaxKind::Identifier), "Identifier"},
//        {static_cast<int>(SyntaxKind::IdentifierDim1), "IdentifierDim1"},
//        {static_cast<int>(SyntaxKind::IdentifierDim2), "IdentifierDim2"},
//        {static_cast<int>(SyntaxKind::IdentifierDimEnd1), "IdentifierDimEnd1"},
//        {static_cast<int>(SyntaxKind::IdentifierDimEnd2), "IdentifierDimEnd2"},
//        {static_cast<int>(SyntaxKind::IdentifierDescription), "IdentifierDescription"},
//        {static_cast<int>(SyntaxKind::IdentifierAssignment), "IdentifierAssignment"},
//        {static_cast<int>(SyntaxKind::AssignmentLabel), "AssignmentLabel"},
//        {static_cast<int>(SyntaxKind::AssignmentValue), "AssignmentValue"},
//        {static_cast<int>(SyntaxKind::IdentifierAssignmentEnd), "IdentifierAssignmentEnd"},

//        {static_cast<int>(SyntaxKind::IdentifierTable), "IdentifierTable"},
//        {static_cast<int>(SyntaxKind::IdentifierTableDim1), "IdentifierTableDim1"},
//        {static_cast<int>(SyntaxKind::IdentifierTableDim2), "IdentifierTableDim2"},
//        {static_cast<int>(SyntaxKind::IdentifierTableDimEnd1), "IdentifierTableDimEnd1"},
//        {static_cast<int>(SyntaxKind::IdentifierTableDimEnd2), "IdentifierTableDimEnd2"},
//        {static_cast<int>(SyntaxKind::IdentifierTableDescription), "IdentifierTableDescription1"},
//        {static_cast<int>(SyntaxKind::IdentifierTableAssignmentHead), "IdentifierTableAssignment"},
//        {static_cast<int>(SyntaxKind::IdentifierTableAssignmentRow), "IdentifierTableAssignmentEnd"},

//        {static_cast<int>(SyntaxKind::Embedded), "Embedded"},
//        {static_cast<int>(SyntaxKind::EmbeddedBody), "EmbeddedBody"},
//        {static_cast<int>(SyntaxKind::EmbeddedEnd), "EmbeddedEnd"},
//        {static_cast<int>(SyntaxKind::Reserved), "Reserved"},
//        {static_cast<int>(SyntaxKind::ReservedBody), "ReservedBody"},

//        {static_cast<int>(SyntaxKind::KindCount), "KindCount"},
//    };
//    return syntaxKindName.value(static_cast<int>(kind), QString("--unassigned(%1)--").arg(static_cast<int>(kind)));
}


SyntaxTransitions SyntaxAbstract::nextKinds(bool emptyLine)
{
    if (emptyLine && !mEmptyLineKinds.isEmpty()) return mEmptyLineKinds;
    return mSubKinds;
}

QTextCharFormat SyntaxAbstract::charFormatError()
{
    QTextCharFormat errorFormat(charFormat());
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    return errorFormat;
}

int SyntaxAbstract::stateToInt(SyntaxKind _state)
{
    return static_cast<int>(_state);
}

SyntaxKind SyntaxAbstract::intToState(int intState)
{
    int i = intState - QTextFormat::UserObject;
    if (i >= 0 && i < static_cast<int>(SyntaxKind::KindCount))
        return static_cast<SyntaxKind>(intState);
    else
        return SyntaxKind::Standard;
}


SyntaxStandard::SyntaxStandard() : SyntaxAbstract(SyntaxKind::Standard)
{
    mSubKinds << SyntaxKind::Semicolon
               << SyntaxKind::CommentLine
               << SyntaxKind::Declaration
               << SyntaxKind::DeclarationSetType
               << SyntaxKind::DeclarationVariableType
               << SyntaxKind::DeclarationTable
               << SyntaxKind::Directive
               << SyntaxKind::Reserved
               << SyntaxKind::Embedded
               << SyntaxKind::Formula;
}

SyntaxBlock SyntaxStandard::find(SyntaxKind entryKind, const QString& line, int index)
{
    static QVector<SyntaxKind> invalidEntries {SyntaxKind::Declaration, SyntaxKind::DeclarationSetType,
                SyntaxKind::DeclarationTable, SyntaxKind::DeclarationVariableType};
    Q_UNUSED(entryKind);
    bool error = invalidEntries.contains(entryKind);
    int end = index;
    while (isKeywordChar(line, end)) end++;
    while (!isKeywordChar(line, end) && end < line.length()) end++;
    return SyntaxBlock(this, index, end, error);
}

SyntaxBlock SyntaxStandard::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(line);
    Q_UNUSED(index);
    Q_UNUSED(hasContent);
    return SyntaxBlock();
}

SyntaxDirective::SyntaxDirective(QChar directiveChar) : SyntaxAbstract(SyntaxKind::Directive)
{
    mRex.setPattern(QString("(^%1|%1%1)\\s*([\\w]+)\\s*").arg(QRegularExpression::escape(directiveChar)));

    // TODO(JM) parse source file: src\gamscmex\gmsdco.gms or better create a lib that can be called to get the list
    QList<QPair<QString, QString>> data = SyntaxData::directives();
    QStringList blockEndingDirectives;
    blockEndingDirectives << "offText" << "pauseEmbeddedCode" << "endEmbeddedCode" << "offEmbeddedCode";
    for (const QPair<QString, QString> &list: data) {
        if (blockEndingDirectives.contains(list.first)) {
            // block-ending directives are checked separately -> ignore here
            blockEndingDirectives.removeAll(list.first);
        } else {
            mDirectives << list.first;
            mDescription << list.second;
        }
    }
    if (!blockEndingDirectives.isEmpty()) {
        DEB() << "Initialization error in SyntaxDirective. Unknown directive(s): " << blockEndingDirectives.join(",");
    }
    // !!! Enter special kinds always in lowercase
    mSpecialKinds.insert(QString("title").toLower(), SyntaxKind::Title);
    mSpecialKinds.insert(QString("onText").toLower(), SyntaxKind::CommentBlock);
    mSpecialKinds.insert(QString("embeddedCode").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("embeddedCodeS").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("embeddedCodeV").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("continueEmbeddedCode").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("continueEmbeddedCodeS").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("continueEmbeddedCodeV").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("onEmbeddedCode").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("onEmbeddedCodeS").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("onEmbeddedCodeV").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("hidden").toLower(), SyntaxKind::DirectiveComment);
}

SyntaxBlock SyntaxDirective::find(SyntaxKind entryKind, const QString& line, int index)
{
    QRegularExpressionMatch match = mRex.match(line, index);
    if (!match.hasMatch()) return SyntaxBlock(this);
    if (entryKind == SyntaxKind::CommentBlock) {
        if (match.captured(2).compare("offtext", Qt::CaseInsensitive) == 0)
            return SyntaxBlock(this, match.capturedStart(1), match.capturedEnd(0), SyntaxShift::out);
        return SyntaxBlock(this);
    } else if (entryKind == SyntaxKind::EmbeddedBody) {
        if (match.captured(2).compare("pauseembeddedcode", Qt::CaseInsensitive) == 0
                || match.captured(2).compare("endembeddedcode", Qt::CaseInsensitive) == 0
                || match.captured(2).compare("offembeddedcode", Qt::CaseInsensitive) == 0)
            return SyntaxBlock(this, match.capturedStart(1), match.capturedEnd(0), SyntaxShift::out);
        return SyntaxBlock(this);
    }
    SyntaxKind next = mSpecialKinds.value(match.captured(2).toLower(), SyntaxKind::DirectiveBody);
    if (mDirectives.contains(match.captured(2), Qt::CaseInsensitive)) {
        bool atEnd = match.capturedEnd(0) >= line.length()-1;
        SyntaxShift shift = (atEnd && next != SyntaxKind::CommentBlock) ? SyntaxShift::out : SyntaxShift::in;
        return SyntaxBlock(this, match.capturedStart(1), match.capturedEnd(0), false, shift, next);
    } else {
        return SyntaxBlock(this, match.capturedStart(1), match.capturedEnd(0), next, true);
    }
}

SyntaxBlock SyntaxDirective::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = false;
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}


SyntaxDirectiveBody::SyntaxDirectiveBody(SyntaxKind kind) : SyntaxAbstract(kind)
{
    if (kind != SyntaxKind::DirectiveBody && kind != SyntaxKind::DirectiveComment && kind != SyntaxKind::Title)
        FATAL() << "invalid SyntaxKind to initialize SyntaxDirectiveBody: " << syntaxKindName(kind);
}

SyntaxBlock SyntaxDirectiveBody::find(SyntaxKind entryKind, const QString& line, int index)
{
    Q_UNUSED(entryKind);
    return SyntaxBlock(this, index, line.length(), SyntaxShift::out);
}

SyntaxBlock SyntaxDirectiveBody::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, index, line.length(), SyntaxShift::out);
}


SyntaxCommentLine::SyntaxCommentLine(QChar commentChar)
    : SyntaxAbstract(SyntaxKind::CommentLine), mCommentChar(commentChar)
{ }

SyntaxBlock SyntaxCommentLine::find(SyntaxKind entryKind, const QString& line, int index)
{
    Q_UNUSED(entryKind)
    if (entryKind == SyntaxKind::CommentLine || (index==0 && line.startsWith(mCommentChar)))
        return SyntaxBlock(this, index, line.length(), false, SyntaxShift::skip);
    return SyntaxBlock();
}

SyntaxBlock SyntaxCommentLine::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, index, line.length(), SyntaxShift::out);
}


SyntaxCommentBlock::SyntaxCommentBlock() : SyntaxAbstract(SyntaxKind::CommentBlock)
{
    mSubKinds << SyntaxKind::Directive;
}

SyntaxBlock SyntaxCommentBlock::find(SyntaxKind entryKind, const QString& line, int index)
{
    Q_UNUSED(entryKind)
    return SyntaxBlock(this, index, line.length());
}

SyntaxBlock SyntaxCommentBlock::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, index, line.length(), SyntaxShift::shift);
}

SyntaxDelimiter::SyntaxDelimiter(SyntaxKind kind)
    : SyntaxAbstract(kind)
{
    if (kind == SyntaxKind::Semicolon) {
        mDelimiter = ';';
    } else if (kind == SyntaxKind::Comma) {
        mDelimiter = ',';
        mSubKinds << SyntaxKind::Identifier;
    } else FATAL() << "invalid SyntaxKind to initialize SyntaxDelimiter: " << syntaxKindName(kind);
}

SyntaxBlock SyntaxDelimiter::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int end = index;
    while (isWhitechar(line, end)) end++;
    if (end < line.length() && line.at(end) == mDelimiter) {
        if (kind() == SyntaxKind::Semicolon)
            return SyntaxBlock(this, index, end+1, SyntaxShift::reset);
        return SyntaxBlock(this, index, end+1, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxDelimiter::validTail(const QString &line, int index, bool &hasContent)
{
    hasContent = false;
    int end = index;
    while (isWhitechar(line, end)) end++;
    if (kind() == SyntaxKind::Semicolon)
        return SyntaxBlock(this, index, end, SyntaxShift::reset);
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

SyntaxFormula::SyntaxFormula() : SyntaxAbstract(SyntaxKind::Formula)
{
    mSubKinds << SyntaxKind::Embedded << SyntaxKind::Semicolon << SyntaxKind::Reserved
              << SyntaxKind::CommentLine << SyntaxKind::CommentEndline << SyntaxKind::CommentInline
              << SyntaxKind::String << SyntaxKind::Directive << SyntaxKind::Assignment
              << SyntaxKind::Declaration << SyntaxKind::DeclarationSetType
              << SyntaxKind::DeclarationVariableType << SyntaxKind::DeclarationTable
              << SyntaxKind::Formula;
}

int SyntaxFormula::canBreak(QChar ch, int &prev) {

    // ASCII:   "   $   '   .   0  9   ;   =   A  Z   _   a   z
    // Code:   34, 36, 39, 46, 48-57, 59, 61, 65-90, 95, 97-122
    static QVector<QChar> cList = {'"', '$', '\'', '.', ';', '='};  // other breaking kind

    if (ch < '"' || ch > 'z')
        prev = 0;
    else if (ch >= 'a' || (ch >= 'A' && ch <= 'Z') || ch == '_')
        prev = 2;  // break by keyword kind
    else if (ch >= '0' && ch <= '9') {
        if (prev!=2) prev = 0;
    } else prev = cList.contains(ch) ? 1 : 0;
    return prev;
}

SyntaxBlock SyntaxFormula::find(const SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    int prev = 0;
    bool skipWord = (canBreak(line.at(start), prev) == 2);
    int end = start;
    while (++end < line.length()) {
        int chKind = canBreak(line.at(end), prev);
        if (chKind == 1) break;
        if (chKind != 2) skipWord = false;
        else if (!skipWord) break;
    }
    return SyntaxBlock(this, start, end, SyntaxShift::shift);
}

SyntaxBlock SyntaxFormula::validTail(const QString &line, int index, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    int end = start;
    if (end >= line.length()) return SyntaxBlock(this);
    int prev = 0;
    int cb1 = end < line.length() ? canBreak(line.at(end), prev) : 0;
    while (++end < line.length() && canBreak(line.at(end), prev) == cb1) ;
    hasContent = false;
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

SyntaxString::SyntaxString()
    : SyntaxAbstract(SyntaxKind::String)
{}

SyntaxBlock SyntaxString::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    int start = index;
    while (isWhitechar(line, start)) start++;
    int end = start;
    if (start < line.length() && (line.at(start) == '\'' || line.at(start) == '"')) {
        while (++end < line.length() && line.at(end) != line.at(start)) ;
        if (line.at(end) == line.at(start)) return SyntaxBlock(this, start, end+1, SyntaxShift::skip);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxString::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(hasContent)
    return SyntaxBlock(this);
}

SyntaxAssign::SyntaxAssign()
    : SyntaxAbstract(SyntaxKind::Assignment)
{}

SyntaxBlock SyntaxAssign::find(gams::studio::syntax::SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start)) start++;
    if (start >= line.length()) return SyntaxBlock(this);
    if (line.at(start) == '.') {
        if (line.length() > start+1 || line.at(start) == '.')
            return SyntaxBlock(this, start, start+2, SyntaxShift::skip);
    } else if (line.at(start) == '=') {
        if (line.length() <= start+2 || line.at(start+2) != '=')
            return SyntaxBlock(this, start, start+1, SyntaxShift::skip);
        if (line.length() >= start+2) {
            bool error = (QString("eglnxcb").indexOf(line.at(start+1), 0, Qt::CaseInsensitive) < 0);
            return SyntaxBlock(this, start, start+3, error, SyntaxShift::skip);
        }
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxAssign::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(hasContent)
    return SyntaxBlock(this);
}

} // namespace syntax
} // namespace studio
} // namespace gams
