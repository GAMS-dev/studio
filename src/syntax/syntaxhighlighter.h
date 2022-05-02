/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include "theme.h"
#include "blockdata.h"

namespace gams {
namespace studio {

struct ParenthesesPos;
typedef int CodeRelationIndex;

namespace syntax {

class SyntaxHighlighter : public BaseHighlighter
{
    Q_OBJECT
public:
    SyntaxHighlighter(QTextDocument *doc);
    ~SyntaxHighlighter() override;

    void highlightBlock(const QString &text) override;
    void reloadColors();

public slots:
    void syntaxKind(int position, int &intKind, int &flavor);
    void scanSyntax(QTextBlock block, QMap<int, QPair<int,int>> &blockSyntax, int pos = -1);
    void syntaxDocAt(QTextBlock block, int pos, QStringList &syntaxDoc);

private:
    void scanParentheses(const QString &text, SyntaxBlock block, SyntaxKind preKind, NestingData &nestingData);
    QString parseName(const QString &text, int start);

private:
    struct CodeRelation {
        CodeRelation(BlockCode code, CodeRelationIndex prevCri) : blockCode(code), prevCodeRelIndex(prevCri) {}
        bool operator ==(const CodeRelation &other) {
            return blockCode == other.blockCode && prevCodeRelIndex == other.prevCodeRelIndex; }
        BlockCode blockCode;
        CodeRelationIndex prevCodeRelIndex;
    };
//    typedef QPair<KindIndex, CodeIndex> KindCodeX;
    typedef QHash<SyntaxKind, SyntaxAbstract*> Kinds;
    typedef QList<CodeRelation> CodeRelations;

    void initKind(int debug, SyntaxAbstract* syntax, Theme::ColorSlot slot = Theme::Syntax_neutral);
    void initKind(SyntaxAbstract* syntax, Theme::ColorSlot slot = Theme::Syntax_neutral);

    int addCode(BlockCode code, CodeRelationIndex parentIndex);
    CodeRelationIndex getCode(CodeRelationIndex cri, SyntaxShift shift, SyntaxBlock block, int nest = 0);
    int purgeCode(CodeRelationIndex cri);
    QString codeDeb(CodeRelationIndex cri);
    void syntaxDebug(SyntaxBlock syntaxBlock, QString syntaxName, int prevFlavor);
    void syntaxDebug(QString text);

private:
    int mScanBlockNr = -1;
    int mScanPosInBlock = -1;
    QMap<int, QPair<int, int> > mScannedBlockSyntax;
    QStringList mScannedPosDoc;
    int mPositionForSyntaxKind = -1;
    int mLastSyntaxKind = 0;
    int mLastFlavor = 0;
    QVector<SyntaxKind> mSingleLineKinds;
    QVector<SyntaxKind> mPostKindBlocker;
    QVector<SyntaxAbstract*> mPostSyntax;
    Kinds mKinds;
    CodeRelations mCodes;

    static const QVector<SyntaxKind> cInvalidParenthesesSyntax;
    static const QString cValidParentheses;
    static const QString cSpecialBlocks;
    static const QString cFlavorChars;
};

} // namespace syntax
} // namespace studio
} // namespace gams

#endif // SYNTAXHIGHLIGHTER_H
