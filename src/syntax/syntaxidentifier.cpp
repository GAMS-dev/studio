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

    // sub-states to check for all types
    mSubStates << SyntaxState::Semicolon << SyntaxState::Directive << SyntaxState::CommentLine
               << SyntaxState::CommentEndline << SyntaxState::CommentInline;

    switch (state) {
    case SyntaxState::Identifier:
        mEmptyLineStates << SyntaxState::Declaration << SyntaxState::DeclarationTable  << SyntaxState::Identifier;
        mSubStates << SyntaxState::IdentifierDescription1
                   << SyntaxState::IdentifierDescription2
                   << SyntaxState::IdentifierAssignment;
        break;
    case SyntaxState::IdentifierTable:
        mEmptyLineStates << SyntaxState::Declaration << SyntaxState::DeclarationTable  << SyntaxState::IdentifierTable;
        mSubStates << SyntaxState::IdentifierTableDescription1
                   << SyntaxState::IdentifierTableDescription2
                   << SyntaxState::IdentifierTableAssignmentHead;
        break;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxIdentifier: " << syntaxStateName(state);
        break;
    }
}

SyntaxBlock SyntaxIdentifier::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState);
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start<line.length() && !mRex.match(line.midRef(start,1)).hasMatch())
        return SyntaxBlock(this, start, start+1, SyntaxStateShift::shift);
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentifier::validTail(const QString &line, int index, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    int end = line.indexOf(mRex, index);
    if (end < 0) end = line.length();
    hasContent = start < end;
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
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
    mEmptyLineStates = mSubStates;

    switch (state) {
    case SyntaxState::IdentifierDescription1:
    case SyntaxState::IdentifierDescription2:
        mEmptyLineStates << SyntaxState::DeclarationSetType << SyntaxState::DeclarationVariableType
                         << SyntaxState::Declaration << SyntaxState::DeclarationTable
                         << SyntaxState::Comma << SyntaxState::IdentifierAssignment << SyntaxState::Identifier;
        mSubStates << SyntaxState::Comma << SyntaxState::IdentifierAssignment;
        mTable = false;
        break;
    case SyntaxState::IdentifierTableDescription1:
    case SyntaxState::IdentifierTableDescription2:
        mEmptyLineStates << SyntaxState::DeclarationSetType << SyntaxState::DeclarationVariableType
                         << SyntaxState::Declaration << SyntaxState::DeclarationTable
                         << SyntaxState::IdentifierTableAssignmentHead;
        mSubStates << SyntaxState::IdentifierTableAssignmentHead;
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

SyntaxBlock SyntaxIdentDescript::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
}

SyntaxIdentAssign::SyntaxIdentAssign(SyntaxState state) : SyntaxAbstract(state)
{
    mSubStates << SyntaxState::Semicolon << SyntaxState::Directive << SyntaxState::CommentLine
               << SyntaxState::CommentEndline << SyntaxState::CommentInline;

    mDelimiter = '/';
    switch (state) {
    case SyntaxState::IdentifierAssignment:
        mEmptyLineStates << SyntaxState::IdentifierAssignmentEnd;
        mSubStates << SyntaxState::IdentifierAssignmentEnd;
        break;
    case SyntaxState::IdentifierAssignmentEnd:
        mSubStates << SyntaxState::Comma;
        mEmptyLineStates = mSubStates << SyntaxState::Identifier;
        break;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxIdentAssign: " << syntaxStateName(state);
        break;
    }
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

SyntaxBlock SyntaxIdentAssign::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    if (state() == SyntaxState::IdentifierAssignmentEnd || state() == SyntaxState::IdentifierTableAssignmentRow) {
        hasContent = false;
        while (isWhitechar(line, end)) end++;
        return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
    }
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    end = line.indexOf(mDelimiter, start);
    if (end < 0) end = line.length();
    hasContent = (end > start);
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
}

SyntaxTableAssign::SyntaxTableAssign(SyntaxState state) : SyntaxAbstract(state)
{
    mSubStates << SyntaxState::Semicolon << SyntaxState::Directive << SyntaxState::CommentLine
               << SyntaxState::CommentEndline << SyntaxState::CommentInline;
    switch (state) {
    case SyntaxState::IdentifierTableAssignmentHead:
        mSubStates << SyntaxState::IdentifierTableAssignmentRow;
        break;
    case SyntaxState::IdentifierTableAssignmentRow:
        mSubStates << SyntaxState::IdentifierTableAssignmentHead;
        break;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxTableAssign: " << syntaxStateName(state);
        break;
    }
}

SyntaxBlock SyntaxTableAssign::find(SyntaxState entryState, const QString &line, int index)
{
    if (index > 0) return SyntaxBlock(this);

    if (state() == SyntaxState::IdentifierTableAssignmentHead
            && entryState == SyntaxState::IdentifierTableAssignmentRow) {
        int start = index;
        while (isWhitechar(line, start))
            ++start;
        if (start >= line.length() || line.at(start) != '+')
            return SyntaxBlock(this);
    }
    int end = line.indexOf(';', index);
    if (end < 0)
        return SyntaxBlock(this, index, line.length(), SyntaxStateShift::shift);
    return SyntaxBlock(this, index, end, SyntaxStateShift::out);
}

SyntaxBlock SyntaxTableAssign::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(hasContent)
    int end = line.indexOf(';', index);
    if (end < 0)
        return SyntaxBlock(this, index, line.length(), SyntaxStateShift::shift);
    return SyntaxBlock(this, index, end, SyntaxStateShift::out);
}

} // namespace studio
} // namespace gams
