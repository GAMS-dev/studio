/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
    if (!mDoc) return;
    setDirty(mDoc->firstBlock(), mDoc->lastBlock());
    processDirtyParts();
}

void BaseHighlighter::rehighlightBlock(const QTextBlock &block)
{
    if (!mDoc || !block.isValid()) return;
    mCurrentBlock = block;
    bool forceHighlightOfNextBlock = true;
    if (mTime.isNull()) mTime = QTime::currentTime();

    while (!mAborted && mCurrentBlock.isValid() && (forceHighlightOfNextBlock || !mDirtyBlocks.isEmpty())) {
        const int stateBeforeHighlight = mCurrentBlock.userState();

        reformatCurrentBlock();
        forceHighlightOfNextBlock = (mCurrentBlock.userState() != stateBeforeHighlight);
        setClean(mCurrentBlock);
        if (!mTime.isNull() && QTime::currentTime().msecsSinceStartOfDay()-mTime.msecsSinceStartOfDay() > 50) {
            mCurrentBlock = mCurrentBlock.next();
            if (forceHighlightOfNextBlock && mCurrentBlock.isValid())
                setDirty(mCurrentBlock, mCurrentBlock);
            mTime = QTime();
            break;
        }
        mCurrentBlock = nextDirty();
    }
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
//    DEB() << "dirty: " << QString::number(fromBlock.blockNumber()).rightJustified(2,'0')
//          << "-" << QString::number(lastBlock.blockNumber()).rightJustified(2,'0');
    rehighlightBlock(fromBlock);
}

QTextBlock cutEnd(QTextBlock block, int altLine, QTextDocument *doc)
{
    if (block.isValid() && (block < doc->lastBlock() || block == doc->lastBlock())) return block;
    if (block.isValid()) return doc->lastBlock();
    if (altLine < doc->blockCount()) return doc->findBlockByNumber(altLine);
    return doc->lastBlock();
}

void BaseHighlighter::blockCountChanged(int newBlockCount)
{
    if (!mDoc) return;
    for (int i = 0; i < mDirtyBlocks.size(); ++i) {
        mDirtyBlocks[i].setFirst(cutEnd(mDirtyBlocks.at(i).bFirst, mDirtyBlocks.at(i).first, mDoc));
        mDirtyBlocks[i].setSecond(cutEnd(mDirtyBlocks.at(i).bSecond, mDirtyBlocks.at(i).second, mDoc));
        if (mDirtyBlocks.at(i).isValid()) {
            mDirtyBlocks.remove(i--);
        }
    }
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
    if (mAborted || start < 0 || start >= mFormatChanges.count())
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

QTextBlock BaseHighlighter::nextDirty()
{
    QTextBlock res;
    if (!mDoc) return res;
    if (!mDirtyBlocks.isEmpty()) {
        res = mDirtyBlocks.first().bFirst;
        if (!res.isValid()) res = mDoc->findBlockByNumber(mDirtyBlocks.first().first);
        if (res.isValid()) return res;
    }
    return mCurrentBlock.next();
}

void BaseHighlighter::setDirty(QTextBlock fromBlock, QTextBlock toBlock)
{
    Q_ASSERT_X(fromBlock.isValid() && toBlock.isValid(), "BaseHighlighter::setDirty()", "invalid block");
    if (toBlock < fromBlock) return;
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
