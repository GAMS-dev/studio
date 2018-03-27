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
#include "syntaxidentifier.h"
#include "exception.h"

namespace gams {
namespace studio {

SyntaxIdentifier::SyntaxIdentifier(SyntaxState state) : SyntaxAbstract(state)
{
    mRex = QRegularExpression("[;\"\'/]");
    switch (state) {
    case SyntaxState::Identifier:
        mSubStates << SyntaxState::IdentifierDescription1
                   << SyntaxState::IdentifierDescription2
                   << SyntaxState::IdentifierAssignment;
        break;
    case SyntaxState::IdentifierTable:
        mSubStates << SyntaxState::IdentifierTableDescription1
                   << SyntaxState::IdentifierTableDescription2
                   << SyntaxState::IdentifierTableAssignment;
        break;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxIdentifier: " << syntaxStateName(state);
        break;
    }
    // sub-states to check for all types
    mSubStates << SyntaxState::Semicolon << SyntaxState::Directive << SyntaxState::CommentLine
               << SyntaxState::CommentEndline << SyntaxState::CommentInline;
}

SyntaxBlock SyntaxIdentifier::find(SyntaxState entryState, const QString& line, int index)
{
    bool commaOnly = false;
    if (state() == SyntaxState::Identifier && entryState >= state() && entryState <= SyntaxState::IdentifierAssignment)
        commaOnly = true;
    if (state() == SyntaxState::IdentifierTable && entryState >= state() && entryState <= SyntaxState::IdentifierTableAssignment)
        commaOnly = true;
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (commaOnly) {
        if (start < line.length() && line.at(start) == ',')
            return SyntaxBlock(this, start, start+1, SyntaxStateShift::shift);
        return SyntaxBlock(this);
    }
    if (start<line.length() && !mRex.match(line.midRef(start,1)).hasMatch())
        return SyntaxBlock(this, start, start+1, SyntaxStateShift::shift);
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentifier::validTail(const QString &line, int index)
{
    int end = line.indexOf(mRex, index);
    return SyntaxBlock(this, index, (end>0 ? end : line.length()), SyntaxStateShift::shift);
}

SyntaxIdentDescript::SyntaxIdentDescript(SyntaxState state) : SyntaxAbstract(state)
{
    QHash<int, QChar> delims {
        {static_cast<int>(SyntaxState::IdentifierDescription1), '\''},
        {static_cast<int>(SyntaxState::IdentifierDescription2), '\"'},
        {static_cast<int>(SyntaxState::IdentifierTableDescription1), '\''},
        {static_cast<int>(SyntaxState::IdentifierTableDescription2), '\"'},
    };
    mSubStates << SyntaxState::Semicolon << SyntaxState::Directive << SyntaxState::CommentLine
               << SyntaxState::CommentEndline << SyntaxState::CommentInline;

    switch (state) {
    case SyntaxState::IdentifierDescription1:
    case SyntaxState::IdentifierDescription2:
        mSubStates << SyntaxState::IdentifierAssignment << SyntaxState::Identifier;
        mTable = false;
        break;
    case SyntaxState::IdentifierTableDescription1:
    case SyntaxState::IdentifierTableDescription2:
        mSubStates << SyntaxState::IdentifierTableAssignment;
        mTable = true;
        break;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxIdentDescript: " << syntaxStateName(state);
        break;
    }
    if (!delims.contains(static_cast<int>(state)))
        FATAL() << "missing delimiter for state " << syntaxStateName(state);
    mDelimiter = delims.value(static_cast<int>(state));
}

SyntaxBlock SyntaxIdentDescript::find(SyntaxState entryState, const QString &line, int index)
{
    bool skip = mTable ? (entryState != SyntaxState::IdentifierTable) : (entryState != SyntaxState::Identifier);
    if (skip) SyntaxBlock(this);

    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start < line.length() && line.at(start) == mDelimiter) {
        int end = line.indexOf(mDelimiter, start+1);
        if (end > start)
            return SyntaxBlock(this, index, end+1, SyntaxStateShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentDescript::validTail(const QString &line, int index)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
}

SyntaxIdentAssign::SyntaxIdentAssign(SyntaxState state) : SyntaxAbstract(state)
{
    mSubStates << SyntaxState::Semicolon << SyntaxState::Directive << SyntaxState::CommentLine
               << SyntaxState::CommentEndline << SyntaxState::CommentInline;

    switch (state) {
    case SyntaxState::IdentifierAssignment:
        mSubStates << SyntaxState::IdentifierAssignmentEnd;
        break;
    case SyntaxState::IdentifierAssignmentEnd:
        mSubStates << SyntaxState::Identifier;
        break;
    case SyntaxState::IdentifierTableAssignment:
        // TODO(JM) special format for tables maybe other class!
        mSubStates << SyntaxState::IdentifierTableAssignmentEnd;
        break;
    case SyntaxState::IdentifierTableAssignmentEnd:
        // TODO(JM) special format for tables maybe other class!
        break;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxIdentDescript: " << syntaxStateName(state);
        break;
    }
    mDelimiter = '/';
}

SyntaxBlock SyntaxIdentAssign::find(SyntaxState entryState, const QString &line, int index)
{
    Q_UNUSED(entryState)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start < line.length() && line.at(start) == mDelimiter)
        return SyntaxBlock(this, index, start+1, SyntaxStateShift::shift);
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentAssign::validTail(const QString &line, int index)
{
    int end;
    if (state() == SyntaxState::IdentifierAssignmentEnd || state() == SyntaxState::IdentifierTableAssignmentEnd) {
        end = index;
        while (isWhitechar(line, end)) end++;
        return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
    }
    end = line.indexOf(mDelimiter, index);
    if (end < 0)
        return SyntaxBlock(this, index, line.length(), SyntaxStateShift::shift);
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
}

} // namespace studio
} // namespace gams
