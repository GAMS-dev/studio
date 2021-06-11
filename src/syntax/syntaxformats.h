/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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

// ===============================================================================================================
// TODO(JM) REMOVE-START |  these declarations are temporary until the definitions are available in syntaxdata.h

static const QList<QPair<QString, QString>> systemAttributes() {
    static const QList<QPair<QString, QString>> list = {
        {"Page", "Current page number"},
        {"ILine", "Current source line number beeing executed"},
        {"OPage", "Current listing page number"},
        {"Elapsed", "Elasped time in seconds since start of job"},
        {"Memory", "Memory in use"},
        {"TStart", "Time to restart GAMS"},
        {"TClose", "Time to save GAMS"},
        {"TComp", "Time to compile"},
        {"TExec", "Time to execute"},
        {"IncParentL", "Include file parent line number"},
        {"IncLine", "Include file line"},
        {"Line", "Line number in source code"},
        {"ListLine", "Line number on listing file"},
        {"PrLine", "Line on listing page"},
        {"PrPage", "Listing page number"},
        {"LicenseDateSysN", "GAMS module system date (numeric)"},
        {"LicenseDateN", "License date (numeric)"},
        {"LicenseDateMaintN", "Maintenance date (numeric)"},
        {"LicenseDateEvalN", "Evaluation date (numric)"},
        {"LicenseLevel", "GAMS license level"},
        {"LicenseDaysEval", "License evaluation days left"},
        {"LicenseDaysMaint", "License maintenance days left"},
        {"LicenseEval", "License is eval license"},
        {"IsAlfaBeta", "Indicates an Alfa or Beta bulid"},
        {"MaxInput", "Max input line length that can be processed"},
        {"Date", "Job date"},
        {"Time", "Job time"},
        {"Title", "Current listing title"},
        {"SFile", "Save file name"},
        {"RTime", "Restart file time"},
        {"RDate", "Restart file date"},
        {"RFile", "Restart file name"},
        {"IFile", "Input file"},
        {"OFile", "Output (Listing) file"},
        {"Version", "GAMS compiler version"},
        {"Platform", "Job platform"},
        {"Lice1", "License display line 1"},
        {"Lice2", "License display line 2"},
        {"GString", "GAMS system audit string"},
        {"SString", "Subsystem (Solver) audit string"},
        {"PFile", "Current put file"},
        {"VerID", "GAMS version ID"},
        {"Date1", "Job date format"},
        {"Tab", "Tab character"},
        {"IncParent", "Include file parent"},
        {"IncName", "Include file name"},
        {"FileSys", "Operating system type"},
        {"FE", "File extension"},
        {"FN", "File name"},
        {"FP", "File path"},
        {"ReDirLog", "Append redirection string into the logfile"},
        {"NullFile", "The null filename"},
        {"ErrorLevel", "System Errorlevel"},
        {"LicenseDateSysS", "GAMS module system date (string)"},
        {"LicenseDateS", "License date (string)"},
        {"LicenseDateMaintS", "Maintenance date (string)"},
        {"LicenseDateEvalS", "Evaluation date (string)"},
        {"LicenseLevelText", "GAMS license level text"},
        {"LicenseDC", "License number"},
        {"LicenseID", "License ID string"},
        {"LicenseLicensee", "License Holder"},
        {"LicenseInstitution", "License holding institution"},
        {"LicenseType", "License type"},
        {"LicenseMudText", "License MUD type"},
        {"LicenseVendor", "License Vendor"},
        {"LicensePlatform", "License Platform XXX"},
        {"LicensePlatformText", "License Platform Text"},
        {"HostPlatform", "Host platform"},
        {"GamsVersion", "GAMS version number"},
        {"GdxFileNameIn", "Standard gdx file name for input"},
        {"GdxFileNameOut", "Standard gdx file name for output"},
        {"BuildCode", "System build code"},
        {"GamsRelease", "GAMS Release number"},
        {"LicenseStatus", "License validation status code"},
        {"LicenseStatusText", "License validation status text"},
        {"PutFileName", "The filename of the currently active PUT file"},
        {"JobHandle", "Job handle of last async call"},
        {"UserName", "Operating system user name"},
        {"ComputerName", "Operating system computer name"},
        {"GamsReleaseMaint", "GAMS Release number with maintenance number suffix"},
        {"DirSep", "File or directory separator in file names"},
        {"MACAddress", "MAC address of the first network adapter"},
        {"UserConfigDir", "User writable directory that is searched for gamsconfig.yaml"},
        {"UserDataDir", "User writable directory that is searched for gamslice.txt and others"},
        {"LicenseFileName", "The file name of the license file currently used"},
    };
    return list;
}

static QList<QPair<QString, QString>> systemData() {
    QList<QPair<QString, QString>> list = {
        {"dollarOptions", "Dollar control options"},
        {"empty", "Empty label"},
        {"gamsParameters", "Command line parameters"},
        {"gamsParameterSynonyms", "Synonyms for command line parameters"},
        {"gamsParameterSynonymMap", "Map between command line parameters and their synonyms"},
        {"gamsFunctions", "Intrinsic functions"},
        {"modelTypes", "Model types"},
        {"platforms", "Platform code"},
        {"powerSetLeft", "Numbering system with base b and s digits"},
        {"powerSetRight", "Numbering system with base b and s digits"},
        {"predefinedSymbols", "Predefined symbols"},
        {"setConstants", "System data names"},
        {"solverNames", "Names of solvers and tools"},
        {"solverPlatformMap", "Map between solvers and platforms"},
        {"solverTypePlatformMap", "Map between solvers, model types and platforms"},
        {"systemSuffixes", "System suffixes"},
    };
    return list;
}


static QList<QPair<QString, QString>> keyPut() {
    QList<QPair<QString, QString>> list = {
        {"put", "Write output to defined file"},//
        {"put_utilities", "Execute external programs using put syntax"},//
        {"put_utility", "Execute external programs using put syntax"},//
        {"putclear", "Delete the title and header blocks of a put file"},
        {"putclose", "Close a put file (and write to it)"},
        {"putfmcl", "Delete the title and header blocks of a put file"},
        {"puthd", "Write to the header block of a put page"},//
        {"putheader", "Write to the header block of a put page"},//
        {"putpage", "Start a new page in a put file"},//
        {"puttitle", "Write to the title block of a put page"},//
        {"puttl", "Write to the title block of a put page"},//
    };
    return list;
}


static QList<QPair<QString, QString>> keySolve() {
    QList<QPair<QString, QString>> list = {
        {"solve", ""},
    };
    return list;
}


static QList<QPair<QString, QString>> keyOption() {
    QList<QPair<QString, QString>> list = {
        {"option", ""},
        {"options", ""},
    };
    return list;
}


static QList<QPair<QString, QString>> keyExecute() {
    QList<QPair<QString, QString>> list = {
        {"execute", ""},
    };
    return list;
}


static const QList<QPair<QString, QString>> systemCTConstText() {
    static const QList<QPair<QString, QString>> list = {
        {"solPrint", "Solution report print"},
        {"handleStatus", "Status of model instance"},
        {"solveLink", "Solver link option"},
        {"solveOpt", "Multiple solve management"},
        {"solveStat", "Solver status"},
        {"modelStat", "Model status"},
        {"platformCode", "Platform Code"},
    };
    return list;
}


static const QList<QPair<QString, int>> systemCTConstants() {
    static const QList<QPair<QString, int>> list = {
        {"solPrint.Summary", 0},
        {"solPrint.Report", 1},
        {"solPrint.Quiet", 2},
        {"solPrint.Off", 0},
        {"solPrint.On", 1},
        {"solPrint.Silent", 2},
        {"handleStatus.Unknown", 0},
        {"handleStatus.Running", 1},
        {"handleStatus.Ready", 2},
        {"handleStatus.Failure", 3},
        {"solveLink.Chain Script", 0},
        {"solveLink.Call Script", 1},
        {"solveLink.Call Module", 2},
        {"solveLink.Async Grid", 3},
        {"solveLink.Async Simulate", 4},
        {"solveLink.Load Library", 5},
        {"solveLink.ASync Threads", 6},
        {"solveLink.Threads Simulate", 7},
        {"solveOpt.Replace", 0},
        {"solveOpt.Merge", 1},
        {"solveOpt.Clear", 2},
        {"solveStat.Normal Completion", 1},
        {"solveStat.Iteration Interrupt", 2},
        {"solveStat.Resource Interrupt", 3},
        {"solveStat.Terminated By Solver", 4},
        {"solveStat.Evaluation Interrupt", 5},
        {"solveStat.Capability Problems", 6},
        {"solveStat.Licensing Problems", 7},
        {"solveStat.User Interrupt", 8},
        {"solveStat.Setup Failure", 9},
        {"solveStat.Solver Failure", 10},
        {"solveStat.Internal Solver Failure", 11},
        {"solveStat.Solve Processing Skipped", 12},
        {"solveStat.System Failure", 13},
        {"modelStat.Optimal", 1},
        {"modelStat.Locally Optimal", 2},
        {"modelStat.Unbounded", 3},
        {"modelStat.Infeasible", 4},
        {"modelStat.Locally Infeasible", 5},
        {"modelStat.Intermediate Infeasible", 6},
        {"modelStat.Feasible Solution", 7},
        {"modelStat.Integer Solution", 8},
        {"modelStat.Intermediate Non-Integer", 9},
        {"modelStat.Integer Infeasible", 10},
        {"modelStat.Licensing Problem", 11},
        {"modelStat.Error Unknown", 12},
        {"modelStat.Error No Solution", 13},
        {"modelStat.No Solution Returned", 14},
        {"modelStat.Solved Unique", 15},
        {"modelStat.Solved", 16},
        {"modelStat.Solved Singular", 17},
        {"modelStat.Unbounded - No Solution", 18},
        {"modelStat.Infeasible - No Solution", 19},
        {"platformCode.Unknown", 0},
        {"platformCode.DEG", 1},
        {"platformCode.LEG", 2},
        {"platformCode.WEX", 3},
    };
    return list;
}


// TODO(JM) REMOVE-END ===========================================================================================





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
    Solve,
    SolveBody,
    SolveKey,
    Option,
    OptionBody,
    OptionKey,
    Execute,
    ExecuteBody,
    ExecuteKey,
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
class SyntaxCommentEndline;

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

    enum CharClass {ccOther, ccSpecial, ccAlpha};
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

protected:
    SyntaxKind mKind;
    Theme::ColorSlot mColorSlot = Theme::ColorSlot::invalid;
    QTextCharFormat mCharFormat;
    SyntaxTransitions mSubKinds;
    SyntaxTransitions mEmptyLineKinds;
    SharedSyntaxData *mSharedData = nullptr;
};


/// \brief Defines the syntax for standard code.
class SyntaxStandard : public SyntaxAbstract
{
public:
    SyntaxStandard(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

class SyntaxFormula;
class SyntaxDcoBody;
/// \brief Defines the syntax for a DCO.
class SyntaxDco : public SyntaxAbstract
{
public:
    SyntaxDco(SharedSyntaxData* sharedData, QChar dcoChar = '$');
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
private:
    QRegularExpression mRex;
    QStringList mDCOs;
    QStringList mDescription;
    QMap<QString,int> mFlavors;
    QHash<QString, SyntaxKind> mSpecialKinds;

};


/// \brief Defines the syntax for a single comment line.
class SyntaxDcoBody: public SyntaxAbstract
{
    QVector<QChar> mEolComChars;
public:
    SyntaxDcoBody(SyntaxKind kind, SharedSyntaxData* sharedData);
    void setCommentChars(QVector<QChar> chars);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

/// \brief Defines the syntax for a single comment line.
class SyntaxCommentLine: public SyntaxAbstract
{
public:
    SyntaxCommentLine(SharedSyntaxData* sharedData, QChar commentChar = '*');
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
    SyntaxCommentEndline(SharedSyntaxData* sharedData, QString commentChars = "!!");
    void setCommentChars(QString commentChars);
    QString commentChars() const { return mCommentChars; }
    bool check(const QString &line, int index) const;
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
protected:
};

/// \brief Defines the syntax for a uniform multi-line block.
class SyntaxUniformBlock: public SyntaxAbstract
{
public:
    SyntaxUniformBlock(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

/// \brief Defines the syntax for a sub DCO part.
class SyntaxSubDCO: public SyntaxAbstract
{
    QStringList mSubDCOs;
public:
    SyntaxSubDCO(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

class SyntaxDelimiter: public SyntaxAbstract
{
    QChar mDelimiter;
public:
    SyntaxDelimiter(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

class SyntaxFormula: public SyntaxAbstract
{
    QVector<QChar> mSpecialDynamicChars;
public:
    SyntaxFormula(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
    void setSpecialDynamicChars(QVector<QChar> chars);
};

class SyntaxQuoted : public SyntaxAbstract
{
    QString mDelimiters;
public:
    SyntaxQuoted(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};

class SyntaxAssign : public SyntaxAbstract
{
public:
    SyntaxAssign(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, int flavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, int flavor, bool &hasContent) override;
};



} // namespace syntax
} // namespace studio
} // namespace gams

#endif // SYNTAXFORMATS_H
