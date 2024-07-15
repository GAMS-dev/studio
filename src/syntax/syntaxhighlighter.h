/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
    bool reloadColors();

public slots:
    void syntaxKind(int position, int &intKind, int &flavor);
    void scanSyntax(const QTextBlock &block, QMap<int, QPair<int,int>> &blockSyntax, int pos = -1);
    void syntaxDocAt(const QTextBlock &block, int pos, QStringList &syntaxDoc);
    void syntaxFlagData(const QTextBlock &block, gams::studio::syntax::SyntaxFlag flag, QString &value);

private:
    void scanParentheses(const QString &text, SyntaxBlock block, SyntaxKind preKind, NestingData &nestingData);

private:
    struct CodeRelation {
        CodeRelation(BlockCode code, const SyntaxFlags &sFlags, CodeRelationIndex prevCri)
            : blockCode(code), syntaxFlags(sFlags), prevCodeRelIndex(prevCri) {}
        bool operator ==(const CodeRelation &other) const {
            return blockCode == other.blockCode && prevCodeRelIndex == other.prevCodeRelIndex
                    && (syntaxFlags == other.syntaxFlags || equalFlags(other.syntaxFlags)); }
        bool equalFlags(const SyntaxFlags &other) const {
            if (syntaxFlags.isNull() || other.isNull() || syntaxFlags->size() != other->size()) return false;
            SyntaxFlagData::const_iterator iter;
            for (iter = syntaxFlags->constBegin(); iter != syntaxFlags->constEnd() ; ++iter) {
                if (!other->contains(iter.key()) || iter.value() != other->value(iter.key()))
                    return false;
            }
            return true;
        }
        BlockCode blockCode;
        SyntaxFlags syntaxFlags;
        CodeRelationIndex prevCodeRelIndex;
    };
//    typedef QPair<KindIndex, CodeIndex> KindCodeX;
    typedef QHash<SyntaxKind, SyntaxAbstract*> Kinds;
    typedef QList<CodeRelation> CodeRelations;

    void initKind(int debug, SyntaxAbstract* syntax, Theme::ColorSlot slot = Theme::Syntax_neutral);
    void initKind(SyntaxAbstract* syntax, Theme::ColorSlot slot = Theme::Syntax_neutral);

    int addCode(BlockCode code, const SyntaxFlags &synFlags, CodeRelationIndex parentIndex);
    CodeRelationIndex getCode(CodeRelationIndex cri, SyntaxShift shift, const SyntaxBlock &block, const SyntaxFlags &synFlags, int nest = 0);
    int purgeCode(CodeRelationIndex cri);
    QString codeDeb(CodeRelationIndex cri);
    void syntaxDebug(const SyntaxBlock &syntaxBlock, const QString &syntaxName, int prevFlavor);
    void syntaxDebug(const QString &text);

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
    static const QMap<Theme::ColorSlot, Theme::ColorSlot> cForeToBackground;
    static const QString cValidParentheses;
    static const QString cSpecialBlocks;
};

} // namespace syntax
} // namespace studio
} // namespace gams

#endif // SYNTAXHIGHLIGHTER_H
