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
#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "syntaxformats.h"

namespace gams {
namespace studio {

class FileContext;
class TextMarkList;
class TextMark;
struct ParenthesesPos;

enum ColorEnum {
    SyntaxDirex,
    SyntaxDiBdy,
    SyntaxComnt,
    SyntaxTitle,
    SyntaxDeclr,
    SyntaxIdent,
    SyntaxKeywd,
    SyntaxDescr,
    SyntaxAssgn,
    SyntaxTabHd,
    SyntaxEmbed,
};

class ErrorHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    ErrorHighlighter(FileContext *context);
    void highlightBlock(const QString &text);
    void setDocAndConnect(QTextDocument* doc);
    TextMarkList* marks();

public slots:
    void syntaxState(int position, int &intState);

protected:
    void setCombiFormat(int start, int len, const QTextCharFormat& charFormat, QVector<TextMark*> markList);

protected:
    int mPositionForSyntaxState = -1;
    int mLastSyntaxState = 0;

private:
    FileContext* mContext = nullptr;
    QTextBlock mTestBlock;

};

class SyntaxHighlighter : public ErrorHighlighter
{
    Q_OBJECT
public:
    SyntaxHighlighter(FileContext *context);
    ~SyntaxHighlighter();

    void highlightBlock(const QString &text);

private:
    SyntaxAbstract *getSyntax(SyntaxState state) const;
    int getStateIdx(SyntaxState state) const;
    void scanparentheses(const QString &text, int start, int len, SyntaxState state, QVector<ParenthesesPos> &parentheses);

private:
    typedef int StateIndex;
    typedef int CodeIndex;
    typedef QPair<StateIndex, CodeIndex> StateCode;
    typedef QList<SyntaxAbstract*> States;
    typedef QList<StateCode> Codes;

    /// \brief addState
    /// \param syntax The syntax to be added to the stack
    /// \param ci The index in mStates of the previous syntax
    void addState(SyntaxAbstract* syntax, CodeIndex ci = 0);
    void initState(int debug, SyntaxAbstract* syntax, QColor color = QColor(), bool bold = false, bool italic = false);
    void initState(SyntaxAbstract* syntax, QColor color = QColor(), bool bold = false, bool italic = false, int debug = 0);

    int addCode(StateIndex si, CodeIndex ci);
    int getCode(CodeIndex code, SyntaxStateShift shift, StateIndex state, StateIndex stateNext);
    QString codeDeb(int code);

    States mStates;
    Codes mCodes;
};

} // namespace studio
} // namespace gams

#endif // SYNTAXHIGHLIGHTER_H
