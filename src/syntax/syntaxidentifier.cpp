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
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline
              << SyntaxKind::IdentifierDim1 << SyntaxKind::IdentifierDim2
              << SyntaxKind::IdentifierAssignment << SyntaxKind::IdentifierTableAssignmentHead
              << SyntaxKind::CommaIdent;

    mEmptyLineKinds << mSubKinds
                    << SyntaxKind::Identifier
                    << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                    << SyntaxKind::Declaration;

    mSubKinds << SyntaxKind::IdentifierDescription;  // must not exist in emptyLineKinds
}

int SyntaxIdentifier::identChar(const QChar &c) const
{
    if (c >= 'A' && c <= 'z' && (c >= 'a' || c <= 'Z')) return 2;   // valid start identifier letter
    if ((c >= '0' && c <= '9') || c == '_') return 1;               // valid next identifier letter
    return 0;                                                       // no valid identifier letter
}

SyntaxBlock SyntaxIdentifier::find(const SyntaxKind entryKind, const int kindFlavor, const QString& line, int index)
{
    // TODO(JM) ensure only one identifier on flavor=1 (table)
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    int end = start;
    if (identChar(line.at(end++)) != 2) return SyntaxBlock(this);
    while (end < line.length()) {
        if (!identChar(line.at(end++))) return SyntaxBlock(this, kindFlavor, start, end-1, SyntaxShift::shift);
    }
    return SyntaxBlock(this, kindFlavor, start, end, SyntaxShift::shift);
}

SyntaxBlock SyntaxIdentifier::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    hasContent = false;
    return SyntaxBlock(this, flavor, index, start, SyntaxShift::shift);
}

SyntaxIdentifierDim::SyntaxIdentifierDim(SyntaxKind kind) : SyntaxAbstract(kind)
{
    QHash<int, QChar> delims {
        {static_cast<int>(SyntaxKind::IdentifierDim1), '('},
        {static_cast<int>(SyntaxKind::IdentifierDim2), '['},
    };
    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierDimEnd1
              << SyntaxKind::IdentifierDimEnd2;
    mEmptyLineKinds << mSubKinds;
    if (!delims.contains(static_cast<int>(kind)))
        Q_ASSERT_X(false, "SyntaxIdentifierDim", QString("missing delimiter for SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    mDelimiterIn = delims.value(static_cast<int>(kind));
    mDelimiterOut = (mDelimiterIn == '(') ? ')' : ']';
}

SyntaxBlock SyntaxIdentifierDim::find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    if (line.at(start) != mDelimiterIn) return SyntaxBlock(this);
    return SyntaxBlock(this, kindFlavor, start, start+1, SyntaxShift::shift);
}

SyntaxBlock SyntaxIdentifierDim::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    const QString invalid("([])");
    int end = index-1;
    // inside valid identifier dimension
    while (++end < line.length()) {
        if (line.at(end--) == mDelimiterOut) break;
        if (invalid.indexOf(line.at(++end)) >= 0)
            return SyntaxBlock(this, flavor, index, end, SyntaxShift::out, true);
    }
    hasContent = index < end;
    return SyntaxBlock(this, flavor, index, end+1, SyntaxShift::shift);
}

SyntaxIdentifierDimEnd::SyntaxIdentifierDimEnd(SyntaxKind kind) : SyntaxAbstract(kind)
{
    QHash<int, QChar> delims {
        {static_cast<int>(SyntaxKind::IdentifierDimEnd1), ')'},
        {static_cast<int>(SyntaxKind::IdentifierDimEnd2), ']'},
    };
    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::CommaIdent << SyntaxKind::Semicolon
              << SyntaxKind::IdentifierAssignment << SyntaxKind::IdentifierTableAssignmentHead;
    mEmptyLineKinds << mSubKinds
                    << SyntaxKind::Identifier
                    << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                    << SyntaxKind::Declaration;
    mSubKinds << SyntaxKind::IdentifierDescription; // must not exist in emptyLineKinds

    if (!delims.contains(static_cast<int>(kind)))
        Q_ASSERT_X(false, "SyntaxIdentifierDimEnd", QString("missing delimiter for SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    mDelimiter = delims.value(static_cast<int>(kind));
}

SyntaxBlock SyntaxIdentifierDimEnd::find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length() || line.at(start) != mDelimiter)  return SyntaxBlock(this);
    return SyntaxBlock(this, kindFlavor, index, qMin(start+1, line.length()), SyntaxShift::shift);
}

SyntaxBlock SyntaxIdentifierDimEnd::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
//    if (index == 0) return SyntaxBlock(this);
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
}

SyntaxIdentDescript::SyntaxIdentDescript(SyntaxKind kind) : SyntaxAbstract(kind)
{
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mEmptyLineKinds = mSubKinds;
    mEmptyLineKinds << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                     << SyntaxKind::Declaration << SyntaxKind::IdentifierAssignment
                     << SyntaxKind::IdentifierTableAssignmentHead << SyntaxKind::Identifier;
    mSubKinds << SyntaxKind::CommaIdent << SyntaxKind::Semicolon << SyntaxKind::IdentifierAssignment;
}

SyntaxBlock SyntaxIdentDescript::find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index)
{
    if (index == 0) return SyntaxBlock(this);
    if (entryKind != SyntaxKind::Identifier
            && entryKind != SyntaxKind::IdentifierDimEnd1
            && entryKind != SyntaxKind::IdentifierDimEnd2)
        return SyntaxBlock(this);
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
        if (line.at(end) == delim) return SyntaxBlock(this, kindFlavor, start, end+(delim=='/'?0:1), SyntaxShift::shift);
        if (delim == '/' && line.at(end) == ';') break;
        if (delim == '/' && line.at(end) == ',') break;
        if (!isWhitechar(line, end)) lastNonWhite = end;
    }
    return SyntaxBlock(this, kindFlavor, start, lastNonWhite+1, SyntaxShift::shift, (delim != '/'));
}

SyntaxBlock SyntaxIdentDescript::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
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

SyntaxBlock SyntaxIdentAssign::find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index)
{
    if (kindFlavor == 1) return SyntaxBlock(this);
    int start = index;
    bool inside = (kind() != SyntaxKind::IdentifierAssignmentEnd
                   && (entryKind == SyntaxKind::AssignmentLabel
                       || entryKind == SyntaxKind::AssignmentValue));
    QChar delim = inside ? ',' : mDelimiter;
    while (isWhitechar(line, start))
        ++start;
    if (start < line.length() && line.at(start) == delim)
        return SyntaxBlock(this, kindFlavor, start, start+1, SyntaxShift::shift);
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentAssign::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    int end = (line.length() > start) ? start+1 : start;
    while (isWhitechar(line, end) || (line.length() > end && line.at(end) == ',')) end++;
    hasContent = (end > start);
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
}

AssignmentLabel::AssignmentLabel()
     : SyntaxAbstract(SyntaxKind::AssignmentLabel)
{
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierAssignmentEnd << SyntaxKind::IdentifierAssignment
              << SyntaxKind::AssignmentLabel << SyntaxKind::AssignmentValue ;
}

SyntaxBlock AssignmentLabel::find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    if (kindFlavor == 1) return SyntaxBlock(this);
    int start = index;
    while (isWhitechar(line, start)) start++;
    if (start >= line.size()) return SyntaxBlock(this);
    if (entryKind == SyntaxKind::AssignmentLabel && index != 0) {
        return SyntaxBlock(this);
    }
    int nesting = 0; // (JM) Best, if we can get nesting from prev line

    QString quote("\"\'");
    QString extender(".:*");
    QString ender("/,");
    QString pPairs("()");
    int end = start;
    bool extended = false;
    for (int pos = start; pos < line.length(); ++pos) {
        if (extender.contains(line.at(pos))) {
            while (isWhitechar(line, pos+1)) ++pos;
            extended = true;
        } else {
            if (quote.contains(line.at(pos))) {
                pos = endOfQuotes(line, pos);
                extended = false;
                if (pos < start) return SyntaxBlock(this);
            } else if (line.at(pos) == '(') {
                ++nesting;
                pos = endOfParentheses(line, pos, pPairs, nesting);
                if (pos <= start) pos = line.length()-1; // wrong_nesting
            } else if (isWhitechar(line, pos)) {
                while (isWhitechar(line, pos+1)) ++pos;
                if (!extended && (pos+1 < line.length()) && !extender.contains(line.at(pos+1)))
                    return SyntaxBlock(this, 0, start, pos, SyntaxShift::shift);
            } else if (ender.contains(line.at(pos))) {
                return SyntaxBlock(this, 0, start, pos, SyntaxShift::shift);
            }
            extended = false;
        }
        end = pos+1;
    }

    if (end > start) {
        return SyntaxBlock(this, 0, start, end, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock AssignmentLabel::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
}

AssignmentValue::AssignmentValue()
    : SyntaxAbstract(SyntaxKind::AssignmentValue)
{
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierAssignment << SyntaxKind::IdentifierAssignmentEnd;
}

SyntaxBlock AssignmentValue::find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    if (kindFlavor == 1) return SyntaxBlock(this);
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
            return SyntaxBlock(this, 0, start, pos+1, SyntaxShift::out, true);
        pos = end+1;
    } else {
        while (++pos < line.length() && !special.contains(line.at(pos))) end = pos;
    }
    end = pos;
//    while (isWhitechar(line, pos)) ++pos;

    if (end > start) {
        return SyntaxBlock(this, 0, start, end, SyntaxShift::skip);
    }
    return SyntaxBlock(this);
}

SyntaxBlock AssignmentValue::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
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
//        mSubKinds << SyntaxKind::IdentifierTableAssignmentHead;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxTableAssign", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
}

SyntaxBlock SyntaxTableAssign::find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index)
{
    if (kindFlavor != 1) return SyntaxBlock(this);
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
        return SyntaxBlock(this, 0, index, line.length(), SyntaxShift::shift);
    return SyntaxBlock(this, 0, index, end, SyntaxShift::out);
}

SyntaxBlock SyntaxTableAssign::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    Q_UNUSED(hasContent)
    int end = line.indexOf(';', index);
    if (end < 0)
        return SyntaxBlock(this, flavor, index, line.length(), SyntaxShift::shift);
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::out);
}

} // namespace syntax
} // namespace studio
} // namespace gams
