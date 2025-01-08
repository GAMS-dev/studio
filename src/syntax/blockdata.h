/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_BLOCKDATA_H
#define GAMS_STUDIO_BLOCKDATA_H

#include <QTextBlock>

namespace gams {
namespace studio {
namespace syntax {

struct ParenthesesPos
{
    ParenthesesPos() : character(QChar()), relPos(-1) {}
    ParenthesesPos(QChar _character, int _relPos) : character(_character), relPos(_relPos) {}
    bool operator !=(const ParenthesesPos &other) const { return character != other.character || relPos != other.relPos; }
    bool operator ==(const ParenthesesPos &other) const { return character == other.character && relPos == other.relPos; }
    QChar character;
    int relPos;
};

struct NestingData
{
    NestingData();
    NestingData(const NestingData &other);
    NestingData &operator =(const NestingData &other);
    bool operator !=(const NestingData &other) const;
    bool operator ==(const NestingData &other) const;
    void addOpener(QChar _character, int _relPos);
    void addCloser(QChar _character, int _relPos);
    int impact() const { return mImpact; }
    int leftOpen() const { return mMaxDepth; }
    int rightOpen() const { return mImpact - mMaxDepth; }
    QVector<ParenthesesPos> parentheses() const { return mParentheses; }
private:
    short mImpact = 0;
    short mMaxDepth = 0;
    QVector<ParenthesesPos> mParentheses;
};

class BlockData : public QTextBlockUserData
{
public:
    BlockData() {}
    ~BlockData() override;
    static BlockData *fromTextBlock(const QTextBlock& block);
    QChar charForPos(int relPos);
    QVector<ParenthesesPos> parentheses() const;
    void setParentheses(const NestingData &nestingData);
    NestingData &nesting() { return mNestingData; }
    int &foldCount() { return mFoldCount; }
    bool isFolded() const { return mFoldCount; }
    void setFoldCount(int foldCount) { mFoldCount = foldCount; }
    void setVar(int var) { mVar = var; }
    int &var() { return mVar; }

private:
    // if extending the data remember to enhance isEmpty()
    NestingData mNestingData;
    int mFoldCount = 0;
    int mVar = 0;
};

} // namespace syntax
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_BLOCKDATA_H
