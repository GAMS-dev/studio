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
    QChar character;
    int relPos;
};

struct NestingData
{
    NestingData();
    void addOpener(QChar _character, int _relPos);
    void addCloser(QChar _character, int _relPos);
    int impact() const { return mImpact; }
    int leftOpen() const { return mMaxDepth; }
    int rightOpen() const { return mImpact - mMaxDepth; }
    QVector<ParenthesesPos> parentheses() const { return mParentheses; }
    bool inNamedBlock() { return !mNamedBlock.isEmpty(); }
    QChar namedBlockType() { return mNamedBlockType; }
    QString namedBlock() { return mNamedBlock; }
    void setNamedBlock(QChar type, const QString &name) { mNamedBlockType = type; mNamedBlock = name; }
private:
    short mImpact = 0;
    short mMaxDepth = 0;
    QVector<ParenthesesPos> mParentheses;
    QChar mNamedBlockType;
    QString mNamedBlock;
};

class BlockData : public QTextBlockUserData
{
public:
    BlockData() {}
    ~BlockData() override;
    static BlockData *fromTextBlock(QTextBlock block);
    QChar charForPos(int relPos);
    bool isEmpty() {return mNestingData.parentheses().isEmpty();}
    QVector<ParenthesesPos> parentheses() const;
    void setParentheses(const NestingData &nestingData);
    NestingData nestingImpact() const { return mNestingData; }
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
