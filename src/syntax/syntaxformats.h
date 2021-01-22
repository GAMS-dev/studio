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
#ifndef SYNTAXFORMATS_H
#define SYNTAXFORMATS_H

#include "theme.h"
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
    String,
    Formula,
    Assignment,
    Call,

    CommentLine,
    CommentBlock,
    CommentEndline,
    CommentInline,
    IgnoredHead,
    IgnoredBlock,

    Semicolon,
    CommaIdent,
    DeclarationSetType,             // must be followed by Declaration
    DeclarationVariableType,        // must be followed by Declaration
    Declaration,                    // uses flavor bits > 1

    Identifier,
    IdentifierDim,                  // dimension started with '(' or '[' - uses flavor bit 1
    IdentifierDimEnd,               // dimension end with ')' or ']' - uses flavor bit 1
    IdentifierDescription,

    IdentifierAssignment,
    AssignmentLabel,
    AssignmentValue,
    IdentifierAssignmentEnd,        // after assignment to keep declaration-level

    IdentifierTableAssignmentColHead,
    IdentifierTableAssignmentRowHead,
    IdentifierTableAssignmentRow,   // after assignment to keep declaration-level

    Embedded,
    EmbeddedBody,
    EmbeddedEnd,
    Reserved,
    Solve,
    SolveBody,
    SolveKey,
    Option,
    OptionBody,
    OptionKey,
    Execute,
    ExecuteBody,
    ExecuteKey,

    KindCount
};
Q_ENUM_NS(SyntaxKind);

//inline QTextStream &operator <<(QTextStream &steam, SyntaxKind key) noexcept { return steam << QVariant::fromValue(key).toString(); }

QString syntaxKindName(SyntaxKind kind);

enum FlavorFlag {
    flavorBrace = 1,
    flavorTable = 2,
    flavorModel = 4,
};

enum class SyntaxShift {
    shift,      ///> replace current-top-kind by this
    skip,       ///> skips this kind (keep current-top-kind)
    in,         ///> stacks the nextKind on top of current-top-kind
    out,        ///> steps out of the kind (unstacks current-top-kind)
    reset,      ///> steps out of the whole stack until SyntaxKind::Standard is reached
};
Q_ENUM_NS(SyntaxShift);

struct SyntaxTransition
{
    SyntaxTransition(SyntaxKind _kind, SyntaxShift _shift) : kind(_kind), shift(_shift) {}
    const SyntaxKind kind;
    const SyntaxShift shift;
};

typedef QList<SyntaxKind> SyntaxTransitions;

class SyntaxAbstract;

struct SyntaxBlock
{
    SyntaxBlock(SyntaxAbstract* _syntax = nullptr, int kindFlavor = 0, int _start = 0, int _end = 0
            , bool _error = false , SyntaxShift _shift = SyntaxShift::shift, SyntaxKind _next = SyntaxKind::Standard)
        : syntax(_syntax), flavor(kindFlavor), start(_start), end(_end), error(_error), shift(_shift), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, int kindFlavor, int _start, int _end, SyntaxKind _next, bool _error = false)
        : syntax(_syntax), flavor(kindFlavor), start(_start), end(_end), error(_error), shift(SyntaxShift::in), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, int kindFlavor, int _start, int _end, SyntaxShift _shift, bool _error = false)
        : syntax(_syntax), flavor(kindFlavor), start(_start), end(_end), error(_error), shift(_shift),
          next(SyntaxKind::Standard)
    { }
    SyntaxAbstract *syntax;
    int flavor;
    int start;
    int end;
    bool error;
    SyntaxShift shift;
    SyntaxKind next;
    int length() { return end-start; }
    bool isValid() { return syntax && start<end; }
};

/// \brief An abstract class to be used inside the <c>SyntaxHighlighter</c>.
class SyntaxAbstract
{
public:
    SyntaxAbstract(SyntaxKind kind) : mKind(kind) {}
    virtual ~SyntaxAbstract() {}
    SyntaxKind kind() const { return mKind; }
    void assignColorSlot(Theme::ColorSlot slot);
    Theme::ColorSlot colorSlot() const { return mColorSlot; }

    /// Finds the begin of this syntax
    virtual SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) = 0;

    /// Finds the end of valid trailing characters for this syntax
    virtual SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) = 0;
    virtual SyntaxTransitions nextKinds(bool emptyLine = false);
    virtual QTextCharFormat& charFormat() { return mCharFormat; }
    virtual QTextCharFormat charFormatError();
    virtual int maxNesting() { return 0; }
    virtual void copyCharFormat(QTextCharFormat charFormat) { mCharFormat = charFormat; }
    int intSyntaxType() { return static_cast<int>(kind()); }
    static int stateToInt(SyntaxKind _state);
    static SyntaxKind intToState(int intState);
protected:
    static const QVector<QChar> cSpecialCharacters;  // other breaking kind

    inline int charClass(QChar ch, int &prev, QVector<QChar> moreSpecialChars = QVector<QChar>()) {
        // ASCII:   "   $   '   .   0  9   ;   =   A  Z   _   a   z
        // Code:   34, 36, 39, 46, 48-57, 59, 61, 65-90, 95, 97-122
        if (ch < '"' || ch > 'z')
            prev = 0;
        else if (ch >= 'a' || (ch >= 'A' && ch <= 'Z') || ch == '_')
            prev = 2;  // break by keyword kind
        else if (ch >= '0' && ch <= '9') {
            if (prev != 2) prev = 0;
        } else {
            prev = (cSpecialCharacters.contains(ch) || moreSpecialChars.contains(ch)) ? 1 : 0;
        }
        return prev;
    }

    int endOfQuotes(const QString &line, const int &start);
    int endOfParentheses(const QString &line, const int &start, const QString &validPairs, int &nest);

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
    SyntaxKind mKind;
    Theme::ColorSlot mColorSlot = Theme::ColorSlot::invalid;
    QTextCharFormat mCharFormat;
    SyntaxTransitions mSubKinds;
    SyntaxTransitions mEmptyLineKinds;
};


/// \brief Defines the syntax for standard code.
class SyntaxStandard : public SyntaxAbstract
{
public:
    SyntaxStandard();
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

class SyntaxCommentEndline;
class SyntaxFormula;
class SyntaxDirectiveBody;
/// \brief Defines the syntax for a directive.
class SyntaxDirective : public SyntaxAbstract
{
public:
    SyntaxDirective(QChar directiveChar = '$');
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
    void setSyntaxCommentEndline(SyntaxCommentEndline *syntax) {mSyntaxCommentEndline = syntax;}
    void addSubBody(SyntaxFormula *syntax) {mSubSyntaxBody << syntax;}
    void setDirectiveBody(SyntaxDirectiveBody *syntax) {mSubDirectiveBody = syntax;}
private:
    QRegularExpression mRex;
    QStringList mDirectives;
    QStringList mDescription;
    QMap<QString,int> mFlavors;
    QHash<QString, SyntaxKind> mSpecialKinds;
    SyntaxCommentEndline *mSyntaxCommentEndline = nullptr;
    QVector<SyntaxFormula*> mSubSyntaxBody;
    SyntaxDirectiveBody *mSubDirectiveBody = nullptr;

};


/// \brief Defines the syntax for a single comment line.
class SyntaxDirectiveBody: public SyntaxAbstract
{
    QVector<QChar> mCommentChars;
public:
    SyntaxDirectiveBody(SyntaxKind kind);
    void setCommentChars(QVector<QChar> chars);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

/// \brief Defines the syntax for a single comment line.
class SyntaxCommentLine: public SyntaxAbstract
{
public:
    SyntaxCommentLine(QChar commentChar = '*');
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
private:
    QChar mCommentChar;
};

 /// \brief Defines the syntax for a single comment line.
class SyntaxCommentEndline: public SyntaxAbstract
{
    QString mCommentChars;
public:
    SyntaxCommentEndline(QString commentChars = "!!");
    void setCommentChars(QString commentChars);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

/// \brief Defines the syntax for a uniform multi-line block.
class SyntaxUniformBlock: public SyntaxAbstract
{
public:
    SyntaxUniformBlock(SyntaxKind kind);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

/// \brief Defines the syntax for a multi-line comment block.
class SyntaxCall: public SyntaxAbstract
{
    QStringList mSubDirective;
public:
    SyntaxCall();
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

class SyntaxDelimiter: public SyntaxAbstract
{
    QChar mDelimiter;
public:
    SyntaxDelimiter(SyntaxKind kind);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

class SyntaxFormula: public SyntaxAbstract
{
    QVector<QChar> mSpecialDynamicChars;
public:
    SyntaxFormula(SyntaxKind kind);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
    void setSpecialDynamicChars(QVector<QChar> chars);
};

class SyntaxString : public SyntaxAbstract
{
public:
    SyntaxString();
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

class SyntaxAssign : public SyntaxAbstract
{
public:
    SyntaxAssign();
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};



} // namespace syntax
} // namespace studio
} // namespace gams

#endif // SYNTAXFORMATS_H
