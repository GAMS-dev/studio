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
#include <QMetaEnum>
#include <QTextStream>

namespace gams {
namespace studio {

namespace syntax {

Q_NAMESPACE

enum class SyntaxKind {
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
    IdentifierDim1,                 // dimension started with '('
    IdentifierDim2,                 // dimension started with '['
    IdentifierDimEnd1,              // dimension started with '(' must end with ')'
    IdentifierDimEnd2,              // dimension started with '[' must end with ']'
    IdentifierDescription,
    IdentifierAssignment,
    AssignmentLabel,
    AssignmentValue,
    IdentifierAssignmentEnd,        // after assignment to keep declaration-level

    IdentifierTable,
    IdentifierTableDim1,
    IdentifierTableDim2,
    IdentifierTableDimEnd1,
    IdentifierTableDimEnd2,
    IdentifierTableDescription,
    IdentifierTableAssignmentHead,
    IdentifierTableAssignmentRow,   // after assignment to keep declaration-level

    Embedded,
    EmbeddedBody,
    EmbeddedEnd,
    Reserved,
    ReservedBody,

    KindCount
};
Q_ENUM_NS(SyntaxKind);

//inline QTextStream &operator <<(QTextStream &steam, syntax::SyntaxKind key) noexcept { return steam << QVariant::fromValue(key).toString(); }

QString syntaxKindName(syntax::SyntaxKind kind);

enum class SyntaxShift {
    shift,      ///> shifts from the previous kind to the current kind
    skip,       ///> skips the current kind
    in,         ///> stacks the nextKind on top
    out,        ///> steps out of the kind (unstacks current kind)
    reset,      ///> steps out of the whole stack until SyntaxKind::Standard is reached
};
Q_ENUM_NS(SyntaxShift);

struct SyntaxTransition
{
    SyntaxTransition(syntax::SyntaxKind _kind, SyntaxShift _shift) : kind(_kind), shift(_shift) {}
    const syntax::SyntaxKind kind;
    const SyntaxShift shift;
};

typedef QList<syntax::SyntaxKind> SyntaxTransitions;

class SyntaxAbstract;

struct SyntaxBlock
{
    SyntaxBlock(SyntaxAbstract* _syntax = nullptr, int _start = 0, int _end = 0, bool _error = false
            , SyntaxShift _shift = SyntaxShift::shift, syntax::SyntaxKind _next = SyntaxKind::Standard)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(_shift), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, int _start, int _end, syntax::SyntaxKind _next, bool _error = false)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(SyntaxShift::in), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, int _start, int _end, SyntaxShift _shift, bool _error = false)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(_shift), next(SyntaxKind::Standard)
    { }
    SyntaxAbstract *syntax;
    int start;
    int end;
    bool error;
    SyntaxShift shift;
    syntax::SyntaxKind next;
    int length() { return end-start; }
    bool isValid() { return syntax && start<end; }
};

/// \brief An abstract class to be used inside the <c>SyntaxHighlighter</c>.
class SyntaxAbstract
{
public:
    SyntaxAbstract(syntax::SyntaxKind kind) : mKind(kind) {}
    virtual ~SyntaxAbstract() {}
    syntax::SyntaxKind kind() { return mKind; }

    /// Finds the begin of this syntax
    virtual SyntaxBlock find(syntax::SyntaxKind entryKind, const QString &line, int index) = 0;

    /// Finds the end of valid trailing characters for this syntax
    virtual SyntaxBlock validTail(const QString &line, int index, bool &hasContent) = 0;
    virtual SyntaxTransitions nextKinds(bool emptyLine = false);
    virtual QTextCharFormat& charFormat() { return mCharFormat; }
    virtual QTextCharFormat charFormatError();
    virtual int maxNesting() { return 0; }
    virtual void copyCharFormat(QTextCharFormat charFormat) { mCharFormat = charFormat; }
    int intSyntaxType() { return static_cast<int>(kind()); }
    static int stateToInt(syntax::SyntaxKind _state);
    static syntax::SyntaxKind intToState(int intState);
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
    syntax::SyntaxKind mKind;
    QTextCharFormat mCharFormat;
    SyntaxTransitions mSubKinds;
    SyntaxTransitions mEmptyLineKinds;
};


/// \brief Defines the syntax for standard code.
class SyntaxStandard : public SyntaxAbstract
{
public:
    SyntaxStandard();
    SyntaxBlock find(syntax::SyntaxKind entryKind, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};


/// \brief Defines the syntax for a directive.
class SyntaxDirective : public SyntaxAbstract
{
public:
    SyntaxDirective(QChar directiveChar = '$');
    SyntaxBlock find(syntax::SyntaxKind entryKind, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
private:
    QRegularExpression mRex;
    QStringList mDirectives;
    QStringList mDescription;
    QHash<QString, syntax::SyntaxKind> mSpecialKinds;
};


/// \brief Defines the syntax for a single comment line.
class SyntaxDirectiveBody: public SyntaxAbstract
{
public:
    SyntaxDirectiveBody(syntax::SyntaxKind kind);
    SyntaxBlock find(syntax::SyntaxKind entryKind, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};

/// \brief Defines the syntax for a single comment line.
class SyntaxCommentLine: public SyntaxAbstract
{
public:
    SyntaxCommentLine(QChar commentChar = '*');
    SyntaxBlock find(syntax::SyntaxKind entryKind, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
private:
    QChar mCommentChar;
};

/// \brief Defines the syntax for a multi-line comment block.
class SyntaxCommentBlock: public SyntaxAbstract
{
public:
    SyntaxCommentBlock();
    SyntaxBlock find(syntax::SyntaxKind entryKind, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};

class SyntaxDelimiter: public SyntaxAbstract
{
    QChar mDelimiter;
public:
    SyntaxDelimiter(syntax::SyntaxKind kind);
    SyntaxBlock find(syntax::SyntaxKind entryKind, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};


} // namespace syntax
} // namespace studio
} // namespace gams

#endif // SYNTAXFORMATS_H
