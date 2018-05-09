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
#ifndef SYNTAXFORMATS_H
#define SYNTAXFORMATS_H

#include <QTextCharFormat>
#include <QHash>
#include <QStringList>
#include <QRegularExpression>

namespace gams {
namespace studio {

enum class SyntaxState {
    Standard,
    Directive,
    DirectiveBody,                  // text following the Directive
    DirectiveComment,               // a DirectiveBody formatted as comment
    Title,                          // a DirectiveBody formatted as title

    CommentLine,
    CommentBlock,
    CommentEndline,
    CommentInline,

    Semicolon,
    Comma,
    DeclarationSetType,             // must be followed by Declaration
    DeclarationVariableType,        // must be followed by Declaration
    Declaration,
    DeclarationTable,

    Identifier,
    IdentifierDescription1,         // description started with single quote '
    IdentifierDescription2,         // description started with double quote "
    IdentifierAssignment,
    IdentifierAssignmentEnd,        // after assignment to keep declaration-level

    IdentifierTable,
    IdentifierTableDescription1,
    IdentifierTableDescription2,
    IdentifierTableAssignmentHead,
    IdentifierTableAssignmentRow,   // after assignment to keep declaration-level

    Embedded,
    EmbeddedBody,
    EmbeddedEnd,
    Reserved,
    ReservedBody,

    StateCount
};
QString syntaxStateName(SyntaxState state);

enum class SyntaxStateShift {
    shift,      ///> shifts from the previous state to the current state
    skip,       ///> skips the current state
    in,         ///> stacks the nextState on top
    out,        ///> steps out of the state (unstacks current state)
    reset,      ///> steps out of the whole stack until SyntaxState::Standard is reached
};

struct SyntaxTransition
{
    SyntaxTransition(SyntaxState _state, SyntaxStateShift _shift) : state(_state), shift(_shift) {}
    const SyntaxState state;
    const SyntaxStateShift shift;
};

typedef QList<SyntaxState> SyntaxTransitions;

class SyntaxAbstract;

struct SyntaxBlock
{
    SyntaxBlock(SyntaxAbstract* _syntax = nullptr, int _start = 0, int _end = 0, bool _error = false
            , SyntaxStateShift _shift = SyntaxStateShift::shift, SyntaxState _next = SyntaxState::Standard)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(_shift), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, int _start, int _end, SyntaxState _next, bool _error = false)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(SyntaxStateShift::in), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, int _start, int _end, SyntaxStateShift _shift, bool _error = false)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(_shift), next(SyntaxState::Standard)
    { }
    SyntaxAbstract *syntax;
    int start;
    int end;
    bool error;
    SyntaxStateShift shift;
    SyntaxState next;
    int length() { return end-start; }
    bool isValid() { return syntax && start<end; }
};

/// \brief An abstract class to be used inside the <c>SyntaxHighlighter</c>.
class SyntaxAbstract
{
public:
    SyntaxAbstract(SyntaxState state) : mState(state) {}
    virtual ~SyntaxAbstract() {}
    SyntaxState state() { return mState; }

    /// Finds the begin of this syntax
    virtual SyntaxBlock find(SyntaxState entryState, const QString &line, int index) = 0;

    /// Finds the end of valid trailing characters for this syntax
    virtual SyntaxBlock validTail(const QString &line, int index, bool &hasContent) = 0;
    virtual SyntaxTransitions nextStates(bool emptyLine = false);
    virtual QTextCharFormat& charFormat() { return mCharFormat; }
    virtual QTextCharFormat charFormatError();
    virtual void copyCharFormat(QTextCharFormat charFormat) { mCharFormat = charFormat; }
    int intSyntaxType() { return static_cast<int>(state()); }
    static int stateToInt(SyntaxState _state);
    static SyntaxState intToState(int intState);
protected:

    inline bool isKeywordChar(const QChar& ch) {
        return (ch.isLetterOrNumber() || ch == '_' || ch == '.');
    }
    inline bool isKeywordChar(const QString& line, int index) {
        if (index >= line.length()) return false;
        const QChar& ch(line.at(index));
        return (ch.isLetterOrNumber() || ch == '_' || ch == '.');
    }
    inline bool isWhitechar(const QString& line, int index) {
        if (index >= line.length()) return false;
        const QChar& ch(line.at(index));
        return (ch.category()==QChar::Separator_Space || ch == '\t' || ch == '\n' || ch == '\r');
    }
protected:
    SyntaxState mState;
    QTextCharFormat mCharFormat;
    SyntaxTransitions mSubStates;
    SyntaxTransitions mEmptyLineStates;
};


/// \brief Defines the syntax for standard code.
class SyntaxStandard : public SyntaxAbstract
{
public:
    SyntaxStandard();
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};


/// \brief Defines the syntax for a directive.
class SyntaxDirective : public SyntaxAbstract
{
public:
    SyntaxDirective(QChar directiveChar = '$');
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
private:
    QRegularExpression mRex;
    QStringList mDirectives;
    QStringList mDescription;
    QHash<QString, SyntaxState> mSpecialStates;
};


/// \brief Defines the syntax for a single comment line.
class SyntaxDirectiveBody: public SyntaxAbstract
{
public:
    SyntaxDirectiveBody(SyntaxState state);
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};

/// \brief Defines the syntax for a single comment line.
class SyntaxCommentLine: public SyntaxAbstract
{
public:
    SyntaxCommentLine(QChar commentChar = '*');
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
private:
    QChar mCommentChar;
};

/// \brief Defines the syntax for a multi-line comment block.
class SyntaxCommentBlock: public SyntaxAbstract
{
public:
    SyntaxCommentBlock();
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};

class SyntaxDelimiter: public SyntaxAbstract
{
    QChar mDelimiter;
public:
    SyntaxDelimiter(SyntaxState state);
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};

} // namespace studio
} // namespace gams

#endif // SYNTAXFORMATS_H
