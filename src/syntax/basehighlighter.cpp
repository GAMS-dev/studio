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
#include "basehighlighter.h"
#include "logger.h"
#include "blockdata.h"
#include "blockcode.h"
#include "settings.h"
#include <QTimer>

using namespace std::chrono_literals;

namespace gams {
namespace studio {
namespace syntax {

BaseHighlighter::BaseHighlighter(QObject *parent) : QObject(parent)
{
    if (parent->inherits("QTextEdit")) {
        QTextDocument *doc = parent->property("document").value<QTextDocument *>();
        if (doc) setDocument(doc);
        mPrevMaxBlockCount = Settings::settings()->toInt(skEdHighlightMaxLines);
    }
}

BaseHighlighter::BaseHighlighter(QTextDocument *parent): QObject(parent)
{
    setDocument(parent);
}

BaseHighlighter::~BaseHighlighter()
{
    mAborted = true;
    mDirtyBlocks.clear();
    setDocument(nullptr);
}

void BaseHighlighter::abortHighlighting()
{
    mAborted = true;
}

void BaseHighlighter::setDocument(QTextDocument *doc, bool wipe)
{
    if (mDoc) {
        abortHighlighting();
        disconnect(mDoc, &QTextDocument::contentsChange, this, &BaseHighlighter::reformatBlocks);
        disconnect(mDoc, &QTextDocument::blockCountChanged, this, &BaseHighlighter::blockCountChanged);
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
        setDirty(mDoc->firstBlock(), mDoc->lastBlock());
//        QTimer::singleShot(0, this, [this](){ mBlockCount = mDoc->blockCount(); });
        mBlockCount = mDoc->blockCount();
        QTimer::singleShot(0, this, &BaseHighlighter::processDirtyParts);
    } else {
        mDirtyBlocks.clear();
    }
}

QTextDocument *BaseHighlighter::document() const
{
    return mDoc;
}

void BaseHighlighter::pause()
{
    if (!mDoc) return;
    disconnect(mDoc, &QTextDocument::contentsChange, this, &BaseHighlighter::reformatBlocks);
    disconnect(mDoc, &QTextDocument::blockCountChanged, this, &BaseHighlighter::blockCountChanged);
    mAborted = true;
}

void BaseHighlighter::resume()
{
    if (!mDoc) return;
    mAborted = false;
    connect(mDoc, &QTextDocument::contentsChange, this, &BaseHighlighter::reformatBlocks);
    connect(mDoc, &QTextDocument::blockCountChanged, this, &BaseHighlighter::blockCountChanged);
    rehighlight();
}

void BaseHighlighter::rehighlight()
{
    bool skipFullHighlight = setMaxLines(Settings::settings()->toInt(skEdHighlightMaxLines));
    if (skipFullHighlight && mMaxLineLength == Settings::settings()->toInt(skEdHighlightBound)) {
        processDirtyParts();
        return;
    }
    mMaxLineLength = Settings::settings()->toInt(skEdHighlightBound);
    if (!mDoc) return;
    QTextBlock lastBlock = (mDoc->blockCount() > mPrevMaxBlockCount && mPrevMaxBlockCount >= 0) ?
                               mDoc->findBlockByNumber(mPrevMaxBlockCount) : mDoc->lastBlock();
    setDirty(mDoc->firstBlock(), lastBlock);
    processDirtyParts();
}

void BaseHighlighter::rehighlightBlock(const QTextBlock &block)
{
    if (!mDoc || !block.isValid()) return;
    mCurrentBlock = block;
    bool forceHighlightOfNextBlock = true;
    if (mTime.isNull()) mTime = QTime::currentTime();

    while (!mAborted && mCurrentBlock.isValid() && (forceHighlightOfNextBlock || !mDirtyBlocks.isEmpty())) {
        const int prevState(mCurrentBlock.userState());

        reformatCurrentBlock();

        forceHighlightOfNextBlock = (prevState != mCurrentBlock.userState());
        setClean(mCurrentBlock);
        if (!mTime.isNull() && QTime::currentTime().msecsSinceStartOfDay() - mTime.msecsSinceStartOfDay() > 50) {
            mCurrentBlock = nextDirty();
            if (forceHighlightOfNextBlock && mCurrentBlock.isValid())
                setDirty(mCurrentBlock, mCurrentBlock);
            mTime = QTime();
            break;
        }
        mCurrentBlock = nextDirty();
    }
    if (mCurrentBlock.blockNumber() == mPrevMaxBlockCount-1 && mPrevMaxBlockCount > mMaxBlockCount)
        mPrevMaxBlockCount = mMaxBlockCount;
    mFormatChanges.clear();
    if (mAborted) mDirtyBlocks.clear();
    if (!mDirtyBlocks.isEmpty()) QTimer::singleShot(0, this, &BaseHighlighter::processDirtyParts);
    else if (!mCompleted) {
        mCompleted = true;
        emit completed();
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
    setDirty(fromBlock, lastBlock);
    QTextBlock endBlock = mDoc->findBlock(from + charsAdded);
    mAddedBlocks = (fromBlock.blockNumber() < mPrevMaxBlockCount) ? endBlock.blockNumber() - fromBlock.blockNumber() : -1;
    QTimer::singleShot(10ms, this, &BaseHighlighter::processDirtyParts);
}

QTextBlock cutEnd(const QTextBlock& block, int altLine, QTextDocument *doc)
{
    if (block.isValid() && (block < doc->lastBlock() || block == doc->lastBlock())) return block;
    if (block.isValid()) return doc->lastBlock();
    if (altLine < doc->blockCount()) return doc->findBlockByNumber(altLine);
    return doc->lastBlock();
}

void BaseHighlighter::blockCountChanged(int newBlockCount)
{
    if (!mDoc) return;
    QTextBlock fromBlock;
    if (mAddedBlocks >= 0) {
        // a recent contentChanged effected the block count: adapt unformatted part
        int removed = mBlockCount - newBlockCount + mAddedBlocks;
        fromBlock = mDoc->findBlockByNumber(mPrevMaxBlockCount - removed);
        if (fromBlock.isValid()) {
            QTextBlock lastBlock = mDoc->findBlockByNumber(mPrevMaxBlockCount + mAddedBlocks - 1);
            if (!lastBlock.isValid())
                lastBlock = mDoc->lastBlock();
            setDirty(fromBlock, lastBlock, true);
        }
    }
    for (int i = 0; i < mDirtyBlocks.size(); ++i) {
        mDirtyBlocks[i].setFirst(cutEnd(mDirtyBlocks.at(i).bFirst, mDirtyBlocks.at(i).first, mDoc));
        mDirtyBlocks[i].setSecond(cutEnd(mDirtyBlocks.at(i).bSecond, mDirtyBlocks.at(i).second, mDoc));
        if (mDirtyBlocks.at(i).isValid()) {
            mDirtyBlocks.remove(i--);
        }
    }
    mBlockCount = newBlockCount;
    if (mAddedBlocks >= 0) {
        mAddedBlocks = -1;
        QTimer::singleShot(10ms, this, &BaseHighlighter::processDirtyParts);
    }
}

void BaseHighlighter::processDirtyParts()
{
    if (mDirtyBlocks.isEmpty() || !mDoc) return;
    QTextBlock block = mDoc->findBlockByNumber(mDirtyBlocks.first().first);
    rehighlightBlock(block);
}

void BaseHighlighter::setFormat(int start, int count, const QTextCharFormat &format)
{
    if (mAborted || start < 0 || start >= mFormatChanges.count())
        return;

    const int end = qMin(start + count, mFormatChanges.count());
    for (int i = start; i < end; ++i)
        mFormatChanges[i] = format;

}

QTextCharFormat BaseHighlighter::format(int pos) const
{
    if (pos < 0 || pos >= mFormatChanges.count() || mCurrentBlock.blockNumber() > mMaxBlockCount)
        return QTextCharFormat();
    return mFormatChanges.at(pos);
}

int BaseHighlighter::previousBlockCRIndex() const
{
    if (!mCurrentBlock.isValid()) return -1;
    const QTextBlock previous = mCurrentBlock.previous();
    if (!previous.isValid()) return -1;

    return previous.userState();
}

void BaseHighlighter::setCurrentBlockCRIndex(int newState)
{
    if (mCurrentBlock.isValid()) mCurrentBlock.setUserState(newState);
}

QTextBlock BaseHighlighter::currentBlock() const
{
    return mCurrentBlock;
}

bool BaseHighlighter::isUnformatted() const
{
    return mCurrentBlock.blockNumber() >= mMaxBlockCount;
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
        auto it = std::remove_if(ranges.begin(), ranges.end(),
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

QTextBlock BaseHighlighter::nextDirty()
{
    QTextBlock res;
    if (!mDoc) return res;
    if (mCurrentBlock.blockNumber() == mPrevMaxBlockCount - 1 && mPrevMaxBlockCount != mMaxBlockCount)
        mPrevMaxBlockCount = mMaxBlockCount;
    if (!mDirtyBlocks.isEmpty()) {
        res = mDirtyBlocks.first().bFirst;
        if (!res.isValid()) res = mDoc->findBlockByNumber(mDirtyBlocks.first().first);
        if (res.isValid()) return res;
    }
    if (mCurrentBlock.blockNumber() >= mPrevMaxBlockCount - 1) {
        return res;
    }
    return mCurrentBlock.next();
}

void BaseHighlighter::setDirty(const QTextBlock& fromBlock, QTextBlock toBlock, bool force)
{
    Q_ASSERT_X(fromBlock.isValid() && toBlock.isValid(), "BaseHighlighter::setDirty()", "invalid block");
    if (toBlock < fromBlock) return;
    if (!force) {
        if (fromBlock.blockNumber() >= mPrevMaxBlockCount)
            return;
        if (toBlock.blockNumber() >= mPrevMaxBlockCount) {
            toBlock = mDoc->findBlockByNumber(qMin(mDoc->blockCount(), mPrevMaxBlockCount)-1);
            if (!toBlock.isValid())
                return;
        }
    }
    mDirtyBlocks << Interval(fromBlock, toBlock);
    std::sort(mDirtyBlocks.begin(), mDirtyBlocks.end());

    for (int i = 0; i < mDirtyBlocks.size()-1; i++)  {
        if (mDirtyBlocks[i+1].extendOverlap(mDirtyBlocks.at(i)))
            mDirtyBlocks.removeAt(i--);
    }
}

void BaseHighlighter::setClean(QTextBlock block)
{
    Q_ASSERT_X(block.isValid(), "BaseHighlighter::setClean()", "invalid block");
    if (mDirtyBlocks.isEmpty()) return;

    // find interval (This requires mDirtyBlocks to be ordered and disjoint)
    int i = 0;
    while (i < mDirtyBlocks.size() && block.blockNumber() > mDirtyBlocks.at(i).second)
        ++i;
    if (i >= mDirtyBlocks.size() || block.blockNumber() < mDirtyBlocks.at(i).first)
        return;
    // block is part of interval mDirtyBlocks[i]
    if (block.blockNumber() == mDirtyBlocks.at(i).first) {
        mDirtyBlocks[i].setFirst(block.next());
        if (mDirtyBlocks[i].isValid())
            mDirtyBlocks.remove(i);
    } else if (block.blockNumber() == mDirtyBlocks.at(i).second) {
        mDirtyBlocks[i].setSecond(block.previous());
        if (mDirtyBlocks[i].isValid())
            mDirtyBlocks.remove(i);
    } else {
        Interval newBlock = mDirtyBlocks[i].setSplit(block);
        if (!newBlock.isValid())
            mDirtyBlocks.insert(++i, newBlock);
        else if (mDirtyBlocks[i].isValid())
            mDirtyBlocks.removeAt(i);
    }
}

int BaseHighlighter::maxLines() const
{
    if (mMaxBlockCount == std::numeric_limits<int>().max())
        return -1;
    return mMaxBlockCount;
}

bool BaseHighlighter::setMaxLines(int newMaxLines)
{
    if (newMaxLines < 0) newMaxLines = std::numeric_limits<int>().max();
    if (newMaxLines == mMaxBlockCount) return false;
    if (mDoc) {
        QTextBlock from = mDoc->findBlockByNumber(qMin(newMaxLines, mMaxBlockCount));
        QTextBlock to = mDoc->findBlockByNumber(qMax(newMaxLines, mMaxBlockCount));
        if (from.isValid() && to.isValid())
            setDirty(from, to, true);
    }
    mMaxBlockCount = (newMaxLines == -1) ? std::numeric_limits<int>().max() : newMaxLines;
    if (mPrevMaxBlockCount < mMaxBlockCount)
        mPrevMaxBlockCount = mMaxBlockCount;
    return true;
}

BaseHighlighter::Interval::Interval(QTextBlock firstBlock, QTextBlock secondBlock)
    : QPair<int,int>(0,0)
{
    if(!firstBlock.isValid()) return;
    if (!secondBlock.isValid())
        secondBlock = firstBlock;
    else if (secondBlock < firstBlock)
        qSwap(firstBlock, secondBlock);
    setFirst(firstBlock);
    setSecond(secondBlock);
}

bool BaseHighlighter::Interval::updateFromBlocks()
{
    if (!bFirst.isValid() || !bSecond.isValid()) return false;
    if (first != bFirst.blockNumber()) {
        first = bFirst.blockNumber();
    }
    if (second != bSecond.blockNumber()) {
        second = bSecond.blockNumber();
    }
    return true;
}

bool BaseHighlighter::Interval::extendOverlap(const BaseHighlighter::Interval &other)
{
    if (first > other.second || second < other.first) return false;     // disjoint
    first = qMin(first, other.first);
    second = qMax(second, other.second);
    return true;
}

BaseHighlighter::Interval BaseHighlighter::Interval::setSplit(QTextBlock &block)
{
    if (!block.isValid()) return Interval();
    if (bFirst.isValid()) first = bFirst.blockNumber();
    if (bSecond.isValid()) second = bSecond.blockNumber();
    if (block.blockNumber() < first || block.blockNumber() > second) {   // outside
        return Interval();
    } else if (first == block.blockNumber()) {
        setFirst(block.next());
    } else if (second == block.blockNumber()) {
        setSecond(block.previous());
    } else {
        Interval res(block.next(), bSecond);
        setSecond(block.previous());
        return res; // return valid second part
    }
    return Interval();
}

} // namespace syntax
} // namespace studio
} // namespace gams
