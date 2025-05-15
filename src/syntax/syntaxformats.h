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
#ifndef SYNTAXFORMATS_H
#define SYNTAXFORMATS_H

#include "theme.h"
#include "syntaxcommon.h"

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
    Dco,
    DcoBody,                            // text following the DCO
    DcoComment,                         // a DCO body formatted as comment
    Title,                              // a DCO body formatted as title
    String,
    Formula,
    Assignment,
    SubDCO,
    UserCompileAttrib,
    SystemCompileAttrib,
    SystemCompileAttribR,
    SystemRunAttrib,

    CommentLine,
    CommentBlock,
    CommentEndline,
    CommentInline,
    IgnoredHead,
    IgnoredBlock,

    Semicolon,
    CommaIdent,
    DeclarationSetType,                 // must be followed by Declaration
    DeclarationVariableType,            // must be followed by Declaration
    Declaration,                        // uses flavor bits > 1

    Identifier,
    IdentifierDim,                      // dimension started with '(' or '[' - uses flavor bit 1
    IdentifierDimEnd,                   // dimension end with ')' or ']' - uses flavor bit 1
    IdentifierDescription,

    IdentifierAssignment,
    AssignmentLabel,
    AssignmentValue,
    AssignmentSystemData,
    IdentifierAssignmentEnd,            // after assignment to keep declaration-level

    IdentifierTableAssignmentColHead,
    IdentifierTableAssignmentRowHead,
    IdentifierTableAssignmentRow,       // after assignment to keep declaration-level

    Embedded,
    EmbeddedBody,
    EmbeddedEnd,
    Reserved,
    Abort,
    AbortKey,
    Solve,
    SolveBody,
    SolveKey,
    Option,
    OptionBody,
    OptionKey,
    Execute,
    ExecuteBody,
    ExecuteKey,
    ExecuteTool,
    ExecuteToolKey,
    Put,
    PutFormula,                         // Formula that allows SystemRunAttrib

    KindCount
};
Q_ENUM_NS(SyntaxKind);

//inline QTextStream &operator <<(QTextStream &steam, SyntaxKind key) noexcept { return steam << QVariant::fromValue(key).toString(); }

QString syntaxKindName(SyntaxKind kind);
QString syntaxKindName(int kind);

// TODO(JM) this needs to be more compact, drag disjunct parts to multi-bit regions together:
//          - check if Table, Model, and preTable can be joined (like done with flavorQuotePart)
enum FlavorFlag {
    flavorQuote1 = 1,       // in AssignmentLabel and AssignmentValue
    flavorQuote2 = 2,       // in AssignmentLabel and AssignmentValue
    flavorBrace = 3,        // only in SyntaxIdentifierDim
    flavorQuotePart = 3,

    flavorTable = 4,
    flavorModel = 8,
    flavorPreTable = 16,
    flavorBindLabel = 32,

    flavorExecDot = 4,
    flavorAbortCmd = 64,
};

enum DcoFlavor {
    flavorText1 = 1,
    flavorText0 = 2,
    flavorEcho1 = 3,
    flavorEcho0 = 4,
    flavorPut1 = 5,
    flavorPut0 = 6,
    flavorExIn1 = 7,
    flavorExIn0 = 8,
    flavorExOut1 = 9,
    flavorExOut0 = 10,
    flavorIf1 = 11,
    flavorIf0 = 12,
    flavorFold1 = 13,
    flavorFold0 = 14,

    flavorInc = 15,
    flavorAbort = 16,
    flavorCall = 17,
    flavorSave = 18,
    flavorEval = 19,

    flavorEmbed1 = 21,
    flavorEmbed0 = 22,
};
const QString cFlavorChars("TtCcPpIiOoFfUu");

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

typedef QHash<SyntaxFlag, QString> SyntaxFlagData;
typedef QSharedPointer<SyntaxFlagData> SyntaxFlags;

/// SyntaxState: Used to pass additional state values from highlighter to each syntax and back
class SyntaxState
{
    SyntaxFlags mSyntaxFlags;
    void clone() {
        SyntaxFlags copy = mSyntaxFlags;
        mSyntaxFlags = SyntaxFlags(copy.isNull() ? new SyntaxFlagData() : new SyntaxFlagData(*copy.get()));
    }
public:
    int flavor = 0;
    SyntaxFlags syntaxFlags() { return mSyntaxFlags; }
    void setSyntaxFlags(const SyntaxFlags &flags) { mSyntaxFlags = flags; }
    QString syntaxFlagValue(SyntaxFlag flag) {
        if (mSyntaxFlags.isNull()) return QString();
        return mSyntaxFlags->value(flag);
    }
    void setSyntaxFlag(SyntaxFlag flag, const QString &value) {
        clone();
        mSyntaxFlags->insert(flag, value);
    }
    void removeSyntaxFlag(SyntaxFlag flag) {
        if (!mSyntaxFlags.isNull()) {
            if (mSyntaxFlags->size() == 1 && mSyntaxFlags->contains(flag)) {
                mSyntaxFlags = nullptr;
                return;
            }
            clone();
            mSyntaxFlags->remove(flag);
        }
    }
};

class SyntaxAbstract;
class SyntaxCommentEndline;

struct SyntaxBlock
{
    SyntaxBlock(SyntaxAbstract* _syntax = nullptr, const SyntaxState &_state = SyntaxState(), int _start = 0, int _end = 0
            , bool _error = false , SyntaxShift _shift = SyntaxShift::shift, SyntaxKind _next = SyntaxKind::Standard)
        : syntax(_syntax), state(_state), start(_start), end(_end), error(_error), shift(_shift), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, const SyntaxState &_state, int _start, int _end, SyntaxKind _next, bool _error = false)
        : syntax(_syntax), state(_state), start(_start), end(_end), error(_error), shift(SyntaxShift::in), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, const SyntaxState &_state, int _start, int _end, SyntaxShift _shift, bool _error = false)
        : syntax(_syntax), state(_state), start(_start), end(_end), error(_error), shift(_shift),
          next(SyntaxKind::Standard)
    { }
    SyntaxAbstract *syntax;
    SyntaxState state;
    int start;
    int end;
    bool error;
    SyntaxShift shift;
    SyntaxKind next;
    int length() const { return end-start; }
    bool isValid() const { return syntax && start<end; }
};

class SyntaxFormula;
class SyntaxDcoBody;

class SharedSyntaxData
{
    QVector<SyntaxFormula*> mSubFormula;
    SyntaxDcoBody *mDcoBody = nullptr;
    SyntaxCommentEndline *mCommentEndline = nullptr;
public:
    void addFormula(SyntaxFormula* syntax) { if (syntax) mSubFormula << syntax; }
    void registerCommentEndLine(SyntaxCommentEndline * syntax) { if (syntax) mCommentEndline = syntax; }
    void registerDcoBody(SyntaxDcoBody * syntax) { if (syntax) mDcoBody = syntax; }
    bool isValid() { return mCommentEndline && mDcoBody && mSubFormula.size() == 5; }
    const QVector<SyntaxFormula*> allFormula() { return mSubFormula; }
    SyntaxCommentEndline *commentEndLine() { return mCommentEndline; }
    SyntaxDcoBody *dcoBody() { return mDcoBody; }
};

/// \brief An abstract class to be used inside the <c>SyntaxHighlighter</c>.
class SyntaxAbstract
{
public:
    SyntaxAbstract(SyntaxKind kind, SharedSyntaxData* sharedData) : mKind(kind), mSharedData(sharedData) {}
    virtual ~SyntaxAbstract() {}
    SyntaxKind kind() const { return mKind; }
    QString name() const { return syntaxKindName(mKind); }
    bool assignColorSlot(Theme::ColorSlot slot);
    Theme::ColorSlot colorSlot() const { return mColorSlot; }

    /// Finds the begin of this syntax
    virtual SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) = 0;

    /// Finds the end of valid trailing characters for this syntax
    virtual SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) = 0;
    virtual const SyntaxTransitions nextKinds(bool emptyLine = false);
    virtual QTextCharFormat& charFormat() { return mCharFormat; }
    virtual QTextCharFormat charFormatError();
    virtual int maxNesting() { return 0; }
    virtual void copyCharFormat(const QTextCharFormat &charFormat) { mCharFormat = charFormat; }
    virtual QStringList docForLastRequest() const { return QStringList(); }
    int intSyntaxType() { return static_cast<int>(kind()); }

    static const QPair<QString, QString> systemEmpData;

protected:
    static const QVector<QChar> cSpecialCharacters;  // other breaking kind

    enum CharClass {ccOther, ccSpecial, ccAlpha};
    inline int charClass(QChar ch, int &prev, const QVector<QChar> &moreSpecialChars = QVector<QChar>()) {
        // ASCII:   "   $   '   .   0  9   ;   =   A  Z   _   a   z
        // Code:   34, 36, 39, 46, 48-57, 59, 61, 65-90, 95, 97-122
        if (ch < '"' || ch > 'z')
            prev = ccOther;
        else if (ch >= 'a' || (ch >= 'A' && ch <= 'Z') || ch == '_')
            prev = ccAlpha;  // break by keyword kind
        else if (ch >= '0' && ch <= '9') {
            if (prev != ccAlpha) prev = ccOther;
        } else {
            prev = (cSpecialCharacters.contains(ch) || moreSpecialChars.contains(ch)) ? ccSpecial : ccOther;
        }
        return prev;
    }

    int endOfQuotes(const QString &line, const int &start);
    int endOfParentheses(const QString &line, const int &start, const QString &validPairs, int &nest);

    inline bool isKeywordChar(const QChar& ch, const QString &extraChars = QString()) {
        return (ch.isLetterOrNumber() || extraChars.contains(ch));
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
    bool hasMatchingSuffix(QChar typeChar, SyntaxState &state, const QString &line, int &pos, QString &suffix);

protected:
    SyntaxKind mKind;
    Theme::ColorSlot mColorSlot = Theme::ColorSlot::invalid;
    QTextCharFormat mCharFormat;
    SyntaxTransitions mSubKinds;
    SyntaxTransitions mEmptyLineKinds;
    SharedSyntaxData *mSharedData = nullptr;

private:
    bool convertAndCompareSuffix(QString &suffix, QChar outType, const QString &outName);
};


/// \brief Defines the syntax for standard code.
class SyntaxStandard : public SyntaxAbstract
{
public:
    SyntaxStandard(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxFormula;
class SyntaxDcoBody;
/// \brief Defines the syntax for a DCO.
class SyntaxDco : public SyntaxAbstract
{
public:
    SyntaxDco(SharedSyntaxData* sharedData, QChar dcoChar = '$');
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
protected:
    QStringList docForLastRequest() const override;
private:
    QRegularExpression mRex;
    QStringList mDCOs;
    QStringList mDCOlow;
    QStringList mDescription;
    int mLastIKey = -1;
    QStringList mEndDCOs;
    QStringList mEndDCOlow;
    QStringList mEndDescription;
    int mLastEndIKey = -1;
    QMap<QString,int> mFlavors;
    QHash<QString, SyntaxKind> mSpecialKinds;

};

/// \brief Defines the syntax for the remain of a DCO line.
class SyntaxDcoBody: public SyntaxAbstract
{
    QList<QChar> mEolComChars;
public:
    SyntaxDcoBody(SyntaxKind kind, SharedSyntaxData* sharedData);
    void setCommentChars(const QList<QChar> &chars);
    QList<QChar> commentChars() const;
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

/// \brief Defines the syntax for a single comment line.
class SyntaxCommentLine: public SyntaxAbstract
{
public:
    SyntaxCommentLine(SharedSyntaxData* sharedData, QChar commentChar = '*');
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
private:
    QChar mCommentChar;
};

/// \brief Defines the syntax for a single comment line.
class SyntaxCommentEndline: public SyntaxAbstract
{
    QString mCommentChars;
public:
    SyntaxCommentEndline(SharedSyntaxData* sharedData, const QString &commentChars = "!!");
    void setCommentChars(const QString &commentChars);
    QString commentChars() const { return mCommentChars; }
    bool check(const QString &line, int index) const;
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
protected:
};

/// \brief Defines the syntax for a uniform multi-line block.
class SyntaxUniformBlock: public SyntaxAbstract
{
public:
    SyntaxUniformBlock(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

/// \brief Defines the syntax for a sub DCO part.
class SyntaxSubDCO: public SyntaxAbstract
{
    QStringList mSubDCOs;
public:
    SyntaxSubDCO(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxDelimiter: public SyntaxAbstract
{
    QChar mDelimiter;
public:
    SyntaxDelimiter(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxFormula: public SyntaxAbstract
{
    QVector<QChar> mSpecialDynamicChars;
public:
    SyntaxFormula(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
    void setSpecialDynamicChars(const QVector<QChar> &chars);
};

class SyntaxQuoted : public SyntaxAbstract
{
    QString mDelimiters;
public:
    SyntaxQuoted(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxAssign : public SyntaxAbstract
{
public:
    SyntaxAssign(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};


} // namespace syntax
} // namespace studio
} // namespace gams

#endif // SYNTAXFORMATS_H
