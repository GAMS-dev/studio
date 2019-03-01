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
#include "logger.h"

namespace gams {
namespace studio {

SyntaxIdentifier::SyntaxIdentifier(SyntaxKind kind) : SyntaxAbstract(kind)
{
    mRex = QRegularExpression("[;\"\'/]");

    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;

    switch (kind) {
    case SyntaxKind::Identifier:
        mSubKinds << SyntaxKind::Comma
                   << SyntaxKind::IdentifierDescription1
                   << SyntaxKind::IdentifierDescription2
                   << SyntaxKind::IdentifierAssignment;
        mEmptyLineKinds << mSubKinds
                         << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                         << SyntaxKind::Declaration << SyntaxKind::DeclarationTable;
        break;
    case SyntaxKind::IdentifierTable:
        mSubKinds << SyntaxKind::IdentifierTableDescription1
                   << SyntaxKind::IdentifierTableDescription2
                   << SyntaxKind::IdentifierTableAssignmentHead;
        mEmptyLineKinds << mSubKinds
                         << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                         << SyntaxKind::Declaration << SyntaxKind::DeclarationTable;
        break;
    default:
        FATAL() << "invalid SyntaxKind to initialize SyntaxIdentifier: " << syntaxKindName(kind);
    }
}

SyntaxBlock SyntaxIdentifier::find(SyntaxKind entryKind, const QString& line, int index)
{
    Q_UNUSED(entryKind);
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start<line.length() && !mRex.match(line.midRef(start,1)).hasMatch())
        return SyntaxBlock(this, start, start+1, SyntaxShift::shift);
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
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

SyntaxIdentDescript::SyntaxIdentDescript(SyntaxKind kind) : SyntaxAbstract(kind)
{
    QHash<int, QChar> delims {
        {static_cast<int>(SyntaxKind::IdentifierDescription1), '\''},
        {static_cast<int>(SyntaxKind::IdentifierDescription2), '\"'},
        {static_cast<int>(SyntaxKind::IdentifierTableDescription1), '\''},
        {static_cast<int>(SyntaxKind::IdentifierTableDescription2), '\"'},
    };
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mEmptyLineKinds = mSubKinds;

    switch (kind) {
    case SyntaxKind::IdentifierDescription1:
    case SyntaxKind::IdentifierDescription2:
        mEmptyLineKinds << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                         << SyntaxKind::Declaration << SyntaxKind::DeclarationTable
                         << SyntaxKind::Comma << SyntaxKind::IdentifierAssignment << SyntaxKind::Identifier;
        mSubKinds << SyntaxKind::Comma << SyntaxKind::IdentifierAssignment;
        mTable = false;
        break;
    case SyntaxKind::IdentifierTableDescription1:
    case SyntaxKind::IdentifierTableDescription2:
        mEmptyLineKinds << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                         << SyntaxKind::Declaration << SyntaxKind::DeclarationTable
                         << SyntaxKind::IdentifierTableAssignmentHead;
        mSubKinds << SyntaxKind::IdentifierTableAssignmentHead;
        mTable = true;
        break;
    default:
        FATAL() << "invalid SyntaxKind to initialize SyntaxIdentDescript: " << syntaxKindName(kind);
    }
    if (!delims.contains(static_cast<int>(kind)))
        FATAL() << "missing delimiter for syntax-kind " << syntaxKindName(kind);
    mDelimiter = delims.value(static_cast<int>(kind));
}

SyntaxBlock SyntaxIdentDescript::find(SyntaxKind entryKind, const QString &line, int index)
{
    bool skip = mTable ? (entryKind != SyntaxKind::IdentifierTable) : (entryKind != SyntaxKind::Identifier);
    if (skip) SyntaxBlock(this);

    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start < line.length() && line.at(start) == mDelimiter) {
        int end = line.indexOf(mDelimiter, start+1);
        if (end > start)
            return SyntaxBlock(this, index, end+1, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentDescript::validTail(const QString &line, int index, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

SyntaxIdentAssign::SyntaxIdentAssign(SyntaxKind kind) : SyntaxAbstract(kind)
{
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;

    mDelimiter = '/';
    switch (kind) {
    case SyntaxKind::IdentifierAssignment:
        mSubKinds << SyntaxKind::IdentifierAssignmentEnd << SyntaxKind::AssignmentLabel;
        mEmptyLineKinds = mSubKinds;
        break;
    case SyntaxKind::IdentifierAssignmentEnd:
        mSubKinds << SyntaxKind::Comma;
        mEmptyLineKinds = mSubKinds << SyntaxKind::Identifier;
        break;
    default:
        FATAL() << "invalid SyntaxKind to initialize SyntaxIdentAssign: " << syntaxKindName(kind);
    }
}

SyntaxBlock SyntaxIdentAssign::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    bool inside = (kind() != SyntaxKind::IdentifierAssignmentEnd
                   && (entryKind == SyntaxKind::AssignmentLabel
                       || entryKind == SyntaxKind::AssignmentValue));
    QChar delim = inside ? ',' : mDelimiter;
    while (isWhitechar(line, start))
        ++start;
    if (start < line.length() && line.at(start) == delim)
        return SyntaxBlock(this, start, start+1, SyntaxShift::shift);
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentAssign::validTail(const QString &line, int index, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    int end = (line.length() > start) ? start+1 : start;
    while (isWhitechar(line, end) || (line.length() > end && line.at(end) == ',')) end++;
    hasContent = (end > start);
    return SyntaxBlock(this, start, end, SyntaxShift::shift);
}

AssignmentLabel::AssignmentLabel()
     : SyntaxAbstract(SyntaxKind::AssignmentLabel)
{
    mSubKinds << SyntaxKind::IdentifierAssignmentEnd << SyntaxKind::IdentifierAssignment
               << SyntaxKind::AssignmentLabel << SyntaxKind::AssignmentValue ;
}

SyntaxBlock AssignmentLabel::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start)) start++;
    if (start >= line.size()) return SyntaxBlock(this);
    if (entryKind == SyntaxKind::AssignmentLabel && index != 0) {
        return SyntaxBlock(this);
    }

    // get delimiters
    QString delim("\"\'");
    QString special("/, .:");
    int end = start;
    int pos = start;
    while (pos < line.length()) {
        end = pos;
        // we are at the first non-white-char
        if (int quoteKind = delim.indexOf(line.at(pos))+1) {
            // find matching quote
            end = line.indexOf(delim.at(quoteKind-1), pos+1);
            if (end < 0)
                return SyntaxBlock(this, start, pos+1, SyntaxShift::in, true);
            pos = end+1;
        } else {
            while (++pos < line.length() && !special.contains(line.at(pos))) end = pos;
        }
        ++end;
        // if no dot or colon follows, finish
        while (isWhitechar(line,pos)) ++pos;
        if (pos < line.length() && special.indexOf(line.at(pos)) < 3) break;
        ++pos;
        while (isWhitechar(line,pos)) ++pos;
        end = pos;
    }

    if (end > start) {
        return SyntaxBlock(this, start, end, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock AssignmentLabel::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(line);
    Q_UNUSED(index);
    Q_UNUSED(hasContent);
    return SyntaxBlock();
}

AssignmentValue::AssignmentValue()
    : SyntaxAbstract(SyntaxKind::AssignmentValue)
{
    mSubKinds << SyntaxKind::IdentifierAssignment << SyntaxKind::IdentifierAssignmentEnd;
}

SyntaxBlock AssignmentValue::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index+1;
    while (isWhitechar(line, start)) start++;
    if (start >= line.size()) return SyntaxBlock(this);

    // get delimiters
    QString delim("\"\'[");
    QString special("/,");
    int end = start;
    int pos = start;
    // we are at the first non-white-char
    if (int quoteKind = delim.indexOf(line.at(pos))+1) {
        // find matching quote
        QChar ch = quoteKind==3 ? ']' : delim.at(quoteKind-1);
        end = line.indexOf(ch, pos+1);
        if (end < 0)
            return SyntaxBlock(this, start, pos+1, SyntaxShift::out, true);
        pos = end+1;
    } else {
        while (++pos < line.length() && !special.contains(line.at(pos))) end = pos;
    }
    end = pos;
//    while (isWhitechar(line, pos)) ++pos;

    if (end > start) {
        return SyntaxBlock(this, start, end, SyntaxShift::skip);
    }
    return SyntaxBlock(this);
}

SyntaxBlock AssignmentValue::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(line);
    Q_UNUSED(index);
    Q_UNUSED(hasContent);
    return SyntaxBlock();
}

SyntaxTableAssign::SyntaxTableAssign(SyntaxKind kind) : SyntaxAbstract(kind)
{
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    switch (kind) {
    case SyntaxKind::IdentifierTableAssignmentHead:
        mSubKinds << SyntaxKind::IdentifierTableAssignmentRow;
        break;
    case SyntaxKind::IdentifierTableAssignmentRow:
        mSubKinds << SyntaxKind::IdentifierTableAssignmentHead;
        break;
    default:
        FATAL() << "invalid SyntaxKind to initialize SyntaxTableAssign: " << syntaxKindName(kind);
    }
}

SyntaxBlock SyntaxTableAssign::find(SyntaxKind entryKind, const QString &line, int index)
{
    if (index > 0) return SyntaxBlock(this);

    if (kind() == SyntaxKind::IdentifierTableAssignmentHead
            && entryKind == SyntaxKind::IdentifierTableAssignmentRow) {
        int start = index;
        while (isWhitechar(line, start))
            ++start;
        if (start >= line.length() || line.at(start) != '+')
            return SyntaxBlock(this);
    }
    int end = line.indexOf(';', index);
    if (end < 0)
        return SyntaxBlock(this, index, line.length(), SyntaxShift::shift);
    return SyntaxBlock(this, index, end, SyntaxShift::out);
}

SyntaxBlock SyntaxTableAssign::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(hasContent)
    int end = line.indexOf(';', index);
    if (end < 0)
        return SyntaxBlock(this, index, line.length(), SyntaxShift::shift);
    return SyntaxBlock(this, index, end, SyntaxShift::out);
}

} // namespace studio
} // namespace gams
