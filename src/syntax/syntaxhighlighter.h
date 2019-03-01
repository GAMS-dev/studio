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
#include "textmarkrepo.h"
#include "logger.h"

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

///
///
///
class BlockCode
{
    static const int b1 = 65536;
    static const int b2 = 512;
    static const int b3 = 64;
    int mCode;
public:
    BlockCode(int code) : mCode(code) { if (code < 0) DEB() << "Error: code must not be < 0"; }
    int code() const { return mCode; }
    int kind() const { return mCode % b1; }
    int depth() const { return (mCode / b1) % b2; } // up to one element on the kind-stack may have nesting
    int parser() const { return mCode / (b1 * b2); } // parser==0 is GAMS syntax, others like python may be added
    bool setKind(int _kind) {
        int val = qBound(0, _kind, b1-1);
        mCode = mCode - kind() + val;
        return val == _kind;
    }
    bool setDepth(int _depth) {
        int val = qBound(0, _depth, b2-1);
        mCode = mCode - (depth() * b1) + (val * b1);
        return val == _depth;
    }
    bool setParser(int _parser) {
        int val = qBound(0, _parser, b3-1);
        mCode = mCode - (parser() * b1 * b2) + (val * b1 * b2);
        return val == _parser;
    }
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    SyntaxHighlighter(QTextDocument *doc);
    ~SyntaxHighlighter();

    void highlightBlock(const QString &text);

public slots:
    void syntaxKind(int position, int &intKind);

private:
    SyntaxAbstract *getSyntax(SyntaxKind kind) const;
    int getKindIdx(SyntaxKind kind) const;
    void scanParentheses(const QString &text, int start, int len, SyntaxKind kind, QVector<ParenthesesPos> &parentheses);

private:
    enum FontModifier {fNormal, fBold, fItalic, fBoldItalic};
    Q_DECLARE_FLAGS(FontModifiers, FontModifier)
    typedef int KindIndex;
    typedef int CodeIndex;
    typedef QPair<KindIndex, CodeIndex> KindCode;
    typedef QList<SyntaxAbstract*> Kinds;
    typedef QList<KindCode> Codes;

    /// \brief addKind
    /// \param syntax The syntax to be added to the stack
    /// \param ci The index in mKinds of the previous syntax
    void addKind(SyntaxAbstract* syntax, CodeIndex ci = 0);
    void initKind(int debug, SyntaxAbstract* syntax, QColor color = QColor(), FontModifier fMod = fNormal);
    void initKind(SyntaxAbstract* syntax, QColor color = QColor(), FontModifier fMod = fNormal);

    int addCode(KindIndex si, CodeIndex ci);
    int getCode(CodeIndex code, SyntaxShift shift, KindIndex kind, KindIndex kindNext);
    QString codeDeb(int code);

private:
    int mPositionForSyntaxKind = -1;
    int mLastSyntaxKind = 0;
    Kinds mKinds;
    Codes mCodes;
    // TODO(JM) process events after a couple of ms
    // http://enki-editor.org/2014/08/22/Syntax_highlighting.html
};

} // namespace studio
} // namespace gams

#endif // SYNTAXHIGHLIGHTER_H
