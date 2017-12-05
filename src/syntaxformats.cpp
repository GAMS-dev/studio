#include "syntaxformats.h"

namespace gams {
namespace studio {


SyntaxStandard::SyntaxStandard()
{
    mSubStates << SyntaxState::Directive << SyntaxState::CommentLine;
}

SyntaxState SyntaxStandard::process(SyntaxState fromState, const QString& line, int& start, int &end)
{
    Q_UNUSED(start);
    if (fromState == state())
        end = line.length();
    return fromState;
}

SyntaxDirective::SyntaxDirective(QChar directiveChar): mDirectiveChar(directiveChar)
{
    mDirectives << "dollar" << "offtext" << "ontext";
}

SyntaxState SyntaxDirective::process(SyntaxState fromState, const QString& line, int& start, int &end)
{
    static QSet<QChar::Category> cats = {QChar::Letter_Lowercase, QChar::Letter_Uppercase, QChar::Letter_Titlecase};

    if (!line.length() || start > 0 || line[0] != mDirectiveChar) {
        return fromState;
    }
    QStringList directives = (fromState != SyntaxState::CommentBlock) ? mDirectives
                                                                      : QStringList() << "offtext";
    QStringRef ref = line.rightRef(line.length()-1);
    for (const QString directive: directives) {
        if (ref.startsWith(directive, Qt::CaseInsensitive)
            && (ref.length()==directive.length() || !cats.contains(ref[directive.length()].category()))) {
            end = directive.length();
            break;
        }
    }
    start = -1;
    return fromState;
}


SyntaxCommentLine::SyntaxCommentLine(QChar commentChar): mCommentChar(commentChar)
{ }

SyntaxState SyntaxCommentLine::process(SyntaxState fromState, const QString& line, int& start, int &end)
{
    Q_UNUSED(fromState)
    if (start==0 && line.startsWith(mCommentChar))
        end = line.length();
    return fromState;
}


} // namespace studio
} // namespace gams
