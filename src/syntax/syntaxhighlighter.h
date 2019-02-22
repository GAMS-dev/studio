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

#include "errorhighlighter.h"

namespace gams {
namespace studio {

class ProjectFileNode;
class TextMarkList;
//class TextMark;
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
    SyntaxAsLab,
    SyntaxAsVal,
    SyntaxAssgn,
    SyntaxTabHd,
    SyntaxEmbed,
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    SyntaxHighlighter(QTextDocument *doc);
    ~SyntaxHighlighter();

    void highlightBlock(const QString &text);

public slots:
    void syntaxState(int position, int &intState);

private:
    SyntaxAbstract *getSyntax(SyntaxState state) const;
    int getStateIdx(SyntaxState state) const;
    void scanParentheses(const QString &text, int start, int len, SyntaxState state, QVector<ParenthesesPos> &parentheses);

private:
    enum FontModifier {fNormal, fBold, fItalic, fBoldItalic};
    Q_DECLARE_FLAGS(FontModifiers, FontModifier)
    typedef int StateIndex;
    typedef int CodeIndex;
    typedef QPair<StateIndex, CodeIndex> StateCode;
    typedef QList<SyntaxAbstract*> States;
    typedef QList<StateCode> Codes;

    /// \brief addState
    /// \param syntax The syntax to be added to the stack
    /// \param ci The index in mStates of the previous syntax
    void addState(SyntaxAbstract* syntax, CodeIndex ci = 0);
    void initState(int debug, SyntaxAbstract* syntax, QColor color = QColor(), FontModifier fMod = fNormal);
    void initState(SyntaxAbstract* syntax, QColor color = QColor(), FontModifier fMod = fNormal);

    int addCode(StateIndex si, CodeIndex ci);
    int getCode(CodeIndex code, SyntaxStateShift shift, StateIndex state, StateIndex stateNext);
    QString codeDeb(int code);

private:
    int mPositionForSyntaxState = -1;
    int mLastSyntaxState = 0;
    States mStates;
    Codes mCodes;
    // TODO(JM) process events after a couple of ms
    // http://enki-editor.org/2014/08/22/Syntax_highlighting.html
};

} // namespace studio
} // namespace gams

#endif // SYNTAXHIGHLIGHTER_H
