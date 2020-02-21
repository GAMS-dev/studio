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
#include "syntaxidentifier.h"
#include "exception.h"
#include "logger.h"

namespace gams {
namespace studio {
namespace syntax {

SyntaxIdentifier::SyntaxIdentifier(SyntaxKind kind) : SyntaxAbstract(kind)
{
    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;

    switch (kind) {
    case SyntaxKind::Identifier:
        mSubKinds << SyntaxKind::CommaIdent
                  << SyntaxKind::IdentifierDim1
                  << SyntaxKind::IdentifierDim2
                  << SyntaxKind::IdentifierAssignment;
        mEmptyLineKinds << mSubKinds
                        << SyntaxKind::Identifier
                        << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                        << SyntaxKind::Declaration << SyntaxKind::DeclarationTable;
        mSubKinds << SyntaxKind::IdentifierDescription;  // must not exist in emptyLineKinds
        break;
    case SyntaxKind::IdentifierTable:
        mSubKinds << SyntaxKind::CommaTable
                  << SyntaxKind::IdentifierTableDim1
                  << SyntaxKind::IdentifierTableDim2
                  << SyntaxKind::IdentifierTableAssignmentHead;
        mEmptyLineKinds << mSubKinds
                         << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                         << SyntaxKind::Declaration << SyntaxKind::DeclarationTable;
        mSubKinds << SyntaxKind::IdentifierTableDescription;  // must not exist in emptyLineKinds
        break;
    default:
        Q_ASSERT_X(false, "SyntaxIdentifier", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
}

int SyntaxIdentifier::identChar(const QChar &c) const
{
    if (c >= 'A' && c <= 'z' && (c >= 'a' || c <= 'Z')) return 2;   // valid start identifier letter
    if ((c >= '0' && c <= '9') || c == '_') return 1;               // valid next identifier letter
    return 0;                                                       // no valid identifier letter
}

SyntaxBlock SyntaxIdentifier::find(const SyntaxKind entryKind, const QString& line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    int end = start;
    if (identChar(line.at(end++)) != 2) return SyntaxBlock(this);
    while (end < line.length()) {
        if (!identChar(line.at(end++))) return SyntaxBlock(this, start, end-1, SyntaxShift::shift);
    }
    return SyntaxBlock(this, start, end, SyntaxShift::shift);
}

SyntaxBlock SyntaxIdentifier::validTail(const QString &line, int index, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    hasContent = false;
    return SyntaxBlock(this, index, start, SyntaxShift::shift);
}

SyntaxIdentifierDim::SyntaxIdentifierDim(SyntaxKind kind) : SyntaxAbstract(kind)
{
    QHash<int, QChar> delims {
        {static_cast<int>(SyntaxKind::IdentifierDim1), '('},
        {static_cast<int>(SyntaxKind::IdentifierDim2), '['},
        {static_cast<int>(SyntaxKind::IdentifierTableDim1), '('},
        {static_cast<int>(SyntaxKind::IdentifierTableDim2), '['},
    };
    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;

    switch (kind) {
    case SyntaxKind::IdentifierDim1:
    case SyntaxKind::IdentifierDim2:
        mSubKinds << SyntaxKind::IdentifierDimEnd1
                  << SyntaxKind::IdentifierDimEnd2;
        mEmptyLineKinds << mSubKinds;
        mTable = false;
        break;
    case SyntaxKind::IdentifierTableDim1:
    case SyntaxKind::IdentifierTableDim2:
        mSubKinds << SyntaxKind::IdentifierTableDimEnd1
                  << SyntaxKind::IdentifierTableDimEnd2;
        mEmptyLineKinds << mSubKinds;
        mTable = true;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxIdentifierDim", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
    if (!delims.contains(static_cast<int>(kind)))
        Q_ASSERT_X(false, "SyntaxIdentifierDim", QString("missing delimiter for SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    mDelimiterIn = delims.value(static_cast<int>(kind));
    mDelimiterOut = (mDelimiterIn == '(') ? ')' : ']';
}

SyntaxBlock SyntaxIdentifierDim::find(const SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    if (line.at(start) != mDelimiterIn) return SyntaxBlock(this);
    return SyntaxBlock(this, start, start+1, SyntaxShift::shift);
}

SyntaxBlock SyntaxIdentifierDim::validTail(const QString &line, int index, bool &hasContent)
{
    const QString invalid("([])");
    int end = index-1;
    // inside valid identifier dimension
    while (++end < line.length()) {
        if (line.at(end--) == mDelimiterOut) break;
        if (invalid.indexOf(line.at(++end)) >= 0)
            return SyntaxBlock(this, index, end, SyntaxShift::out, true);
    }
    hasContent = index < end;
    return SyntaxBlock(this, index, end+1, SyntaxShift::shift);
}

SyntaxIdentifierDimEnd::SyntaxIdentifierDimEnd(SyntaxKind kind) : SyntaxAbstract(kind)
{
    QHash<int, QChar> delims {
        {static_cast<int>(SyntaxKind::IdentifierDimEnd1), ')'},
        {static_cast<int>(SyntaxKind::IdentifierDimEnd2), ']'},
        {static_cast<int>(SyntaxKind::IdentifierTableDimEnd1), ')'},
        {static_cast<int>(SyntaxKind::IdentifierTableDimEnd2), ']'},
    };
    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;

    switch (kind) {
    case SyntaxKind::IdentifierDimEnd1:
    case SyntaxKind::IdentifierDimEnd2:
        mSubKinds << SyntaxKind::CommaIdent << SyntaxKind::Semicolon
                  << SyntaxKind::IdentifierAssignment;
        mEmptyLineKinds << mSubKinds
                        << SyntaxKind::Identifier
                        << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                        << SyntaxKind::Declaration << SyntaxKind::DeclarationTable;
        mSubKinds << SyntaxKind::IdentifierDescription; // must not exist in emptyLineKinds
        mTable = false;
        break;
    case SyntaxKind::IdentifierTableDimEnd1:
    case SyntaxKind::IdentifierTableDimEnd2:
        mSubKinds  << SyntaxKind::CommaTable << SyntaxKind::Semicolon
                   << SyntaxKind::IdentifierTableAssignmentHead;
        mEmptyLineKinds << mSubKinds
                        << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                        << SyntaxKind::Declaration << SyntaxKind::DeclarationTable;
        mSubKinds << SyntaxKind::IdentifierTableDescription; // must not exist in emptyLineKinds
        mTable = true;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxIdentifierDimEnd", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
    if (!delims.contains(static_cast<int>(kind)))
        Q_ASSERT_X(false, "SyntaxIdentifierDimEnd", QString("missing delimiter for SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    mDelimiter = delims.value(static_cast<int>(kind));
}

SyntaxBlock SyntaxIdentifierDimEnd::find(const SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length() || line.at(start) != mDelimiter)  return SyntaxBlock(this);
    return SyntaxBlock(this, index, qMin(start+1, line.length()), SyntaxShift::shift);
}

SyntaxBlock SyntaxIdentifierDimEnd::validTail(const QString &line, int index, bool &hasContent)
{
//    if (index == 0) return SyntaxBlock(this);
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

SyntaxIdentDescript::SyntaxIdentDescript(SyntaxKind kind) : SyntaxAbstract(kind)
{
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mEmptyLineKinds = mSubKinds;

    switch (kind) {
    case SyntaxKind::IdentifierDescription:
        mEmptyLineKinds << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                         << SyntaxKind::Declaration << SyntaxKind::DeclarationTable
                         << SyntaxKind::IdentifierAssignment << SyntaxKind::Identifier;
        mSubKinds << SyntaxKind::CommaIdent << SyntaxKind::Semicolon << SyntaxKind::IdentifierAssignment;
        mTable = false;
        break;
    case SyntaxKind::IdentifierTableDescription:
        mEmptyLineKinds << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                         << SyntaxKind::Declaration << SyntaxKind::DeclarationTable
                         << SyntaxKind::IdentifierTableAssignmentHead;
        mSubKinds << SyntaxKind::CommaTable << SyntaxKind::Semicolon << SyntaxKind::IdentifierTableAssignmentHead;
        mTable = true;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxIdentDescript", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
}

SyntaxBlock SyntaxIdentDescript::find(const SyntaxKind entryKind, const QString &line, int index)
{
    if (index == 0) return SyntaxBlock(this);
    if (mTable) {
        if (entryKind != SyntaxKind::IdentifierTable
                && entryKind != SyntaxKind::IdentifierTableDimEnd1
                && entryKind != SyntaxKind::IdentifierTableDimEnd2)
            return SyntaxBlock(this);
    } else {
        if (entryKind != SyntaxKind::Identifier
                && entryKind != SyntaxKind::IdentifierDimEnd1
                && entryKind != SyntaxKind::IdentifierDimEnd2)
            return SyntaxBlock(this);
    }
    const QString invalidFirstChar("/([;");

    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length() || invalidFirstChar.contains(line.at(start))) return SyntaxBlock(this);
    QChar delim = line.at(start);
    if (delim != '\"' && delim != '\'') delim = '/';
    int end = start;
    int lastNonWhite = start;
    while (++end < line.length()) {
        if (line.at(end) == delim) return SyntaxBlock(this, start, end+(delim=='/'?0:1), SyntaxShift::shift);
        if (delim == '/' && line.at(end) == ';') break;
        if (delim == '/' && line.at(end) == ',') break;
        if (!isWhitechar(line, end)) lastNonWhite = end;
    }
    return SyntaxBlock(this, start, lastNonWhite+1, SyntaxShift::shift, (delim != '/'));
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
        mSubKinds << SyntaxKind::CommaIdent;
        mEmptyLineKinds = mSubKinds << SyntaxKind::Identifier;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxIdentAssign", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
}

SyntaxBlock SyntaxIdentAssign::find(const SyntaxKind entryKind, const QString &line, int index)
{
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
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

AssignmentLabel::AssignmentLabel()
     : SyntaxAbstract(SyntaxKind::AssignmentLabel)
{
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierAssignmentEnd << SyntaxKind::IdentifierAssignment
              << SyntaxKind::AssignmentLabel << SyntaxKind::AssignmentValue ;
}

SyntaxBlock AssignmentLabel::find(const SyntaxKind entryKind, const QString &line, int index)
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
    QString special("/, .:*()\"\'");
    int end = start;
    int pos = start;
    while (pos < line.length()) {
        end = pos;
        // we are at the first non-white-char
        if (int quoteKind = delim.indexOf(line.at(pos))+1) {
            // find matching quote
            end = line.indexOf(quoteKind<3 ? delim.at(quoteKind-1) : ')', pos+1);
            if (end < 0)
                return SyntaxBlock(this, start, pos+1, SyntaxShift::in, true);
            pos = end+1;
        } else {
            while (++pos < line.length()) {
                if (special.contains(line.at(pos)))
                    break;
            }
        }
        end = pos;
        // if no dot or colon follows, finish
        while (isWhitechar(line,pos)) ++pos;
        if (pos < line.length() && special.indexOf(line.at(pos)) < 3) break;
        if (pos < line.length() && special.indexOf(line.at(pos)) < 7) ++pos;
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
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

AssignmentValue::AssignmentValue()
    : SyntaxAbstract(SyntaxKind::AssignmentValue)
{
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierAssignment << SyntaxKind::IdentifierAssignmentEnd;
}

SyntaxBlock AssignmentValue::find(const SyntaxKind entryKind, const QString &line, int index)
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
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
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
        Q_ASSERT_X(false, "SyntaxTableAssign", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
}

SyntaxBlock SyntaxTableAssign::find(const SyntaxKind entryKind, const QString &line, int index)
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

} // namespace syntax
} // namespace studio
} // namespace gams
