#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QtGui>
#include "syntaxformats.h"

namespace gams {
namespace studio {

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
    SyntaxHighlighter(QTextDocument *parent = 0);
    ~SyntaxHighlighter();

    void highlightBlock(const QString &text);

private:
    SyntaxAbstract *getSyntax(int maxIdx, SyntaxState state) const;
    int getStateIdx(int maxIdx, SyntaxState state) const;

private:
    typedef QList<SyntaxAbstract*> States;

    States mStates;
};

} // namespace studio
} // namespace gams

#endif // SYNTAXHIGHLIGHTER_H
