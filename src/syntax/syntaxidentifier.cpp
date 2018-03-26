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

SyntaxIdentifier::SyntaxIdentifier(SyntaxState state) : mState(state)
{
    switch (state) {
    case SyntaxState::IdentifierDone:
        mSubStates << SyntaxState::Declaration << SyntaxState::DeclarationTable << SyntaxState::DeclarationSetType
                   << SyntaxState::DeclarationVariableType;
    case SyntaxState::Identifier:
        mSubStates << SyntaxState::IdentifierDescription << SyntaxState::IdentifierAssignment << SyntaxState::Directive
                   << SyntaxState::CommentLine << SyntaxState::CommentEndline << SyntaxState::CommentInline;
        break;

    case SyntaxState::IdentifierTableDone:
        mSubStates << SyntaxState::Declaration << SyntaxState::DeclarationTable << SyntaxState::DeclarationSetType
                   << SyntaxState::DeclarationVariableType;
    case SyntaxState::IdentifierTable:
        mSubStates << SyntaxState::IdentifierTableDescription << SyntaxState::IdentifierTableAssignment
                   << SyntaxState::Directive << SyntaxState::CommentLine << SyntaxState::CommentEndline
                   << SyntaxState::CommentInline;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxDeclaration: " << syntaxStateName(state);
        break;
    }
}

SyntaxBlock SyntaxIdentifier::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    int end = line.indexOf(';', start);
    if (end >= 0) return SyntaxBlock(this, start, end, SyntaxStateShift::out);
    return SyntaxBlock(this, start, line.length(), SyntaxStateShift::shift);
}

SyntaxBlock SyntaxIdentifier::validTail(const QString &line, int index)
{
    Q_UNUSED(index)
    return SyntaxBlock(this, index, line.length(), SyntaxStateShift::out);
}

} // namespace studio
} // namespace gams
