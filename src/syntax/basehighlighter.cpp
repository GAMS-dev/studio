#include "basehighlighter.h"
#include "logger.h"
#include <QTimer>

namespace gams {
namespace studio {
namespace syntax {

BaseHighlighter::BaseHighlighter(QObject *parent) : QObject(parent)
{
    if (parent->inherits("QTextEdit")) {
        QTextDocument *doc = parent->property("document").value<QTextDocument *>();
        if (doc) setDocument(doc);
    }
}

BaseHighlighter::BaseHighlighter(QTextDocument *parent): QObject(parent)
{
    setDocument(parent);
}

BaseHighlighter::~BaseHighlighter()
{
    mDirtyBlocks.clear();
    setDocument(nullptr);
}

void BaseHighlighter::setDocument(QTextDocument *doc, bool wipe)
{
    if (mDoc) {
        disconnect(mDoc, &QTextDocument::contentsChange, this, &BaseHighlighter::reformatBlocks);
        if (wipe) {
            QTextCursor cursor(mDoc);
            cursor.beginEditBlock();
            for (QTextBlock block = mDoc->begin(); block.isValid(); block = block.next())
                block.layout()->clearFormats();
            cursor.endEditBlock();
        }
    }
    mBlockCount = 1;
    mDoc = doc;
    if (mDoc) {
        connect(mDoc, &QTextDocument::contentsChange, this, &BaseHighlighter::reformatBlocks);
        connect(mDoc, &QTextDocument::blockCountChanged, this, &BaseHighlighter::blockCountChanged);
        setDirty(0);
        QTimer::singleShot(0, this, &BaseHighlighter::processDirtyParts);
    }
}

QTextDocument *BaseHighlighter::document() const
{
    return mDoc;
}

void BaseHighlighter::rehighlight()
{
    if (!mDoc) return;
    setDirty(0);
    processDirtyParts();
}

void BaseHighlighter::rehighlightBlock(const QTextBlock &block)
{
    if (!block.isValid()) return;
    mCurrentBlock = block;
    bool forceHighlightOfNextBlock = true;
    int iDirty = dirtyIndex(mCurrentBlock.blockNumber());
    const int firstCleanBlockNr = mCurrentBlock.blockNumber();
    int count = 0;

    while (mDoc && mCurrentBlock.isValid() && (forceHighlightOfNextBlock || !mDirtyBlocks.isEmpty())) {
        const int stateBeforeHighlight = mCurrentBlock.userState();

        reformatCurrentBlock();
        forceHighlightOfNextBlock = (mCurrentBlock.userState() != stateBeforeHighlight);

        if (++count > cMaxCount) {
            if (forceHighlightOfNextBlock)
                setDirty(mCurrentBlock.blockNumber(), mCurrentBlock.blockNumber()+1);
            break;
        }

        if (!forceHighlightOfNextBlock && iDirty < mDirtyBlocks.size()
                && mCurrentBlock.blockNumber() == mDirtyBlocks.at(iDirty).second-1) {
            if (mDirtyBlocks.at(iDirty++).first < mCurrentBlock.blockNumber()) {
                if (iDirty < mDirtyBlocks.size()) {
                    mCurrentBlock = mDoc->findBlockByNumber(mDirtyBlocks.at(iDirty).first);
                    continue;
                }
            }
        }
        mCurrentBlock = mCurrentBlock.next();
    }
    mFormatChanges.clear();
    if (mDoc) {
        const int lastCleanBlockNr = mCurrentBlock.isValid() ? mCurrentBlock.blockNumber() : mDoc->blockCount();
        setClean(firstCleanBlockNr, lastCleanBlockNr);
        if (!mDirtyBlocks.isEmpty()) QTimer::singleShot(20, this, &BaseHighlighter::processDirtyParts);
//        else DEB() << "Highlight done";
    }
}

void BaseHighlighter::reformatBlocks(int from, int charsRemoved, int charsAdded)
{
    if (!mDoc) return;
    QTextBlock fromBlock = mDoc->findBlock(from);
    if (!fromBlock.isValid())
        return;
    QTextBlock lastBlock = mDoc->findBlock(from + charsAdded + (charsRemoved > 0 ? 1 : 0));
    if (!lastBlock.isValid())
        lastBlock = mDoc->lastBlock();
    setDirty(fromBlock.blockNumber(), (lastBlock.isValid() ? lastBlock.blockNumber()+1 : -1));
    mChangeLine = lastBlock.blockNumber();
    QString preHighlight = debugDirty();
    rehighlightBlock(fromBlock);
}

void BaseHighlighter::blockCountChanged(int newBlockCount)
{
    if (mChangeLine < 0) return;
    int change = newBlockCount - mBlockCount;
    for (int i = 0 ; i < mDirtyBlocks.size() ; ++i) {
        if (mDirtyBlocks.at(i).second > mChangeLine) {
            mDirtyBlocks[i].second += change;
            if (mDirtyBlocks.at(i).first > mChangeLine) mDirtyBlocks[i].first += change;
            if (mDirtyBlocks.at(i).isEmpty()) mDirtyBlocks.removeAt(i--);
        }
    }
    mChangeLine = -1;
    mBlockCount = newBlockCount;
}

void BaseHighlighter::processDirtyParts()
{
    if (mDirtyBlocks.isEmpty() || !mDoc) return;
    QTextBlock block = mDoc->findBlockByNumber(mDirtyBlocks.first().first);
    rehighlightBlock(block);
}

void BaseHighlighter::setFormat(int start, int count, const QTextCharFormat &format)
{
    if (start < 0 || start >= mFormatChanges.count())
        return;

    const int end = qMin(start + count, mFormatChanges.count());
    for (int i = start; i < end; ++i)
        mFormatChanges[i] = format;

}

QTextCharFormat BaseHighlighter::format(int pos) const
{
    if (pos < 0 || pos >= mFormatChanges.count())
        return QTextCharFormat();
    return mFormatChanges.at(pos);
}

int BaseHighlighter::previousBlockState() const
{
    if (!mCurrentBlock.isValid()) return -1;
    const QTextBlock previous = mCurrentBlock.previous();
    if (!previous.isValid()) return -1;

    return previous.userState();
}

int BaseHighlighter::currentBlockState() const
{
    if (!mCurrentBlock.isValid()) return -1;
    return mCurrentBlock.userState();
}

void BaseHighlighter::setCurrentBlockState(int newState)
{
    if (mCurrentBlock.isValid()) mCurrentBlock.setUserState(newState);
}

void BaseHighlighter::setCurrentBlockUserData(QTextBlockUserData *data)
{
    if (mCurrentBlock.isValid()) mCurrentBlock.setUserData(data);
}

QTextBlockUserData *BaseHighlighter::currentBlockUserData() const
{
    if (mCurrentBlock.isValid()) return nullptr;
    return mCurrentBlock.userData();
}

QTextBlock BaseHighlighter::currentBlock() const
{
    return mCurrentBlock;
}

void BaseHighlighter::reformatCurrentBlock()
{
    mFormatChanges.fill(QTextCharFormat(), mCurrentBlock.length() - 1);
    highlightBlock(mCurrentBlock.text());
    applyFormatChanges();
}

void BaseHighlighter::applyFormatChanges()
{
    // cloned from QSyntaxHighlighterPrivate
    bool formatsChanged = false;

    QTextLayout *layout = mCurrentBlock.layout();

    QVector<QTextLayout::FormatRange> ranges = layout->formats();

    const int preeditAreaStart = layout->preeditAreaPosition();
    const int preeditAreaLength = layout->preeditAreaText().length();

    if (preeditAreaLength != 0) {
        auto isOutsidePreeditArea = [=](const QTextLayout::FormatRange &range) {
            return range.start < preeditAreaStart
                    || range.start + range.length > preeditAreaStart + preeditAreaLength;
        };
        const auto it = std::remove_if(ranges.begin(), ranges.end(),
                                       isOutsidePreeditArea);
        if (it != ranges.end()) {
            ranges.erase(it, ranges.end());
            formatsChanged = true;
        }
    } else if (!ranges.isEmpty()) {
        ranges.clear();
        formatsChanged = true;
    }

    int i = 0;
    while (i < mFormatChanges.count()) {
        QTextLayout::FormatRange r;

        while (i < mFormatChanges.count() && mFormatChanges.at(i) == r.format)
            ++i;

        if (i == mFormatChanges.count())
            break;

        r.start = i;
        r.format = mFormatChanges.at(i);

        while (i < mFormatChanges.count() && mFormatChanges.at(i) == r.format)
            ++i;

        Q_ASSERT(i <= mFormatChanges.count());
        r.length = i - r.start;

        if (preeditAreaLength != 0) {
            if (r.start >= preeditAreaStart)
                r.start += preeditAreaLength;
            else if (r.start + r.length >= preeditAreaStart)
                r.length += preeditAreaLength;
        }

        ranges << r;
        formatsChanged = true;
    }

    if (formatsChanged) {
        layout->setFormats(ranges);
        mDoc->markContentsDirty(mCurrentBlock.position(), mCurrentBlock.length());
    }

}

void BaseHighlighter::setDirty(int fromBlock, int toBlock)
{
    // TODO(JM) rework with Interval
    if (fromBlock == toBlock) return;
    if (toBlock < 0) toBlock = mDoc->blockCount();
    mDirtyBlocks << Interval(fromBlock, toBlock);
    std::sort(mDirtyBlocks.begin(), mDirtyBlocks.end());

    for (int i = 0; i < mDirtyBlocks.size()-1; i++)  {
        if (mDirtyBlocks[i+1].extendOverlap(mDirtyBlocks.at(i)))
            mDirtyBlocks.removeAt(i--);
    }
}

void BaseHighlighter::setClean(int fromBlock, int toBlock)
{
    if (fromBlock == toBlock) return;
    if (toBlock < 0) toBlock = mDoc->blockCount();
    Interval cleaner(fromBlock, toBlock);
    for (int i = 0; i < mDirtyBlocks.size(); i++)  {
        Interval newBlock = mDirtyBlocks[i].subtractOverlap(cleaner);
        if (!newBlock.isEmpty())
            mDirtyBlocks.insert(++i, newBlock);
        else if (mDirtyBlocks[i].isEmpty())
            mDirtyBlocks.removeAt(i--);
    }
}

bool BaseHighlighter::Interval::extendOverlap(const BaseHighlighter::Interval &other)
{
    if (first > other.second || second < other.first) return false;     // disjoint
    first = qMin(first, other.first);
    second = qMax(second, other.second);
    return true;
}

BaseHighlighter::Interval BaseHighlighter::Interval::subtractOverlap(const BaseHighlighter::Interval &other)
{
    if (first >= other.second || second <= other.first)     // disjoint
        return Interval();

    if (first >= other.first && second <= other.second) {   // complete overlap
        second = first;
    } else if (first >= other.first) {
        first = other.second;
    } else if (second <= other.second) {
        second = other.first;
    } else {
        Interval res(other.second, second);
        second = other.first;
        return res; // return valid second part
    }
    return Interval();
}

} // namespace syntax
} // namespace studio
} // namespace gams
