#ifndef GAMS_STUDIO_BLOCKDATA_H
#define GAMS_STUDIO_BLOCKDATA_H

#include <QTextBlock>

namespace gams {
namespace studio {
namespace syntax {

struct NestingImpact
{
    NestingImpact() {}
    void addCloser() { --mImpact; if (mImpact<mMaxDepth) mMaxDepth = mImpact; }
    void addOpener() { ++mImpact; }
    int impact() { return mImpact; }
    int leftOpen() { return mMaxDepth; }
    int rightOpen() { return mImpact - mMaxDepth; }
private:
    short mImpact = 0;
    short mMaxDepth = 0;
};

struct ParenthesesPos
{
    ParenthesesPos() : character(QChar()), relPos(-1) {}
    ParenthesesPos(QChar _character, int _relPos) : character(_character), relPos(_relPos) {}
    QChar character;
    int relPos;
};

class BlockData : public QTextBlockUserData
{
public:
    BlockData() {}
    ~BlockData() override;
    static BlockData *fromTextBlock(QTextBlock block);
    QChar charForPos(int relPos);
    bool isEmpty() {return mParentheses.isEmpty();}
    QVector<ParenthesesPos> parentheses() const;
    void setParentheses(const QVector<ParenthesesPos> &parentheses, const NestingImpact &nestingImpact);
    NestingImpact nestingImpact() const { return mNestingImpact; }
    int &foldCount() { return mFoldCount; }
    bool isFolded() const { return mFoldCount; }
    void setFoldCount(int foldCount) { mFoldCount = foldCount; }

private:
    // if extending the data remember to enhance isEmpty()
    QVector<ParenthesesPos> mParentheses;
    NestingImpact mNestingImpact;
    int mFoldCount = 0;
};

} // namespace syntax
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_BLOCKDATA_H
