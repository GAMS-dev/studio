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
#include "blockdata.h"

namespace gams {
namespace studio {
namespace syntax {

NestingData::NestingData()
{
    mParentheses.reserve(20);
}

NestingData::NestingData(const NestingData &other) :
    mImpact(other.mImpact),
    mMaxDepth(other.mMaxDepth),
    mParentheses(other.mParentheses)
{}

NestingData &NestingData::operator =(const NestingData &other)
{
    mImpact = other.mImpact;
    mMaxDepth = other.mMaxDepth;
    mParentheses = other.mParentheses;
    return *this;
}

bool NestingData::operator !=(const NestingData &other) const
{
    return operator==(other);
}

bool NestingData::operator ==(const NestingData &other) const
{
    return  mImpact == other.mImpact &&
            mMaxDepth == other.mMaxDepth &&
            mParentheses == other.mParentheses;
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

BlockData *BlockData::fromTextBlock(const QTextBlock& block)
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
