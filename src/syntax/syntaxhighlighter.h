#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QtGui>
#include "syntaxformats.h"
#include "syntaxdeclaration.h"
#include "textmark.h"
#include "textmarklist.h"

namespace gams {
namespace studio {

class ErrorHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    ErrorHighlighter(FileContext *context, TextMarkList* marks);
    void highlightBlock(const QString &text);
    void setDocAndConnect(QTextDocument* doc);
    void setMarks(TextMarkList* marks) {mMarks = marks;}

protected:
    void setCombiFormat(int start, int len, const QTextCharFormat& format, QList<TextMark*> marks);

private slots:
    void docBlockCountChanged(int newCount);
    void docContentsChange(int from, int removed, int added);

protected:
    TextMarkList* mMarks;

private:
    FileContext* mContext = nullptr;
    QTextBlock mTestBlock;

};

class SyntaxHighlighter : public ErrorHighlighter
{
    Q_OBJECT
public:
    SyntaxHighlighter(FileContext *context, TextMarkList *marks);
    ~SyntaxHighlighter();

    void highlightBlock(const QString &text);

private:
    SyntaxAbstract *getSyntax(SyntaxState state) const;
    int getStateIdx(SyntaxState state) const;

private:
    typedef int StateIndex;
    typedef int CodeIndex;
    typedef QPair<StateIndex, CodeIndex> StateCode;
    typedef QList<SyntaxAbstract*> States;
    typedef QList<StateCode> Codes;

    void addState(SyntaxAbstract* syntax, CodeIndex ci = 0);
    int addCode(StateIndex si, CodeIndex ci);
    int getCode(CodeIndex code, SyntaxStateShift shift, StateIndex state);

    States mStates;
    Codes mCodes;
};

} // namespace studio
} // namespace gams

#endif // SYNTAXHIGHLIGHTER_H
