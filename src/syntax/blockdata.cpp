#include "blockdata.h"

namespace gams {
namespace studio {
namespace syntax {

BlockData::~BlockData()
{ }

BlockData *BlockData::fromTextBlock(QTextBlock block)
{
    return (block.isValid() && block.userData()) ? static_cast<BlockData*>(block.userData())
                                                 : nullptr;
}

QChar BlockData::charForPos(int relPos)
{
    for (int i = mParentheses.count()-1; i >= 0; --i) {
        if (mParentheses.at(i).relPos == relPos || mParentheses.at(i).relPos-1 == relPos) {
            return mParentheses.at(i).character;
        }
    }
    return QChar();
}

QVector<ParenthesesPos> BlockData::parentheses() const
{
    return mParentheses;
}

void BlockData::setParentheses(const QVector<ParenthesesPos> &parentheses, const NestingImpact &nestingImpact)
{
    mParentheses = parentheses;
    mNestingImpact = nestingImpact;
}


} // namespace syntax
} // namespace studio
} // namespace gams
