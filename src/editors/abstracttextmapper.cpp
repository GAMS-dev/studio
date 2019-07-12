/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "abstracttextmapper.h"
#include "exception.h"
#include "logger.h"
#include <QFile>
#include <QTextStream>
#include <QGuiApplication>
#include <QClipboard>

namespace gams {
namespace studio {

AbstractTextMapper::AbstractTextMapper(QObject *parent): QObject(parent)
{
    mCodec = QTextCodec::codecForLocale();
    setMappingSizes();
    setPosAbsolute(nullptr, 0, 0);
}

AbstractTextMapper::Chunk *AbstractTextMapper::setActiveChunk(int chunkNr) const
{
    int foundIndex = -1;
    for (int i = mChunkCache.size()-1; i >= 0; --i) {
        if (mChunkCache.at(i)->nr == chunkNr) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex < 0) {
        if (mChunkCache.size() == maxChunksInCache()) {
            Chunk * delChunk = mChunkCache.takeFirst();
            chunkUncached(delChunk);
        }
        Chunk *newChunk = getChunk(chunkNr);
        if (!newChunk) return nullptr;
        mChunkCache << newChunk;

    } else if (foundIndex < mChunkCache.size()-1) {
        mChunkCache.move(foundIndex, mChunkCache.size()-1);
    }
    return mChunkCache.last();
}

AbstractTextMapper::Chunk *AbstractTextMapper::activeChunk()
{
    return mChunkCache.isEmpty() ? nullptr : mChunkCache.last();
}

void AbstractTextMapper::uncacheChunk(AbstractTextMapper::Chunk *&chunk)
{
    if (chunk && mChunkCache.contains(chunk)) {
        mChunkCache.removeAll(chunk);
        chunkUncached(chunk);
    }
}

AbstractTextMapper::~AbstractTextMapper()
{
}

QTextCodec *AbstractTextMapper::codec() const
{
    return mCodec;
}

void AbstractTextMapper::setCodec(QTextCodec *codec)
{
    mCodec = codec;
}

bool AbstractTextMapper::isEmpty() const
{
    return size() == 0;
}

bool AbstractTextMapper::updateMaxTop() // to be updated on change of size or mVisibleLineCount
{
    if (isEmpty()) return false;
    Chunk *chunk = setActiveChunk(chunkCount()-1);
    if (!chunk || !chunk->isValid()) return false;

    int remainingLines = debugMode() ? (bufferedLines()-1)/2 : bufferedLines()-1;
    while (remainingLines > 0) {
        remainingLines -= chunk->lineCount();
        if (remainingLines <= 0) {
            mMaxTopLine.chunkNr = chunk->nr;
            mMaxTopLine.localLine = -remainingLines;
            mMaxTopLine.absStart = chunk->bStart + chunk->lineBytes.at(-remainingLines);
            break;
        } else if (chunk->nr == 0) {
            mMaxTopLine.chunkNr = 0;
            mMaxTopLine.absStart = 0LL;
            mMaxTopLine.localLine = 0;
            break;
        }
        chunk = setActiveChunk(chunk->nr -1);
    }
    return true;
}

qint64 AbstractTextMapper::lastTopAbsPos()
{
    if (isEmpty()) return size();
    Chunk *chunk = setActiveChunk(chunkCount()-1);
    if (!chunk || !chunk->isValid()) return size();

    qint64 lastPos = 0LL;
    int remainingLines = debugMode() ? (visibleLineCount()-1)/2 : visibleLineCount()-1;
    while (remainingLines > 0) {
        remainingLines -= chunk->lineCount();
        if (remainingLines <= 0) {
            lastPos = chunk->bStart + chunk->lineBytes.at(-remainingLines);
            break;
        } else if (chunk->nr == 0) {
            break;
        }
        chunk = setActiveChunk(chunk->nr -1);
    }
    return lastPos;
}

void AbstractTextMapper::reset()
{
    mChunkCache.clear();
    mLastChunkWithLineNr = -1;
    mBytesPerLine = 20.0;
    mChunkLineNrs.clear();
    mChunkLineNrs.squeeze();
    mDelimiter.clear();
}

void AbstractTextMapper::createSection()
{
    return;
}

qint64 AbstractTextMapper::size() const
{
    return 0;
}

void AbstractTextMapper::invalidateLineOffsets(Chunk *chunk, bool cutRemain) const
{
    if (!chunk) return;
    for (int i = mChunkLineNrs.size(); i <= chunk->nr; ++i) {
        // fill missing elements
        mChunkLineNrs << ChunkLines(i);
    }
    ChunkLines *cl = &mChunkLineNrs[chunk->nr];
    cl->lineCount = chunk->lineCount();
    cl->linesStartPos = chunk->bStart + chunk->lineBytes.first();
    cl->linesByteSize = chunk->lineBytes.last() - chunk->lineBytes.first();
    if (chunk->nr == 0) {
        cl->startLineNr = 0;
    } else if (mChunkLineNrs[chunk->nr-1].startLineNr >= 0) {
        cl->startLineNr = mChunkLineNrs[chunk->nr-1].startLineNr + mChunkLineNrs[chunk->nr-1].lineCount;
    }
    if (mLastChunkWithLineNr < cl->chunkNr) {
        mLastChunkWithLineNr = cl->chunkNr;
        updateBytesPerLine(*cl);
    }
    if (cutRemain) {
        if (mLastChunkWithLineNr > cl->chunkNr) {
            mLastChunkWithLineNr = cl->chunkNr;
            updateBytesPerLine(*cl);
        }
        for (int i = chunk->nr + 1; i < mChunkLineNrs.size(); ++i) {
            ChunkLines *cl = &mChunkLineNrs[i];
            if (cl->lineCount < 0) break;
            cl->lineCount = -1;
            cl->startLineNr = -1;
        }
    }
}

void AbstractTextMapper::updateLineOffsets(Chunk *chunk) const
{
    if (!chunk) return;
    for (int i = mChunkLineNrs.size(); i <= chunk->nr; ++i) {
        // fill missing elements
        mChunkLineNrs << ChunkLines(i);
    }
    ChunkLines *cl = &mChunkLineNrs[chunk->nr];
    if (cl->lineCount < 0) { // init ChunkLines on first visit
        cl->lineCount = chunk->lineCount();
        cl->linesStartPos = chunk->bStart + chunk->lineBytes.first();
        cl->linesByteSize = chunk->lineBytes.last() - chunk->lineBytes.first();
        if (cl->chunkNr == 0) { // only for chunk0
            cl->startLineNr = 0;
            if (mLastChunkWithLineNr < 0) {
                mLastChunkWithLineNr = 0;
                updateBytesPerLine(*cl);
            }
        }
    }
    if (cl->chunkNr > 0) {
        ChunkLines *prevCl = &mChunkLineNrs[chunk->nr-1];
        // extend counting as far as known
        while (cl->chunkNr > mLastChunkWithLineNr) {
            // skip if current chunk is unknown or previous chunk has no line-numbers
            if (!cl->isKnown() || !prevCl->hasLineNrs()) break;
            cl->startLineNr = prevCl->startLineNr + prevCl->lineCount;
            if (mLastChunkWithLineNr < cl->chunkNr) {
                mLastChunkWithLineNr = cl->chunkNr;
                updateBytesPerLine(*cl);
            }

            if (cl->chunkNr == mChunkLineNrs.size()-1) break;
            prevCl = cl;
            cl = &mChunkLineNrs[cl->chunkNr+1];
        }
    }
}

bool AbstractTextMapper::setMappingSizes(int visibleLines, int chunkSizeInBytes, int chunkOverlap)
{
    // check constraints
    mVisibleLineCount = qBound(20, visibleLines, 100);
    mMaxLineWidth = qBound(100, chunkOverlap, 10000);
    mChunkSize = qMax(chunkOverlap *8, chunkSizeInBytes);
    updateMaxTop();
    return (mVisibleLineCount != visibleLines || mMaxLineWidth != chunkOverlap || mChunkSize != chunkSizeInBytes);
}

void AbstractTextMapper::setVisibleLineCount(int visibleLines)
{
    mVisibleLineCount = qMax(visibleLines, 20);
    updateMaxTop();
}

int AbstractTextMapper::visibleLineCount() const
{
    return mVisibleLineCount;
}

bool AbstractTextMapper::setTopOffset(qint64 absPos)
{
    // find chunk index
    absPos = qBound(0LL, absPos, size()-1);
    int iChunk = mChunkCache.isEmpty() ? 0 : mChunkCache.last()->nr;
    while (iChunk > 0 && mChunkLineNrs.at(iChunk).linesStartPos > absPos)
        --iChunk;
    while (iChunk < chunkCount()-1 && mChunkLineNrs.at(iChunk).linesEndPos() <= absPos)
        ++iChunk;
    Chunk *chunk = (iChunk < 0) ? nullptr : setActiveChunk(iChunk);
    if (!chunk) return false;

    // adjust top line
    int localByteNr = int(absPos - chunk->bStart);
    mTopLine.chunkNr = chunk->nr;
    mTopLine.localLine = 0;
    for (int i = 0; i < chunk->lineCount(); ++i) {
        if (chunk->lineBytes.at(i) > localByteNr)
            break;
        mTopLine.localLine = i;
    }
    mTopLine.absStart = chunk->bStart + chunk->lineBytes.at(mTopLine.localLine);

    return true;
}

bool AbstractTextMapper::setVisibleTopLine(double region)
{
    if (region < 0.0 || region > 1.0) return false;

    // estimate a chunk-index at or beyond the position
    qint64 absPos = qint64(region * lastTopAbsPos());

    int iChunk = qMin(int(absPos / mChunkSize), chunkCount()-1);
    if (iChunk <= mLastChunkWithLineNr) {
        while (iChunk >= 0 && mChunkLineNrs.at(iChunk).linesStartPos > absPos)
            --iChunk;
        while (iChunk < chunkCount()-1 && mChunkLineNrs.at(iChunk).linesEndPos() <= absPos)
            ++iChunk;
    }
    Chunk *chunk = (iChunk < 0) ? nullptr : setActiveChunk(iChunk);
    if (!chunk) return false;

    // get the chunk-local line number for the visibleTopLine
    int localByteNr = int(absPos - chunk->bStart);
    int line = -1;
    for (int i = 0; i < chunk->lineCount(); ++i) {
        if (chunk->lineBytes.at(i) > localByteNr)
            break;
        line = i;
    }

    // count up to the buffer-topLine
    int bufferTopLine = line - bufferedLines()/3;
    while (bufferTopLine < 0 && chunk->nr > 0) {
        chunk = setActiveChunk(chunk->nr-1);
        if (!chunk) return false;
        bufferTopLine += chunk->lineCount();
    }
    if (bufferTopLine < 0) {
        mVisibleOffset = bufferedLines()/3 + bufferTopLine -1;
        bufferTopLine = 0;
    } else {
        mVisibleOffset = bufferedLines()/3;
    }
    setTopOffset(chunk->bStart + chunk->lineBytes.at(bufferTopLine));
    return true;
}

bool AbstractTextMapper::setVisibleTopLine(int lineNr)
{
    if (lineNr < 0 || lineNr > knownLineNrs())
        return false; // out of counted-lines region

    int maxVisLineNr = qMax(0, qAbs(lineCount()) - visibleLineCount());
    if (lineNr > maxVisLineNr) lineNr = maxVisLineNr;

    int tl = absTopLine();
    if (visibleOffset() >= 0 && tl >= 0 && lineNr == tl + visibleOffset())
        return true; // nothing changed

    // calculate the topLine for this visibleTopLine
    tl = qBound(0, lineNr - visibleLineCount(), qMax(0, qAbs(lineCount())- bufferedLines()));

    int chunkNr = findChunk(tl);
    if (chunkNr >= 0) {
        Chunk* chunk = setActiveChunk(chunkNr);
        if (!chunk) return false;
        int byte = chunk->lineBytes.at(tl - mChunkLineNrs.at(chunkNr).startLineNr);
        setTopOffset(chunk->bStart + byte);
        mVisibleOffset = lineNr - tl; // absTopLine();
        return true;
    }
    return false;
}

int AbstractTextMapper::moveVisibleTopLine(int lineDelta)
{
    if (!lineDelta) return 0;
    if (debugMode())
        lineDelta /= 2;
    Chunk *chunk = setActiveChunk(mTopLine.chunkNr);
    if (!chunk) return 0;
    if (mTopLine.absStart == 0 && chunk->nr == 0) {
        // buffer is at absolute top
        int max = (mTopLine.absStart == mMaxTopLine.absStart) ? bufferedLines()-visibleLineCount()
                                                              : bufferedLines() / 3;
        int localTop = qBound(0, mVisibleOffset+lineDelta, max);
        lineDelta += mVisibleOffset-localTop;
        mVisibleOffset = localTop;
        if (!lineDelta) return mVisibleOffset;
    }

//      if (mTopLine.absStart == mMaxTopLine.absStart) {
    if (mVisibleOffset > bufferedLines()/3) {
        // buffer is at absolute end
        int localTop = qBound(bufferedLines()/3, mVisibleOffset+lineDelta, bufferedLines()-visibleLineCount());
        lineDelta += mVisibleOffset-localTop;
        mVisibleOffset = localTop;
        if (!lineDelta) {
            return mVisibleOffset;
        }
    }

    while (lineDelta < 0) { // move up
        lineDelta += mTopLine.localLine;
        if (lineDelta < 0) {
            if (chunk->nr == 0) {
                // top chunk reached: move mVisibleTopLine
                mTopLine.absStart = chunk->bStart;
                mTopLine.localLine = 0;
                mVisibleOffset = qMax(0, mVisibleOffset + lineDelta);
                return mVisibleOffset;
            } else {
                // continue with previous chunk
                chunk = setActiveChunk(chunk->nr - 1);
                if (!chunk) return 0;
                mTopLine.chunkNr = chunk->nr;
//                mTopLine.lineCount = chunk->lineCount();
                mTopLine.localLine = chunk->lineCount();
                mTopLine.absStart = chunk->bStart + chunk->lineBytes.at(mTopLine.localLine);
            }
        } else {
            mTopLine.localLine = lineDelta;
            mTopLine.absStart = chunk->bStart + chunk->lineBytes.at(mTopLine.localLine);
            return mVisibleOffset;
        }
    }

    while (lineDelta > 0) { // move down
        if (mTopLine.chunkNr == mMaxTopLine.chunkNr && mTopLine.localLine + lineDelta > mMaxTopLine.localLine) {
            // delta runs behind mMaxTopPos
            lineDelta -= mMaxTopLine.localLine - mTopLine.localLine;
            mTopLine = mMaxTopLine;
            // avoid scrolling to the end (keep some lines visible)
            mVisibleOffset = qMin(lineDelta + mVisibleOffset, bufferedLines() - visibleLineCount());
            return mVisibleOffset;
        }
        lineDelta -= mChunkLineNrs.at(mTopLine.chunkNr).lineCount - mTopLine.localLine; // subtract remaining line-count
        if (lineDelta < 0) {
            // delta is in this chunk
            mTopLine.localLine = mChunkLineNrs.at(mTopLine.chunkNr).lineCount + lineDelta;
            mTopLine.absStart = chunk->bStart + chunk->lineBytes.at(mTopLine.localLine);
            return mVisibleOffset;
        } else if (chunk->nr < chunkCount()-1) {
            // switch to next chunk
            chunk = setActiveChunk(chunk->nr + 1);
            if (!chunk) return 0;
            mTopLine.chunkNr = chunk->nr;
//            mTopLine.lineCount = chunk->lineCount();
            mTopLine.localLine = 0;
            mTopLine.absStart = chunk->bStart;
        }
    }
    return mVisibleOffset;
}

void AbstractTextMapper::scrollToPosition()
{
    if (mPosition.chunkNr < 0) return;
    double region = double(mPosition.absLinePos + mPosition.effectiveCharNr()) / size();
    setVisibleTopLine(region);
    moveVisibleTopLine(-5);
}

int AbstractTextMapper::absTopLine() const
{
    Chunk *chunk = setActiveChunk(mTopLine.chunkNr);
    if (!chunk) return 0;
    const ChunkLines &topCL = mChunkLineNrs.at(chunk->nr);
    if (topCL.startLineNr < 0) {
        qint64 absPos = topCL.linesStartPos + chunk->lineBytes.at(mTopLine.localLine);
        double estimateLine = absPos / mBytesPerLine;
        return -int(estimateLine);
    }
    return topCL.startLineNr + mTopLine.localLine;
}

int AbstractTextMapper::lineCount() const
{
    qint64 count = 0;
    if (mLastChunkWithLineNr == chunkCount()-1) {
        count = mChunkLineNrs.last().startLineNr + mChunkLineNrs.last().lineCount;  // counted
    } else {
        count = -qint64(size() / mBytesPerLine) - 1; // estimated
    }
    if (count >= std::numeric_limits<int>::max() || count <= std::numeric_limits<int>::min())
        EXCEPT() << "Textdata contains too many lines";
    return int(count);
}

int AbstractTextMapper::knownLineNrs() const
{
    if (mLastChunkWithLineNr < 0) return 0;
    ChunkLines cl = mChunkLineNrs.at(mLastChunkWithLineNr);
    if (cl.startLineNr < 0) return 0;
    return cl.startLineNr + cl.lineCount;
}

QByteArray AbstractTextMapper::rawLines(int localLineNrFrom, int lineCount, int chunkBorder, int &borderLine) const
{
    QByteArray res;
    if (mChunkCache.isEmpty()) return res;
    QPair<int,int> interval = QPair<int,int>(localLineNrFrom, lineCount); // <start, size>
    res.clear();
    borderLine = 0;
    while (interval.second) {
        QPair<int,int> chunkInterval;
        Chunk *chunk = chunkForRelativeLine(interval.first, &chunkInterval.first);
        if (!chunk) break;
        chunkInterval.second = qMin(interval.second, chunk->lineCount() - chunkInterval.first);
        QByteArray raw;
        raw.setRawData(static_cast<const char*>(chunk->bArray)+chunk->lineBytes.at(chunkInterval.first),
                       uint(chunk->lineBytes.at(chunkInterval.first+chunkInterval.second)
                            - chunk->lineBytes.at(chunkInterval.first) - mDelimiter.size()));
        if (res.isEmpty())
            res = raw;
        else {
            res.append(mDelimiter);
            res.append(raw);
        }
        interval.first += chunkInterval.second;
        interval.second -= chunkInterval.second;
        if (chunk->nr < chunkBorder) {
            borderLine = interval.first - localLineNrFrom + 1;
        }
        if (chunk->nr == chunkCount()-1) break;
    }
    return res;
}

int AbstractTextMapper::topChunk() const
{
    Chunk *chunk = mChunkCache.isEmpty() ? nullptr : mChunkCache.last();
    return chunk ? chunk->nr : -1;
}

QString AbstractTextMapper::lines(int localLineNrFrom, int lineCount) const
{
    QString res;
    if (mChunkCache.isEmpty()) return QString();
    QPair<int,int> interval = QPair<int,int>(localLineNrFrom, lineCount); // <start, size>
    while (interval.second) {
        QPair<int,int> chunkInterval;
        Chunk *chunk = chunkForRelativeLine(interval.first, &chunkInterval.first);
        if (!chunk) break;
        chunkInterval.second = qMin(interval.second, chunk->lineCount() - chunkInterval.first);
        QByteArray raw;
        raw.setRawData(static_cast<const char*>(chunk->bArray)+chunk->lineBytes.at(chunkInterval.first),
                       uint(chunk->lineBytes.at(chunkInterval.first+chunkInterval.second)
                            - chunk->lineBytes.at(chunkInterval.first) - mDelimiter.size()));
        if (!res.isEmpty()) res.append(mDelimiter);
        res.append(mCodec ? mCodec->toUnicode(raw) : QString(raw));
        interval.first += chunkInterval.second;
        interval.second -= chunkInterval.second;
        if (chunk->nr == chunkCount()-1) {
            break;
        }

    }
    return res;
    // get the text of the line
}

QString AbstractTextMapper::lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const
{
    return lines(localLineNrFrom, lineCount);
}

bool AbstractTextMapper::findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind)
{
    bool backwards = flags.testFlag(QTextDocument::FindBackward);
    int part = backwards ? 2 : 1;
    CursorPosition *refPos = &mPosition;
    if (hasSelection()) {
        if (backwards && mAnchor < mPosition) refPos = &mAnchor;
        if (!backwards && mPosition < mAnchor) refPos = &mAnchor;
    }
    if (!continueFind) {
        mFindChunk = refPos->chunkNr;
        part = backwards ? 1 : 2;
        continueFind = true;
    }
    if (mFindChunk != refPos->chunkNr) part = 0; // search in complete chunk

    while (continueFind) {
        int startLine = part==2 ? refPos->localLine : 0;
        int lineCount = part==1 ? refPos->localLine+1 : -1;
        Chunk *chunk = getChunk(mFindChunk);
        QString textBlock = lines(chunk, startLine, lineCount);
        QStringRef partRef(&textBlock);
        int ind = backwards ? -1 : 0;
        if (part == 1) {
            textBlock = textBlock.left(textBlock.lastIndexOf(mDelimiter) + mDelimiter.size() + refPos->charNr);
        }
        if (part == 2 && !backwards) {
            ind = refPos->charNr;
        }

        QRegularExpressionMatch match;
        if (backwards) textBlock.lastIndexOf(searchRegex, ind, &match);
        else textBlock.indexOf(searchRegex, ind, &match);
        if (match.hasMatch() || match.hasPartialMatch()) {
            QStringRef ref = textBlock.leftRef(match.capturedStart());
            int line = ref.count("\n");
            int charNr = line ? match.capturedStart() - ref.lastIndexOf("\n") - 1
                              : match.capturedStart();
            setPosAbsolute(chunk, line+startLine, charNr);
            setPosAbsolute(chunk, line+startLine, charNr + match.capturedLength(), QTextCursor::KeepAnchor);
            continueFind = false;
            if (!isCached(chunk)) chunkUncached(chunk);
            return true;
        }
        if (!isCached(chunk)) chunkUncached(chunk);

        if (refPos->chunkNr == mFindChunk && backwards == (part==2)) {
            // reached start-chunk again - nothing found
            continueFind = false;
        } else {
            // currently searching only one chunk before returning to event-loop
            // maybe repeat for several chunks before interrupt
            if (backwards) mFindChunk = mFindChunk==0 ? chunkCount()-1 : mFindChunk-1;
            else mFindChunk = mFindChunk==chunkCount()-1 ? 0 : mFindChunk+1;
            break;
        }
    }
    return false;
}

AbstractTextMapper::Chunk* AbstractTextMapper::chunkForRelativeLine(int lineDelta, int *lineInChunk) const
{
    if (lineInChunk) *lineInChunk = -1;
    int chunkNr = mTopLine.chunkNr;
    int chunkLineDelta = lineDelta + mTopLine.localLine; // add the offset of the top-line

    while (chunkLineDelta < 0) {
        --chunkNr;
        if (chunkNr < 0) return nullptr;
        if (mChunkLineNrs.at(chunkNr).lineCount <= 0)
            if (!setActiveChunk(chunkNr)) return nullptr;
        chunkLineDelta += mChunkLineNrs.at(chunkNr).lineCount;
    }
    while (chunkLineDelta >= mChunkLineNrs.at(chunkNr).lineCount) {
        chunkLineDelta -= mChunkLineNrs.at(chunkNr).lineCount;
        ++chunkNr;
        if (chunkNr >= chunkCount()) return nullptr;
        if (chunkNr >= mChunkLineNrs.size() || mChunkLineNrs.at(chunkNr).lineCount <= 0) {
            if (!setActiveChunk(chunkNr)) return nullptr;
        }
    }
    if (lineInChunk) *lineInChunk = chunkLineDelta;
    return setActiveChunk(chunkNr);
}

bool AbstractTextMapper::isCached(AbstractTextMapper::Chunk *chunk)
{
    return mChunkCache.contains(chunk);
}

void AbstractTextMapper::updateBytesPerLine(const ChunkLines &chunkLines) const
{
    int absKnownLines = chunkLines.lineCount;
    double absKnownLinesSize = chunkLines.linesByteSize;
    if (chunkLines.startLineNr >= 0) {
        absKnownLines += chunkLines.startLineNr;
        absKnownLinesSize += chunkLines.linesStartPos;
    }
    mBytesPerLine = absKnownLinesSize / absKnownLines;
}

QString AbstractTextMapper::selectedText() const
{
    if (mChunkCache.isEmpty()) return QString();
    if (mPosition.chunkNr < 0 || mAnchor.chunkNr < 0 || mPosition == mAnchor) return QString();
    QByteArray all;
    CursorPosition pFrom = qMin(mAnchor, mPosition);
    CursorPosition pTo = qMax(mAnchor, mPosition);
    all.reserve(int(pTo.absLinePos - pFrom.absLinePos) + 2*pTo.charNr - pFrom.charNr);
    Chunk *chunk = setActiveChunk(pFrom.chunkNr);
    while (chunk && chunk->nr <= pTo.chunkNr) {
        int from = chunk->lineBytes.at(0);
        if (chunk->nr == pFrom.chunkNr) {
            QString text = line(chunk, pFrom.localLine).left(pFrom.charNr);
            from = chunk->lineBytes.at(pFrom.localLine)
                    + (mCodec ? mCodec->fromUnicode(text).length() : text.length());
        }
        int to = chunk->lineBytes.at(chunk->lineCount());
        if (chunk->nr == pTo.chunkNr) {
            QString text = line(chunk, pTo.localLine).left(pTo.charNr);
            to = chunk->lineBytes.at(pTo.localLine)
                    + (mCodec ? mCodec->fromUnicode(text).length() : text.length());
        }
        QByteArray raw;
        raw.setRawData(static_cast<const char*>(chunk->bArray)+from, uint(to - from));
        all.append(mCodec ? mCodec->toUnicode(raw) : QString(raw));
        if (chunk->nr == chunkCount()-1) break;

        chunk = setActiveChunk(chunk->nr + 1);
    }
    return mCodec ? mCodec->toUnicode(all) : all;
}

void AbstractTextMapper::copyToClipboard()
{
    QString text = selectedText();
    if (!text.isEmpty()) {
        QClipboard *clip = QGuiApplication::clipboard();
        clip->setText(text);
    }
}

QString AbstractTextMapper::line(AbstractTextMapper::Chunk *chunk, int chunkLineNr) const
{
    QByteArray raw;
    raw.setRawData(static_cast<const char*>(chunk->bArray)+chunk->lineBytes.at(chunkLineNr),
                   uint(mChunkCache.last()->lineBytes.at(chunkLineNr+1)
                        - mChunkCache.last()->lineBytes.at(chunkLineNr) - mDelimiter.size()));
    return mCodec ? mCodec->toUnicode(raw) : QString(raw);
}

int AbstractTextMapper::lastChunkWithLineNr() const
{
    return mLastChunkWithLineNr;
}

void AbstractTextMapper::initTopLine()
{
    if (mChunkCache.size()) {
//        mTopLine.lineCount = mChunkCache.last()->lineCount();
        mTopLine.localLine = 0;
    }
}

void AbstractTextMapper::initChunkCount(int count) const
{
    mChunkLineNrs.clear();
    mChunkLineNrs.squeeze();
    mChunkLineNrs.reserve(count);
    for (int i = mChunkLineNrs.size(); i < count; ++i) {
        // initialize elements
        mChunkLineNrs << ChunkLines(i);
    }
}

int AbstractTextMapper::maxLineWidth() const
{
    return mMaxLineWidth;
}

int AbstractTextMapper::chunkSize() const
{
    return mChunkSize;
}

int AbstractTextMapper::maxChunksInCache() const
{
    return mMaxChunksInCache;
}

QString AbstractTextMapper::lines(Chunk *chunk, int startLine, int &lineCount) const
{
    if (!chunk) return QString();
    QString res;
    if (lineCount <= 0) lineCount = chunk->lineCount()-startLine;
    QByteArray raw;
    raw.setRawData(static_cast<const char*>(chunk->bArray)+chunk->lineBytes.at(startLine),
                   uint(chunk->lineBytes.at(startLine+lineCount) - chunk->lineBytes.at(startLine) - mDelimiter.size()));
    if (!res.isEmpty()) res.append(mDelimiter);
    res.append(mCodec ? mCodec->toUnicode(raw) : QString(raw));
    return res;
}

int AbstractTextMapper::visibleOffset() const
{
    return debugMode() ? mVisibleOffset*2 : mVisibleOffset;
}

int AbstractTextMapper::findChunk(int lineNr)
{
    int clFirst = 0;
    int clLast = mLastChunkWithLineNr;
    if (lineNr < 0 || lineNr >= mChunkLineNrs.at(clLast).startLineNr + mChunkLineNrs.at(clLast).lineCount) {
        return -1;
    }
    while (clFirst < clLast) {
        if (lineNr >= mChunkLineNrs.at(clLast).startLineNr) return clLast;
        if (lineNr < mChunkLineNrs.at(clFirst).startLineNr + mChunkLineNrs.at(clFirst).lineCount) return clFirst;
        int cl = (clFirst + clLast) / 2;
        if (lineNr < mChunkLineNrs.at(cl).startLineNr)
            clLast = cl;
        else
            clFirst = cl;
    }
    return clLast;
}

void AbstractTextMapper::setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode)
{
    int lineInChunk;
    if (debugMode()) localLineNr = localLineNr / 2;
    Chunk * chunk = chunkForRelativeLine(localLineNr, &lineInChunk);
    setPosAbsolute(chunk, lineInChunk, charNr, mode);
}

void AbstractTextMapper::setPosAbsolute(AbstractTextMapper::Chunk *chunk, int lineInChunk, int charNr, QTextCursor::MoveMode mode)
{
    if (!chunk) {
        mPosition = CursorPosition();
        mAnchor = CursorPosition();
        return;
    }
    // requesting the line moved the chunk to the top, so we can use the last chunk here
    mPosition.chunkNr = chunk->nr;
    mPosition.localLine = lineInChunk;
    mPosition.localLinePos = chunk->lineBytes.at(lineInChunk);
    mPosition.lineLen = chunk->lineBytes.at(lineInChunk+1) - mPosition.localLinePos - mDelimiter.size();
    mPosition.absLinePos = chunk->bStart + mPosition.localLinePos;
    mPosition.charNr = charNr;
    if (mode == QTextCursor::MoveAnchor) mAnchor = mPosition;
}

void AbstractTextMapper::emitBlockCountChanged()
{
    // TODO(JM) internal adjustment
    emit blockCountChanged();
}

void AbstractTextMapper::dumpPos()
{
    DEB() << "pos:  chunk " << mPosition.chunkNr << ",  p " << mPosition.absLinePos;
    DEB() << "anc:  chunk " << mAnchor.chunkNr << ",  p " << mAnchor.absLinePos;
}

void AbstractTextMapper::dumpMetrics()
{

}

void AbstractTextMapper::invalidate()
{

}

void AbstractTextMapper::selectAll()
{
    mAnchor.chunkNr = 0;
    mAnchor.localLinePos = 0;
    mAnchor.localLine = 0;
    mAnchor.absLinePos = 0;
    mAnchor.charNr = 0;
    mAnchor.lineLen = 0; // wrong size but irrelevant in this special case
    Chunk *chunk = setActiveChunk(chunkCount()-1);
    if (!chunk || chunk->lineBytes.size() < 2) {
        mPosition = mAnchor;
        return;
    }
    mPosition.chunkNr = chunk->nr;
    mPosition.localLine = chunk->lineBytes.size()-2;
    mPosition.localLinePos = chunk->lineBytes.at(mPosition.localLine);
    mPosition.lineLen = chunk->lineBytes.at(mPosition.localLine+1) - mPosition.localLinePos - mDelimiter.size();
    mPosition.absLinePos = chunk->bStart + mPosition.localLinePos;
    mPosition.charNr = line(chunk, mPosition.localLine).length();
}

QPoint AbstractTextMapper::convertPosLocal(const CursorPosition &pos) const
{
    // no position or it starts before the topLine
    if (pos.chunkNr < 0 || pos.chunkNr < mTopLine.chunkNr ||
            (pos.chunkNr == mTopLine.chunkNr && pos.localLine < mTopLine.localLine))
        return QPoint(0, 0);

    int lineNr = -mTopLine.localLine;
    int chunkNr = mTopLine.chunkNr;
    while (pos.chunkNr >= chunkNr) {
        // count forward
        lineNr += (pos.chunkNr > chunkNr) ? mChunkLineNrs.at(chunkNr).lineCount : pos.localLine;
        if (lineNr >= bufferedLines()) {
            // position is beyond the end of the buffer
            return QPoint(lines(bufferedLines()-1, 1).length(), bufferedLines()-1);
        }
        if (pos.chunkNr == chunkNr) {
            return QPoint(pos.charNr, lineNr);
        }
        ++chunkNr;
    }
    return QPoint(lines(bufferedLines()-1, 1).length(), bufferedLines()-1);
}

int AbstractTextMapper::bufferedLines() const
{
    return visibleLineCount() * 3;
}

void AbstractTextMapper::setDebugMode(bool debug)
{
    mDebugMode = debug;
    updateMaxTop();
}

QPoint AbstractTextMapper::convertPos(const CursorPosition &pos) const
{
    if (pos.chunkNr < 0) return QPoint(0,0);
    int debLine = debugMode() ? 1 : 0;
    const ChunkLines &cl = mChunkLineNrs.at(pos.chunkNr);
    int line = 0;
    if (cl.startLineNr < 0) {
        qint64 absPos = mChunkLineNrs.at(pos.chunkNr).linesStartPos + pos.localLinePos;
        double estimateLine = absPos / mBytesPerLine;
        line = -int(estimateLine);
    } else {
        line = cl.startLineNr + pos.localLine;
    }
    QPoint res;
    res.setY(line + debLine);
    res.setX(pos.charNr);
    return res;
}

QPoint AbstractTextMapper::position(bool local) const
{
    if (mPosition.chunkNr < 0) return QPoint(-1,-1);
    return (local ? convertPosLocal(mPosition) : convertPos(mPosition));
}


QPoint AbstractTextMapper::anchor(bool local) const
{
    if (mPosition.chunkNr < 0 || mAnchor.chunkNr < 0) return QPoint(-1,-1);
    return local ? convertPosLocal(mAnchor) : convertPos(mAnchor);
}


bool AbstractTextMapper::hasSelection() const
{
    return !((mPosition.chunkNr < 0) || (mAnchor.chunkNr < 0) || (mPosition == mAnchor));
}

int AbstractTextMapper::selectionSize() const
{
    if ((mPosition.chunkNr < 0) || (mAnchor.chunkNr < 0) || (mPosition == mAnchor)) return 0;
    qint64 selSize = qAbs( qAbs(mPosition.absLinePos)+mPosition.effectiveCharNr()
                           - qAbs(mAnchor.absLinePos)+mAnchor.effectiveCharNr() );
    if (selSize >= std::numeric_limits<int>::max() / 20) return -1;
    return int(selSize);
}

void AbstractTextMapper::chunkUncached(Chunk *&chunk) const
{
    Q_UNUSED(chunk);
}


void AbstractTextMapper::initDelimiter(Chunk *chunk) const
{
    if (chunk) {
        for (int i = 0; i < chunk->bArray.size(); ++i) {
            if (chunk->bArray.at(i) == '\n' || chunk->bArray.at(i) == '\r') {
                if (chunk->bArray.size() > i+1 && chunk->bArray.at(i) != chunk->bArray.at(i+1)
                        && (chunk->bArray.at(i+1) == '\n' || chunk->bArray.at(i+1) == '\r')) {
                    mDelimiter = chunk->bArray.mid(i, 2);
                } else {
                    mDelimiter = chunk->bArray.mid(i, 1);
                }
                break;
            }
        }
    }
}


} // namespace studio
} // namespace gams
