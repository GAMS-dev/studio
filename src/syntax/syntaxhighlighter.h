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
#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "basehighlighter.h"
#include "syntaxformats.h"
#include "blockcode.h"
#include "logger.h"
#include "scheme.h"

namespace gams {
namespace studio {

struct ParenthesesPos;

namespace syntax {

class ProjectFileNode;

class SyntaxHighlighter : public BaseHighlighter
{
    Q_OBJECT
public:
    SyntaxHighlighter(QTextDocument *doc);
    ~SyntaxHighlighter();

    void highlightBlock(const QString &text);
    void reloadColors();

public slots:
    void syntaxKind(int position, int &intKind);

private:
    SyntaxAbstract *getSyntax(SyntaxKind kind) const;
    int getKindIdx(SyntaxKind kind) const;
    void scanParentheses(const QString &text, int start, int len, SyntaxKind preKind, SyntaxKind kind,SyntaxKind postKind, QVector<ParenthesesPos> &parentheses);

private:
    typedef int KindIndex;
    typedef int CodeIndex;
    typedef QPair<KindIndex, CodeIndex> KindCode;
    typedef QList<SyntaxAbstract*> Kinds;
    typedef QList<KindCode> Codes;

    /// \brief addKind
    /// \param syntax The syntax to be added to the stack
    /// \param ci The index in mKinds of the previous syntax
    void addKind(SyntaxAbstract* syntax, CodeIndex ci = 0);
    void initKind(int debug, SyntaxAbstract* syntax, Scheme::ColorSlot slot = Scheme::Syntax_neutral);
    void initKind(SyntaxAbstract* syntax, Scheme::ColorSlot slot = Scheme::Syntax_neutral);

    int addCode(KindIndex si, CodeIndex ci);
    BlockCode getCode(BlockCode code, SyntaxShift shift, SyntaxBlock block, int nest = 0);
    int purgeCode(int code);
    QString codeDeb(int code);

private:
    int mPositionForSyntaxKind = -1;
    int mLastSyntaxKind = 0;
    QVector<SyntaxKind> mSingleLineKinds;
    Kinds mKinds;
    Codes mCodes;
};

} // namespace syntax
} // namespace studio
} // namespace gams

#endif // SYNTAXHIGHLIGHTER_H
