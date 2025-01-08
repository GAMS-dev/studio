/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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

SyntaxIdentifier::SyntaxIdentifier(SharedSyntaxData *sharedData) : SyntaxAbstract(SyntaxKind::Identifier, sharedData)
{
    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Dco << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline
              << SyntaxKind::IdentifierDim << SyntaxKind::IdentifierAssignment
              << SyntaxKind::IdentifierTableAssignmentColHead << SyntaxKind::CommaIdent;

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

SyntaxBlock SyntaxIdentifier::find(const SyntaxKind entryKind, SyntaxState state, const QString& line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    int end = start;
    if (identChar(line.at(end++)) != 2) return SyntaxBlock(this);
    while (end < line.length()) {
        if (!identChar(line.at(end++))) return SyntaxBlock(this, state, start, end-1, SyntaxShift::shift);
    }
    return SyntaxBlock(this, state, start, end, SyntaxShift::shift);
}

SyntaxBlock SyntaxIdentifier::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    hasContent = false;
    return SyntaxBlock(this, state, index, start, SyntaxShift::shift);
}

SyntaxIdentifierDim::SyntaxIdentifierDim(SharedSyntaxData *sharedData)
    : SyntaxAbstract(SyntaxKind::IdentifierDim, sharedData), mDelimiters("([)]")
{
    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Dco << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierDimEnd;
    mEmptyLineKinds << mSubKinds;
}

SyntaxBlock SyntaxIdentifierDim::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    if (line.at(start) == mDelimiters.at(0))
        return SyntaxBlock(this, state, start, start+1, SyntaxShift::shift);
    if (line.at(start) == mDelimiters.at(1)) {
        state.flavor += flavorBrace;
        return SyntaxBlock(this, state, start, start+1, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentifierDim::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    int end = index-1;
    // inside valid identifier dimension
    while (++end < line.length()) {
        if (line.at(end--) == mDelimiters.at((state.flavor & flavorQuotePart) == flavorBrace ? 3 : 2)) break;
        if (mDelimiters.indexOf(line.at(++end)) >= 0)
            return SyntaxBlock(this, state, index, end, SyntaxShift::out, true);
    }
    hasContent = index < end;
    return SyntaxBlock(this, state, index, end+1, SyntaxShift::shift);
}

SyntaxIdentifierDimEnd::SyntaxIdentifierDimEnd(SharedSyntaxData *sharedData)
    : SyntaxAbstract(SyntaxKind::IdentifierDimEnd, sharedData), mDelimiters(")]")
{
    // sub-kinds to check for all types
    mSubKinds << SyntaxKind::Dco << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::CommaIdent << SyntaxKind::Semicolon
              << SyntaxKind::IdentifierAssignment << SyntaxKind::IdentifierTableAssignmentColHead;
    mEmptyLineKinds << mSubKinds
                    << SyntaxKind::Identifier
                    << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                    << SyntaxKind::Declaration;
    mSubKinds << SyntaxKind::IdentifierDescription; // must not exist in emptyLineKinds
}

SyntaxBlock SyntaxIdentifierDimEnd::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length() || line.at(start) != mDelimiters.at((state.flavor & flavorQuotePart) == flavorBrace ? 1 : 0))
        return SyntaxBlock(this);
    if ((state.flavor & flavorQuotePart) == flavorBrace)
        state.flavor = state.flavor - flavorBrace;
    return SyntaxBlock(this, state, index, qMin(start+1, line.length()), SyntaxShift::shift);
}

SyntaxBlock SyntaxIdentifierDimEnd::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
}

SyntaxIdentDescript::SyntaxIdentDescript(SharedSyntaxData *sharedData)
    : SyntaxAbstract(SyntaxKind::IdentifierDescription, sharedData)
{
    mSubKinds << SyntaxKind::Dco << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mEmptyLineKinds = mSubKinds;
    mEmptyLineKinds << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
                     << SyntaxKind::Declaration << SyntaxKind::IdentifierAssignment
                     << SyntaxKind::IdentifierTableAssignmentColHead << SyntaxKind::Identifier;
    mSubKinds << SyntaxKind::CommaIdent << SyntaxKind::Semicolon << SyntaxKind::IdentifierAssignment;
}

SyntaxBlock SyntaxIdentDescript::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    if (index == 0) return SyntaxBlock(this);
    if (entryKind != SyntaxKind::Identifier
            && entryKind != SyntaxKind::IdentifierDimEnd)
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
        if (line.at(end) == delim) return SyntaxBlock(this, state, start, end+(delim=='/'?0:1), SyntaxShift::shift);
        if (delim == '/' && line.at(end) == ';') break;
        if (delim == '/' && line.at(end) == ',') break;
        if (!isWhitechar(line, end)) lastNonWhite = end;
    }
    return SyntaxBlock(this, state, start, lastNonWhite+1, SyntaxShift::shift, (delim != '/'));
}

SyntaxBlock SyntaxIdentDescript::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
}

SyntaxIdentAssign::SyntaxIdentAssign(SyntaxKind kind, SharedSyntaxData *sharedData) : SyntaxAbstract(kind, sharedData)
{
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Dco << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;

    switch (kind) {
    case SyntaxKind::IdentifierAssignment:
        mSubKinds << SyntaxKind::IdentifierAssignmentEnd
                  << SyntaxKind::AssignmentSystemData << SyntaxKind::AssignmentLabel;
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

SyntaxBlock SyntaxIdentAssign::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    if (state.flavor & flavorTable) return SyntaxBlock(this);
    int end = index;
    bool inside = (kind() != SyntaxKind::IdentifierAssignmentEnd
                   && (entryKind == SyntaxKind::AssignmentLabel
                       || entryKind == SyntaxKind::AssignmentValue
                       || entryKind == SyntaxKind::AssignmentSystemData));
    QString delims = inside ? (state.flavor & flavorModel ? ",+-" : ",") : "/";
    while (isWhitechar(line, end))
        ++end;
    if (end < line.length() && delims.contains(line.at(end))) {
        ++end;
        return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxIdentAssign::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    int start = index;
    QString delims = (state.flavor & flavorModel) ? ",+-" : ",";
    while (isWhitechar(line, start))
        ++start;
    int end = (line.length() > start) ? start+1 : start;
    while (isWhitechar(line, end) || (line.length() > end && delims.contains(line.at(end)))) end++;
    hasContent = (end > start);
    return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
}

AssignmentLabel::AssignmentLabel(SharedSyntaxData *sharedData)
     : SyntaxAbstract(SyntaxKind::AssignmentLabel, sharedData)
{
    mSubKinds << SyntaxKind::Dco << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierAssignmentEnd << SyntaxKind::IdentifierAssignment
              << SyntaxKind::AssignmentSystemData << SyntaxKind::AssignmentLabel << SyntaxKind::AssignmentValue;
}

SyntaxBlock AssignmentLabel::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    if (state.flavor & flavorTable) return SyntaxBlock(this);
    int start = index;
    while (isWhitechar(line, start)) start++;
    if (start >= line.size()) return SyntaxBlock(this);
    if ((entryKind == SyntaxKind::AssignmentLabel || entryKind == SyntaxKind::AssignmentSystemData)
            && index != 0 && !(state.flavor & flavorBindLabel)) {
        return SyntaxBlock(this);
    }
    int nesting = 0; // (JM) Best, if we can get nesting from prev line

    QString quote("'\"");
    QString extender(".:*");
    QString ender(state.flavor & flavorModel ? "/,+-" : "/,");
    QString pPairs("()[]");
    int end = start;
    bool extended = false;
    if (state.flavor & flavorBindLabel) {
        extended = true;
        state.flavor -= flavorBindLabel;
    }
//    int quoteType = (flavor & flavorQuotePart) - 1;
//    if (quoteType > 1 || entryKind != SyntaxKind::AssignmentLabel)
//        quoteType = -1;

    for (int pos = start; pos < line.length(); ++pos) {
        if (extender.contains(line.at(pos))) {
            while (isWhitechar(line, pos+1)) ++pos;
            if (line.at(pos) == '*')
                extended = true;
            else {
                state.flavor |= flavorBindLabel;
                return SyntaxBlock(this, state, index, pos+1, SyntaxShift::shift);
            }
        } else {
            if (quote.contains(line.at(pos))) {
                pos = endOfQuotes(line, pos);
                if (pos < start) return SyntaxBlock(this);
            } else if (pPairs.indexOf(line.at(pos)) % 2 == 0) {
                ++nesting;
                pos = endOfParentheses(line, pos, pPairs, nesting);
                if (pos <= start) pos = line.length()-1; // wrong_nesting
            } else if (isWhitechar(line, pos)) {
                while (isWhitechar(line, pos+1)) ++pos;
                if (!extended && (pos+1 < line.length()) && !extender.contains(line.at(pos+1)))
                    return SyntaxBlock(this, state, index, pos, SyntaxShift::shift);
            } else if (ender.contains(line.at(pos))) {
                return SyntaxBlock(this, state, index, pos, SyntaxShift::shift);
            }
            extended = false;
        }
        end = pos+1;
    }

    if (end > index)
        return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
    return SyntaxBlock(this);
}

SyntaxBlock AssignmentLabel::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
}

AssignmentValue::AssignmentValue(SharedSyntaxData *sharedData)
    : SyntaxAbstract(SyntaxKind::AssignmentValue, sharedData)
{
    mSubKinds << SyntaxKind::Dco << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierAssignment << SyntaxKind::IdentifierAssignmentEnd;
}

SyntaxBlock AssignmentValue::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    if (state.flavor & flavorTable) return SyntaxBlock(this);
    int start = index+1;
    while (isWhitechar(line, start)) start++;
    if (start >= line.size()) return SyntaxBlock(this);

    // get delimiters
    QString delim("\"\'[");
    QString special("/,");
    int pos = start;
    int end;
    // we are at the first non-white-char
    if (int quoteKind = delim.indexOf(line.at(pos))+1) {
        // find matching quote
        QChar ch = quoteKind==3 ? ']' : delim.at(quoteKind-1);
        end = line.indexOf(ch, pos+1);
        if (end < 0)
            return SyntaxBlock(this, state, start, pos+1, SyntaxShift::out, true);
        pos = end+1;
    } else {
        while (++pos < line.length() && !special.contains(line.at(pos))) ;
    }
    end = pos;
//    while (isWhitechar(line, pos)) ++pos;

    if (end > start) {
        return SyntaxBlock(this, state, start, end, SyntaxShift::skip);
    }
    return SyntaxBlock(this);
}

SyntaxBlock AssignmentValue::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    hasContent = false;
    return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
}

SyntaxTableAssign::SyntaxTableAssign(SyntaxKind kind, SharedSyntaxData *sharedData) : SyntaxAbstract(kind, sharedData)
{
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Dco << SyntaxKind::CommentLine
               << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    switch (kind) {
    case SyntaxKind::IdentifierTableAssignmentColHead:
        mSubKinds << SyntaxKind::IdentifierTableAssignmentRowHead;
        break;
    case SyntaxKind::IdentifierTableAssignmentRowHead:
        mSubKinds << SyntaxKind::IdentifierTableAssignmentColHead
                  << SyntaxKind::IdentifierTableAssignmentRowHead
                  << SyntaxKind::IdentifierTableAssignmentRow;
        break;
    case SyntaxKind::IdentifierTableAssignmentRow:
        mSubKinds << SyntaxKind::IdentifierTableAssignmentColHead
                  << SyntaxKind::IdentifierTableAssignmentRowHead
                  << SyntaxKind::IdentifierTableAssignmentRow;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxTableAssign", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
}

SyntaxBlock SyntaxTableAssign::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    bool inTable = entryKind == SyntaxKind::IdentifierTableAssignmentColHead
            || entryKind == SyntaxKind::IdentifierTableAssignmentRowHead
            || entryKind == SyntaxKind::IdentifierTableAssignmentRow;
    if (!inTable && !(state.flavor & flavorTable)) return SyntaxBlock(this);
    if (index > 0 && kind() != SyntaxKind::IdentifierTableAssignmentRow) return SyntaxBlock(this);

    if (kind() == SyntaxKind::IdentifierTableAssignmentColHead) {
        int end = index;
        state.flavor = 0;
        if (inTable) {
            // validate this is a continued table using '+' and find start of first column
            int plusCount = 0;
            while (end < line.length() && (isWhitechar(line, end) || line.at(end) == '+')) {
                if (line.at(end) == '\t') state.flavor = ((state.flavor/8) +1) *8;
                else ++state.flavor;
                if (line.at(end) == '+') ++plusCount;
                ++end;
            }
            if (end >= line.length() || plusCount != 1)
                return SyntaxBlock(this);
        } else {
            // find start of first column
            while (end < line.length() && isWhitechar(line, end)) {
                if (line.at(end) == '\t') state.flavor = ((state.flavor/8) +1) *8;
                else ++state.flavor;
                ++end;
            }
        }
        return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
    } else {
        // find split point between row-header and value
        int split = state.flavor;
        if (line.length() >= state.flavor)
            while (split >= 0 && !isWhitechar(line, split)) --split;
        else
            split = line.length();
        if (split <= 0) return SyntaxBlock(this);
        int end = index;
        while (end < line.length() && line.at(end) != ';' && !mSharedData->commentEndLine()->check(line, end))
            ++end;
        if (split > end) split = end;

        if (kind() == SyntaxKind::IdentifierTableAssignmentRowHead) {
            return SyntaxBlock(this, state, index, split, SyntaxShift::shift);
        }
        if (index < split) return SyntaxBlock(this);
        return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
    }

}

SyntaxBlock SyntaxTableAssign::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    Q_UNUSED(hasContent)
    int end = index;
    while (end < line.length() && line.at(end) != ';' && !mSharedData->commentEndLine()->check(line, end))
        ++end;
    if (end < 0) end = line.length();
    return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
}

SyntaxSimpleWord::SyntaxSimpleWord(SharedSyntaxData *sharedData): SyntaxAbstract(SyntaxKind::UserCompileAttrib, sharedData)
{
}

SyntaxBlock SyntaxSimpleWord::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    if (line.length() <= index+1 || line.at(index) != '%')
        return SyntaxBlock();
    int end = index+1;
    if ((line.at(end) < 'A' || line.at(end) > 'Z') && (line.at(end) < 'a' || line.at(end) > 'z'))
        return SyntaxBlock();
    ++end;
    while (line.length() > end) {
        if (line.at(end) == '%')
            return SyntaxBlock(this, state, index, end+1, SyntaxShift::shift);
        if ((line.at(end) < 'A' || line.at(end) > 'Z') && (line.at(end) < 'a' || line.at(end) > 'z')
                && (line.at(end) < '0' || line.at(end) > '9') && line.at(end) != '_')
            return SyntaxBlock();
        ++end;
    }
    return SyntaxBlock();
}

SyntaxBlock SyntaxSimpleWord::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(state)
    Q_UNUSED(hasContent)
    return SyntaxBlock();
}

} // namespace syntax
} // namespace studio
} // namespace gams
