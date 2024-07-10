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
#ifndef BASEHIGHLIGHTER_H
#define BASEHIGHLIGHTER_H

#include <QObject>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextObject>
#include <QVector>
#include <QTime>

namespace gams {
namespace studio {
namespace syntax {

class BaseHighlighter : public QObject
{
    Q_OBJECT
public:
    explicit BaseHighlighter(QObject *parent = nullptr);
    explicit BaseHighlighter(QTextDocument *parent = nullptr);
    ~BaseHighlighter() override;
    void abortHighlighting();

    void setDocument(QTextDocument *doc, bool wipe = false);
    QTextDocument *document() const;
    void pause();
    void resume();
    int maxLines() const;
    bool setMaxLines(int newMaxLines);

signals:
    void needUnfold(QTextBlock block);
    void completed();

public slots:
    void rehighlight();
    void rehighlightBlock(const QTextBlock &startBlock);

private slots:
    void reformatBlocks(int from, int charsRemoved, int charsAdded);
    void blockCountChanged(int newBlockCount);
    void processDirtyParts();

protected:
    virtual void highlightBlock(const QString &text) = 0;
    void setFormat(int start, int count, const QTextCharFormat &format);
    QTextCharFormat format(int pos) const;

    int previousBlockCRIndex() const;
    void setCurrentBlockCRIndex(int newState);
    QTextBlock currentBlock() const;
    int maxLineLength() { return mMaxLineLength; }
    bool isUnformatted() const;

private:
    void reformatCurrentBlock();
    void applyFormatChanges();
    QTextBlock nextDirty();
    void setDirty(const QTextBlock& fromBlock, QTextBlock toBlock, bool force = false);
    void setClean(QTextBlock block);
    inline int dirtyIndex(int blockNr) {
        for (int i = 0; i < mDirtyBlocks.size(); ++i) {
            if (mDirtyBlocks.at(i).first > blockNr || mDirtyBlocks.at(i).second > blockNr)
                return i;
        }
        return mDirtyBlocks.size();
    }

private:
    class Interval : public QPair<int,int>  {
        void set(const QTextBlock &block, QTextBlock &bVal, int &val) {
            if (!block.isValid()) {
                bVal = QTextBlock();
                val = 0;
            } else {
                bVal = block;
                val = block.blockNumber();
            }
        }
    public:
        Interval(QTextBlock firstBlock = QTextBlock(), QTextBlock secondBlock = QTextBlock());
        virtual ~Interval() {}
        void setFirst(const QTextBlock& block = QTextBlock()) { set(block, bFirst, first); }
        void setSecond(const QTextBlock& block = QTextBlock()) { set(block, bSecond, second); }
        bool isValid() const { return !bFirst.isValid() || !bSecond.isValid() || second < first; }
        bool updateFromBlocks();
        bool extendOverlap(const Interval &other);
        Interval setSplit(QTextBlock &block);
        QTextBlock bFirst;  // backup for changes
        QTextBlock bSecond; // backup for changes
    };

//    class BInterval : public QPair<QTextBlock, QTextBlock>  {
//    public:
//        BInterval(QTextBlock first = QTextBlock(), QTextBlock second = QTextBlock())
//            : QPair<QTextBlock, QTextBlock>(qMin(first, second), qMax(first, second)) {}
//        bool isEmpty() const { return !first.isValid() || !second.isValid() || second < first; }
//        bool extendOverlap(const BInterval &other);
//        int iFirst() const {return first.isValid() ? first.blockNumber() : -1;}
//        int iSecond() const {return second.isValid() ? second.blockNumber() : -1;}
//        BInterval subtractOverlap(const BInterval &other);
//        virtual ~BInterval() {}
//    };

    QTime mTime;
    bool mAborted = false;
    bool mCompleted = false;
    QTextDocument *mDoc = nullptr;
    int mBlockCount = 1;
    int mMaxBlockCount = -1;
    int mPrevMaxBlockCount = -1;
    int mAddedBlocks = 0;
    int mMaxLineLength = -1;
    QTextBlock mCurrentBlock;
    QVector<Interval> mDirtyBlocks;             // disjoint regions of dirty blocks
    QVector<QTextCharFormat> mFormatChanges;

};

} // namespace syntax
} // namespace studio
} // namespace gams

#endif // BASEHIGHLIGHTER_H
