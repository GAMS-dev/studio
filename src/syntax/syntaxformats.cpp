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
#include "tool.h"
#include "exception.h"

namespace gams {
namespace studio {

QString syntaxStateName(SyntaxState state)
{
    static const QHash<int, QString> syntaxStateName {
        {static_cast<int>(SyntaxState::Standard), "Standard"},
        {static_cast<int>(SyntaxState::Directive), "Directive"},
        {static_cast<int>(SyntaxState::DirectiveBody), "DirectiveBody"},
        {static_cast<int>(SyntaxState::DirectiveComment), "DirectiveComment"},
        {static_cast<int>(SyntaxState::Title), "Title"},

        {static_cast<int>(SyntaxState::CommentLine), "CommentLine"},
        {static_cast<int>(SyntaxState::CommentBlock), "CommentBlock"},
        {static_cast<int>(SyntaxState::CommentEndline), "CommentEndline"},
        {static_cast<int>(SyntaxState::CommentInline), "CommentInline"},

        {static_cast<int>(SyntaxState::Semicolon), "Semicolon"},
        {static_cast<int>(SyntaxState::Comma), "Comma"},
        {static_cast<int>(SyntaxState::DeclarationSetType), "DeclarationSetType"},
        {static_cast<int>(SyntaxState::DeclarationVariableType), "DeclarationVariableType"},
        {static_cast<int>(SyntaxState::Declaration), "Declaration"},
        {static_cast<int>(SyntaxState::DeclarationTable), "DeclarationTable"},

        {static_cast<int>(SyntaxState::Identifier), "Identifier"},
        {static_cast<int>(SyntaxState::IdentifierDescription1), "IdentifierDescription1"},
        {static_cast<int>(SyntaxState::IdentifierDescription2), "IdentifierDescription2"},
        {static_cast<int>(SyntaxState::IdentifierAssignment), "IdentifierAssignment"},
        {static_cast<int>(SyntaxState::IdentifierAssignmentEnd), "IdentifierAssignmentEnd"},

        {static_cast<int>(SyntaxState::IdentifierTable), "IdentifierTable"},
        {static_cast<int>(SyntaxState::IdentifierTableDescription1), "IdentifierTableDescription1"},
        {static_cast<int>(SyntaxState::IdentifierTableDescription2), "IdentifierTableDescription2"},
        {static_cast<int>(SyntaxState::IdentifierTableAssignmentHead), "IdentifierTableAssignment"},
        {static_cast<int>(SyntaxState::IdentifierTableAssignmentRow), "IdentifierTableAssignmentEnd"},

        {static_cast<int>(SyntaxState::Reserved), "Reserved"},
        {static_cast<int>(SyntaxState::ReservedBody), "ReservedBody"},

        {static_cast<int>(SyntaxState::StateCount), "StateCount"},
    };
    return syntaxStateName.value(static_cast<int>(state), QString("--unassigned(%1)--").arg(static_cast<int>(state)));
}


SyntaxTransitions SyntaxAbstract::nextStates(bool emptyLine)
{
    if (emptyLine && !mEmptyLineStates.isEmpty()) return mEmptyLineStates;
    return mSubStates;
}

QTextCharFormat SyntaxAbstract::charFormatError()
{
    QTextCharFormat errorFormat(charFormat());
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    return errorFormat;
}

int SyntaxAbstract::stateToInt(SyntaxState _state)
{
    return static_cast<int>(_state);
}

SyntaxState SyntaxAbstract::intToState(int intState)
{
    int i = intState - QTextFormat::UserObject;
    if (i >= 0 && i < static_cast<int>(SyntaxState::StateCount))
        return static_cast<SyntaxState>(intState);
    else
        return SyntaxState::Standard;
}


SyntaxStandard::SyntaxStandard() : SyntaxAbstract(SyntaxState::Standard)
{
    mSubStates << SyntaxState::Semicolon
               << SyntaxState::CommentLine
               << SyntaxState::Declaration
               << SyntaxState::DeclarationSetType
               << SyntaxState::DeclarationVariableType
               << SyntaxState::DeclarationTable
               << SyntaxState::Directive
               << SyntaxState::Reserved;
}

SyntaxBlock SyntaxStandard::find(SyntaxState entryState, const QString& line, int index)
{
    static QVector<SyntaxState> invalidEntries {SyntaxState::Declaration, SyntaxState::DeclarationSetType,
                SyntaxState::DeclarationTable, SyntaxState::DeclarationVariableType};
    Q_UNUSED(entryState);
    int i = line.indexOf(';', index);
    bool error = invalidEntries.contains(entryState);
    if (i>-1)
        return SyntaxBlock(this, index, i+1, error);
    else
        return SyntaxBlock(this, index, line.length(), error);
}

SyntaxBlock SyntaxStandard::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, index, line.length(), SyntaxStateShift::shift);
}

SyntaxDirective::SyntaxDirective(QChar directiveChar) : SyntaxAbstract(SyntaxState::Directive)
{
    mRex.setPattern(QString("(^%1|%1%1)\\s*([\\w\\.]+)\\s*").arg(QRegularExpression::escape(directiveChar)));

    // TODO(JM) parse source file: src\gamscmex\gmsdco.gms or better create a lib that can be called to get the list
    QList<QPair<QString, QString>> data = SyntaxData::directives();
    for (const QPair<QString, QString> &list: data) {
        mDirectives << list.first;
        mDescription << list.second;
    }
    // offtext is checked separately, so remove it here
    int i = mDirectives.indexOf(QRegExp("offText", Qt::CaseInsensitive));
    if (i>0) {
        mDirectives.removeAt(i);
        mDescription.removeAt(i);
    }
    // !!! Enter special states always in lowercase
    mSpecialStates.insert(QString("title").toLower(), SyntaxState::Title);
    mSpecialStates.insert(QString("onText").toLower(), SyntaxState::CommentBlock);
    mSpecialStates.insert(QString("hidden").toLower(), SyntaxState::DirectiveComment);
}

SyntaxBlock SyntaxDirective::find(SyntaxState entryState, const QString& line, int index)
{
    QRegularExpressionMatch match = mRex.match(line, index);
    if (!match.hasMatch()) return SyntaxBlock(this);
    if (entryState == SyntaxState::CommentBlock) {
        if (match.captured(2).compare("offtext", Qt::CaseInsensitive)==0)
            return SyntaxBlock(this, match.capturedStart(1), match.capturedEnd(0), SyntaxStateShift::out);
        return SyntaxBlock(this);
    }
    SyntaxState next = mSpecialStates.value(match.captured(2).toLower(), SyntaxState::DirectiveBody);
    if (mDirectives.contains(match.captured(2), Qt::CaseInsensitive)) {
        bool atEnd = match.capturedEnd(0) >= line.length()-1;
        SyntaxStateShift shift = (atEnd && next != SyntaxState::CommentBlock) ? SyntaxStateShift::out : SyntaxStateShift::in;
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
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
}


SyntaxDirectiveBody::SyntaxDirectiveBody(SyntaxState state) : SyntaxAbstract(state)
{
    if (state != SyntaxState::DirectiveBody && state != SyntaxState::DirectiveComment && state != SyntaxState::Title)
        FATAL() << "invalid SyntaxState to initialize SyntaxDirectiveBody: " << syntaxStateName(state);
}

SyntaxBlock SyntaxDirectiveBody::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState);
    return SyntaxBlock(this, index, line.length(), SyntaxStateShift::out);
}

SyntaxBlock SyntaxDirectiveBody::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, index, line.length(), SyntaxStateShift::out);
}


SyntaxCommentLine::SyntaxCommentLine(QChar commentChar)
    : SyntaxAbstract(SyntaxState::CommentLine), mCommentChar(commentChar)
{ }

SyntaxBlock SyntaxCommentLine::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState)
    if (entryState == SyntaxState::CommentLine || (index==0 && line.startsWith(mCommentChar)))
        return SyntaxBlock(this, index, line.length(), false, SyntaxStateShift::skip);
    return SyntaxBlock();
}

SyntaxBlock SyntaxCommentLine::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, index, line.length(), SyntaxStateShift::out);
}


SyntaxCommentBlock::SyntaxCommentBlock() : SyntaxAbstract(SyntaxState::CommentBlock)
{
    mSubStates << SyntaxState::Directive;
}

SyntaxBlock SyntaxCommentBlock::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState)
    return SyntaxBlock(this, index, line.length());
}

SyntaxBlock SyntaxCommentBlock::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, index, line.length(), SyntaxStateShift::shift);
}

SyntaxDelimiter::SyntaxDelimiter(SyntaxState state)
    : SyntaxAbstract(state)
{
    if (state == SyntaxState::Semicolon) {
        mDelimiter = ';';
    } else if (state == SyntaxState::Comma) {
        mDelimiter = ',';
        mSubStates << SyntaxState::Identifier;
    } else FATAL() << "invalid SyntaxState to initialize SyntaxDelimiter: " << syntaxStateName(state);
}

SyntaxBlock SyntaxDelimiter::find(SyntaxState entryState, const QString &line, int index)
{
    Q_UNUSED(entryState)
    int end = index;
    while (isWhitechar(line, end)) end++;
    if (end < line.length() && line.at(end) == mDelimiter) {
        if (state() == SyntaxState::Semicolon)
            return SyntaxBlock(this, index, end+1, SyntaxStateShift::reset);
        return SyntaxBlock(this, index, end+1, SyntaxStateShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxDelimiter::validTail(const QString &line, int index, bool &hasContent)
{
    hasContent = false;
    int end = index;
    while (isWhitechar(line, end)) end++;
    if (state() == SyntaxState::Semicolon)
        return SyntaxBlock(this, index, end, SyntaxStateShift::reset);
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
}

} // namespace studio
} // namespace gams
