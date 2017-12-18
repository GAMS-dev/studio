#ifndef SYNTAXFORMATS_H
#define SYNTAXFORMATS_H

#include <QtGui>

namespace gams {
namespace studio {

enum class SyntaxState {
    Standard,
    Directive,
    Title,
    CommentLine,
    CommentBlock,
    CommentEndline,
    CommentInline,
    CommentMargin,
    DeclarationSetType,         // must be followed by Declaration
    DeclarationVariableType,    // must be followed by Declaration
    Declaration,
    InsideDeclaration,

    StateCount
};
typedef QList<SyntaxState> SyntaxStates;

enum class SyntaxStateShift {
    stay,
    in,
    out
};

class SyntaxAbstract;

struct SyntaxBlock
{
    SyntaxBlock(SyntaxAbstract* _syntax=nullptr, int _start=0, int _end=0, bool _error=false
            , SyntaxStateShift _shift = SyntaxStateShift::stay, SyntaxState _next=SyntaxState::Standard)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(_shift), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, int _start, int _end, SyntaxState _next, bool _error=false)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(SyntaxStateShift::in), next(_next)
    { }
    SyntaxBlock(SyntaxAbstract* _syntax, int _start, int _end, SyntaxStateShift _shift, bool _error=false)
        : syntax(_syntax), start(_start), end(_end), error(_error), shift(_shift), next(SyntaxState::Standard)
    { }
    SyntaxAbstract *syntax;
    int start;
    int end;
    bool error;
    SyntaxStateShift shift;
    SyntaxState next;
    int length() { return end-start; }
    bool isValid() { return syntax; }
};

/// \brief An abstract class to be used inside the <c>SyntaxHighlighter</c>.
class SyntaxAbstract
{
public:
    virtual ~SyntaxAbstract() {}
    virtual SyntaxState state() = 0;
    virtual SyntaxBlock find(SyntaxState entryState, const QString &line, int index) = 0;
    virtual SyntaxStates subStates() { return mSubStates; }
    virtual QTextCharFormat& charFormat() { return mCharFormat; }
    virtual QTextCharFormat charFormatError();
    virtual void copyCharFormat(QTextCharFormat charFormat) {
        mCharFormat = charFormat;
    }
protected:
    QTextCharFormat mCharFormat;
    SyntaxStates mSubStates;
};


/// \brief Defines the syntax for standard code.
class SyntaxStandard : public SyntaxAbstract
{
public:
    SyntaxStandard();
    inline SyntaxState state() override { return SyntaxState::Standard; }
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
};


/// \brief Defines the syntax for a directive.
class SyntaxDirective : public SyntaxAbstract
{
public:
    SyntaxDirective(QChar directiveChar = '$');
    inline SyntaxState state() override { return SyntaxState::Directive; }
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
private:
    QRegularExpression mRex;
    QStringList mDirectives;
    QHash<QString, SyntaxState> mSpecialStates;
};


/// \brief Defines the syntax for a single comment line.
class SyntaxTitle: public SyntaxAbstract
{
public:
    SyntaxTitle();
    inline SyntaxState state() override { return SyntaxState::Title; }
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
};

/// \brief Defines the syntax for a single comment line.
class SyntaxCommentLine: public SyntaxAbstract
{
public:
    SyntaxCommentLine(QChar commentChar = '*');
    inline SyntaxState state() override { return SyntaxState::CommentLine; }
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
private:
    QChar mCommentChar;
};

/// \brief Defines the syntax for a multi-line comment block.
class SyntaxCommentBlock: public SyntaxAbstract
{
public:
    SyntaxCommentBlock();
    inline SyntaxState state() override { return SyntaxState::CommentBlock; }
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
};

} // namespace studio
} // namespace gams

#endif // SYNTAXFORMATS_H
