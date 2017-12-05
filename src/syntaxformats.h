#ifndef SYNTAXFORMATS_H
#define SYNTAXFORMATS_H

#include <QtGui>

namespace gams {
namespace studio {

enum class SyntaxState {
    Standard,
    Directive,
    CommentLine,
    CommentBlock,
    CommentEndline,
    CommentInline,
    CommentMargin,

    StateCount
};
typedef QList<SyntaxState> SyntaxStates;


/// \brief An abstract class to be used inside the <c>SyntaxHighlighter</c>.
class SyntaxAbstract
{
public:
    virtual ~SyntaxAbstract() {}
    virtual SyntaxState state() = 0;
    virtual SyntaxState process(SyntaxState fromState, const QString &line, int& start, int& end) = 0;
    virtual SyntaxStates subStates() {return mSubStates;}
    virtual const QTextCharFormat* charFormat() {return &mCharFormat;}
protected:
    QTextCharFormat mCharFormat;
    SyntaxStates mSubStates;
};


/// \brief Defines the syntax for standard code.
class SyntaxStandard : public SyntaxAbstract
{
public:
    SyntaxStandard();
    SyntaxState state() override {return SyntaxState::Standard;}
    SyntaxState process(SyntaxState fromState, const QString &line, int& start, int& end) override;
};


/// \brief Defines the syntax for a directive.
class SyntaxDirective : public SyntaxAbstract
{
public:
    SyntaxDirective(QChar directiveChar = '$');
    SyntaxState state() override {return SyntaxState::Directive;}
    SyntaxState process(SyntaxState fromState, const QString &line, int& start, int& end) override;
private:
    QStringList mDirectives;
    QChar mDirectiveChar;
};


/// \brief Defines the syntax for standard code.
class SyntaxCommentLine: public SyntaxAbstract
{
public:
    SyntaxCommentLine(QChar commentChar = '*');
    SyntaxState state() override {return SyntaxState::CommentLine;}
    SyntaxState process(SyntaxState fromState, const QString &line, int& start, int& end) override;
private:
    QChar mCommentChar;
};

} // namespace studio
} // namespace gams

#endif // SYNTAXFORMATS_H
