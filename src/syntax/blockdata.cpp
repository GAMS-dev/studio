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
#include "blockdata.h"

namespace gams {
namespace studio {
namespace syntax {

NestingData::NestingData()
{
    mParentheses.reserve(20);
}

void NestingData::addOpener(QChar _character, int _relPos)
{
    mParentheses << ParenthesesPos(_character, _relPos);
    ++mImpact;
}

void NestingData::addCloser(QChar _character, int _relPos)
{
    mParentheses << ParenthesesPos(_character, _relPos);
    --mImpact;
    if (mImpact<mMaxDepth) mMaxDepth = mImpact;
}

BlockData *BlockData::fromTextBlock(QTextBlock block)
{
    return (block.isValid() && block.userData()) ? static_cast<BlockData*>(block.userData())
                                                 : nullptr;
}

BlockData::~BlockData()
{ }

QChar BlockData::charForPos(int relPos)
{
    for (int i = mNestingData.parentheses().count()-1; i >= 0; --i) {
        if (mNestingData.parentheses().at(i).relPos == relPos || mNestingData.parentheses().at(i).relPos-1 == relPos) {
            return mNestingData.parentheses().at(i).character;
        }
    }
    return QChar();
}

QVector<ParenthesesPos> BlockData::parentheses() const
{
    return mNestingData.parentheses();
}

void BlockData::setParentheses(const NestingData &nestingData)
{
    mNestingData = nestingData;
}


} // namespace syntax
} // namespace studio
} // namespace gams
