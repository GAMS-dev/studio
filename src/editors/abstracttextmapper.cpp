/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include <QTextStream>
#include <QGuiApplication>
#include <QClipboard>
#include <QtMath>

#include "abstracttextmapper.h"
#include "exception.h"
#include "logger.h"
#include "theme.h"

namespace gams {
namespace studio {

AbstractTextMapper::AbstractTextMapper(QObject *parent): QObject(parent)
{
    setMappingSizes();
    setPosAbsolute(nullptr, 0, 0);
    mCodec = nullptr;
}

AbstractTextMapper::~AbstractTextMapper()
{}

QTextCodec *AbstractTextMapper::codec() const
{
    return mCodec;
}

void AbstractTextMapper::setCodec(QTextCodec *codec)
{
    mCodec = codec;
}

//void AbstractTextMapper::setEncoding(QStringConverter::Encoding encoding)
//{
//    mEncoding = encoding;
//    encode = QStringEncoder(encoding); // QStringEncoder::Flag::Default
//    decode = QStringDecoder(encoding); // QStringEncoder::Flag::Default
//}

bool AbstractTextMapper::isEmpty() const
{
    return size() == 0;
}

bool AbstractTextMapper::updateMaxTop() // to be updated on change of size or mVisibleLineCount
{
    if (isEmpty()) return false;
    Chunk *chunk = getChunk(chunkCount()-1);
    if (!chunk || !chunk->isValid()) return false;
    bool wasMax = (mTopLine == mMaxTopLine);
    int remainingLines = reducedVisibleLineCount();
    while (remainingLines > 0) {
        remainingLines -= chunk->lineCount();
        if (remainingLines <= 0) {
            mMaxTopLine.chunkNr = chunk->nr;
            mMaxTopLine.localLine = -remainingLines;
            mMaxTopLine.absLineStart = chunk->bStart + chunk->lineBytes.at(-remainingLines);
            break;
        } else if (chunk->nr == 0) {
            mMaxTopLine.chunkNr = 0;
            mMaxTopLine.absLineStart = 0LL;
            mMaxTopLine.localLine = 0;
            break;
        }
        chunk = getChunk(chunk->nr -1);
    }
    if (mTopLine > mMaxTopLine || wasMax) mTopLine = mMaxTopLine;
    return true;
}

qint64 AbstractTextMapper::lastTopAbsPos()
{
    if (isEmpty()) return size();
    Chunk *chunk = getChunk(chunkCount()-1);
    if (!chunk || !chunk->isValid()) return size();

    qint64 lastPos = 0LL;
    int remainingLines = reducedVisibleLineCount();
    while (remainingLines > 0) {
        remainingLines -= chunk->lineCount();
        if (remainingLines <= 0) {
            lastPos = chunk->bStart + chunk->lineBytes.at(-remainingLines);
            break;
        } else if (chunk->nr == 0) {
            break;
        }
        chunk = getChunk(chunk->nr -1);
    }
    return lastPos;
}

void AbstractTextMapper::reset()
{
    mLastChunkWithLineNr = -1;
    mBytesPerLine = 20.0;
    mChunkMetrics.squeeze();
    mDelimiter.clear();
    mTopLine = LinePosition();
    mMaxTopLine = LinePosition();
    mPosition = CursorPosition();
    mAnchor = CursorPosition();
    mFindChunk = 0;
    mCursorColumn = 0;
}

qint64 AbstractTextMapper::size() const
{
    return 0;
}

void AbstractTextMapper::invalidateLineOffsets(Chunk *chunk, bool cutRemain) const
{
    // TODO(JM) only called from MemoryMapper -> may be moved there OR joined with updateLineOffsets(..)
    if (!chunk) return;
    ChunkMetrics *cm = chunkMetrics(chunk->nr);
    cm->lineCount = chunk->lineCount();
    cm->linesStartPos = chunk->bStart + chunk->lineBytes.first();
    cm->linesByteSize = chunk->lineBytes.last() - chunk->lineBytes.first();
    if (chunk->nr == 0) {
        cm->startLineNr = 0;
    } else {
        ChunkMetrics *prevCm = chunkMetrics(chunk->nr-1);
        if (prevCm && prevCm->startLineNr >= 0)
            cm->startLineNr = prevCm->startLineNr + prevCm->lineCount;
    }
    if (cutRemain) {
        for (int i = chunk->nr + 1; i < chunkCount(); ++i) {
            ChunkMetrics *cm = chunkMetrics(i);
            if (cm->lineCount < 0) break;
            cm->lineCount = -1;
            cm->startLineNr = -1;
        }
    }
}

void AbstractTextMapper::updateLineOffsets(Chunk *chunk) const
{
    // TODO(JM) only called from FileMapper -> may be moved there OR joined with invalidateLineOffsets(..)
    if (!chunk) return;
    ChunkMetrics *cm = chunkMetrics(chunk->nr);
    if (cm->lineCount < 0) { // init ChunkLines on first visit
        cm->lineCount = chunk->lineCount();
        cm->linesStartPos = chunk->bStart + chunk->lineBytes.first();
        cm->linesByteSize = chunk->lineBytes.last() - chunk->lineBytes.first();
        if (cm->chunkNr == 0) { // only for chunk0
            cm->startLineNr = 0;
            if (mLastChunkWithLineNr < 0) {
                mLastChunkWithLineNr = 0;
                updateBytesPerLine(*cm);
            }
        }
    }
    if (cm->chunkNr > 0) {
        ChunkMetrics *prevCm = chunkMetrics(chunk->nr-1);
        // extend counting as far as known
        while (cm->chunkNr > mLastChunkWithLineNr) {
            // skip if current chunk is unknown or previous chunk has no line-numbers
            if (!cm->isKnown() || !prevCm->hasLineNrs()) break;
            cm->startLineNr = prevCm->startLineNr + prevCm->lineCount;
            if (mLastChunkWithLineNr < cm->chunkNr) {
                mLastChunkWithLineNr = cm->chunkNr;
                updateBytesPerLine(*cm);
            }

            if (cm->chunkNr == mChunkMetrics.size()-1) break;
            prevCm = cm;
            cm = chunkMetrics(cm->chunkNr+1);
        }
    }
}

bool AbstractTextMapper::setMappingSizes(int visibleLines, int chunkSizeInBytes, int chunkOverlap)
{
    // check constraints
    mVisibleLineCount = qMax(1, visibleLines);
    mMaxLineWidth = qBound(100, chunkOverlap, 10000);
    mChunkSize = qMax(chunkOverlap *8, chunkSizeInBytes);
    updateMaxTop();
    return (mVisibleLineCount != visibleLines || mMaxLineWidth != chunkOverlap || mChunkSize != chunkSizeInBytes);
}

void AbstractTextMapper::setVisibleLineCount(int visibleLines)
{
    mVisibleLineCount = qMax(1, visibleLines);
    updateMaxTop();
}

int AbstractTextMapper::visibleLineCount() const
{
    return mVisibleLineCount;
}

int AbstractTextMapper::reducedVisibleLineCount()
{
    return qCeil(visibleLineCount() * 0.95);
}

bool AbstractTextMapper::setTopLine(const Chunk* chunk, int localLine)
{
    if (!chunk) return false;
    // adjust top line
    mTopLine.chunkNr = chunk->nr;
    mTopLine.localLine = localLine;
    mTopLine.absLineStart = chunk->bStart + chunk->lineBytes.at(mTopLine.localLine);
    if (mTopLine > mMaxTopLine) mTopLine = mMaxTopLine;
    return true;
}

bool AbstractTextMapper::setVisibleTopLine(double region)
{
    if (region < 0.0 || region > 1.0) return false;

    // estimate a chunk-index at or beyond the position
    qint64 absPos = qint64(region * lastTopAbsPos());

    int iChunk = qMin(int(absPos / mChunkSize), chunkCount()-1);
    if (iChunk <= lastChunkWithLineNr()) {
        while (iChunk >= 0 && chunkMetrics(iChunk)->linesStartPos > absPos)
            --iChunk;
        while (iChunk < chunkCount()-1 && chunkMetrics(iChunk)->linesEndPos() <= absPos)
            ++iChunk;
    }
    Chunk *chunk = (iChunk < 0) ? nullptr : getChunk(iChunk);
    if (!chunk) return false;

    // get the chunk-local line number for the visibleTopLine
    int localByteNr = int(absPos - chunk->bStart);
    int line = 0;
    for (int i = 0; i < chunk->lineCount(); ++i) {
        if (chunk->lineBytes.at(i) > localByteNr)
            break;
        line = i;
    }
    setTopLine(chunk, line);
    return true;
}

bool AbstractTextMapper::setVisibleTopLine(int lineNr)
{
    if (lineNr < 0 || lineNr > knownLineNrs())
        return false; // out of counted-lines region
    int chunkNr = findChunk(lineNr);
    if (chunkNr < 0) return false;
    Chunk* chunk = getChunk(chunkNr);
    if (!chunk) return false;
    setTopLine(chunk, lineNr - chunkMetrics(chunkNr)->startLineNr);
    return true;
}

int AbstractTextMapper::moveVisibleTopLine(int lineDelta)
{
    if (!lineDelta) return 0;
    if (debugMode())
        lineDelta /= 2;
    Chunk *chunk = getChunk(mTopLine.chunkNr);
    if (!chunk) return 0;

    while (lineDelta < 0) { // move up
        lineDelta += mTopLine.localLine;
        if (lineDelta < 0) {
            if (chunk->nr == 0) { // top chunk reached: move mVisibleTopLine
                mTopLine.absLineStart = chunk->bStart;
                mTopLine.localLine = 0;
                return lineDelta;
            } else { // continue with previous chunk
                chunk = getChunk(chunk->nr - 1);
                if (!chunk) return lineDelta;
                mTopLine.chunkNr = chunk->nr;
                mTopLine.localLine = chunk->lineCount();
                mTopLine.absLineStart = chunk->bStart + chunk->lineBytes.at(mTopLine.localLine);
            }
        } else {
            mTopLine.localLine = lineDelta;
            mTopLine.absLineStart = chunk->bStart + chunk->lineBytes.at(mTopLine.localLine);
            return lineDelta;
        }
    }

    while (lineDelta > 0) { // move down
        if (mTopLine.chunkNr == mMaxTopLine.chunkNr && mTopLine.localLine + lineDelta > mMaxTopLine.localLine) {
            // delta runs behind mMaxTopPos
            lineDelta -= mMaxTopLine.localLine - mTopLine.localLine;
            mTopLine = mMaxTopLine;
            return lineDelta;
        }
        ChunkMetrics *cm = chunkMetrics(mTopLine.chunkNr);
        if (!cm) {
            DEB() << "Error: invalid chunk index " << mTopLine.chunkNr << " [0-" << (chunkCount()-1) << "]";
            break;
        }
        lineDelta -= cm->lineCount - mTopLine.localLine; // subtract remaining line-count

        if (lineDelta < 0) { // delta is in this chunk
            mTopLine.localLine = cm->lineCount + lineDelta;
            mTopLine.absLineStart = chunk->bStart + chunk->lineBytes.at(mTopLine.localLine);
            return lineDelta;
        } else if (chunk->nr < chunkCount()-1) { // switch to next chunk
            chunk = getChunk(chunk->nr + 1);
            if (!chunk) return lineDelta;
            mTopLine.chunkNr = chunk->nr;
            mTopLine.localLine = 0;
            mTopLine.absLineStart = chunk->bStart;
        }
    }
    return lineDelta;
}

void AbstractTextMapper::scrollToPosition()
{
    if (mPosition.chunkNr < 0) return;
    ChunkMetrics *cm = chunkMetrics(mPosition.chunkNr);
    if (!cm) return;
    if (cm->hasLineNrs()) {
        QPoint pos = position(true);
        if (pos.y() < visibleLineCount() / 5 || pos.y() == cursorBeyondEnd || pos.y() > (visibleLineCount() * 4) / 5)
            setVisibleTopLine(cm->startLineNr + mPosition.localLine - visibleLineCount() / 2);
    } else {
        double region = double(mPosition.absLineStart + mPosition.effectiveCharNr()) / size();
        setVisibleTopLine(region);
    }
}

int AbstractTextMapper::visibleTopLine() const
{
    Chunk *chunk = getChunk(mTopLine.chunkNr);
    if (!chunk) return 0;
    ChunkMetrics *cm = chunkMetrics(chunk->nr);
    if (cm->startLineNr < 0) {
        qint64 absPos = cm->linesStartPos + chunk->lineBytes.at(mTopLine.localLine);
        double estimateLine = absPos / mBytesPerLine;
        return -int(estimateLine);
    }
    return cm->startLineNr + mTopLine.localLine;
}

int AbstractTextMapper::lineCount() const
{
    qint64 count = 0;
    if (lastChunkWithLineNr() == chunkCount()-1) {
        ChunkMetrics *cm = chunkMetrics(lastChunkWithLineNr());
        count = cm->startLineNr + cm->lineCount;  // counted
    } else {
        count = -qint64(size() / mBytesPerLine) - 1; // estimated
    }
    if (count >= std::numeric_limits<int>::max() || count <= std::numeric_limits<int>::min())
        EXCEPT() << "Textdata contains too many lines";
    return int(count);
}

int AbstractTextMapper::knownLineNrs() const
{
    if (lastChunkWithLineNr() < 0) return 0;
    ChunkMetrics *cm = chunkMetrics(lastChunkWithLineNr());
    if (cm->startLineNr < 0) return 0;
    return cm->startLineNr + cm->lineCount;
}

QByteArray AbstractTextMapper::rawLines(int localLineNrFrom, int lineCount, int chunkBorder, int &borderLine) const
{
    QByteArray res;
    if (!size()) return res;
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

QString AbstractTextMapper::lines(int localLineNrFrom, int lineCount) const
{
    QString res;
    if (!size()) return QString();
    QPair<int,int> interval = QPair<int,int>(localLineNrFrom, lineCount); // <start, size>
    while (interval.second) {
        QPair<int,int> chunkInterval;
        Chunk *chunk = chunkForRelativeLine(interval.first, &chunkInterval.first);
        if (!chunk) break;
        chunkInterval.second = qMin(interval.second, chunk->lineCount() - chunkInterval.first);
        if (!chunkInterval.second) break;
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
}

QString AbstractTextMapper::lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const
{
    if (!mLineMarkers.isEmpty()) {
        int absTopLine = visibleTopLine();
        if (absTopLine >= 0) {
            QTextCharFormat fmt;
            fmt.setBackground(toColor(Theme::Edit_currentLineBg));
            fmt.setProperty(QTextFormat::FullWidthSelection, true);
            LineFormat markedLine(0, 0, fmt);
            markedLine.lineMarked = true;
            for (int i = absTopLine; i < absTopLine + lineCount; ++i) {
                if (mLineMarkers.contains(i))
                    formats << markedLine;
                else
                    formats << LineFormat();
            }
        }
    }
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
        Chunk *chunk = getChunk(mFindChunk, false);
        if (!chunk) return false;
        QString textBlock = lines(chunk, startLine, lineCount);
        int ind = backwards ? -1 : 0;

        if (part == 1) {
            int delimIndex = textBlock.lastIndexOf(mDelimiter);
            textBlock = textBlock.left( (delimIndex>-1 ? mDelimiter.size()+delimIndex : 0) + refPos->charNr);
        }
        if (part == 2 && !backwards) {
            ind = refPos->charNr;
        }

        QRegularExpressionMatch match;

        if (backwards) textBlock.lastIndexOf(searchRegex, ind, &match);
        else textBlock.indexOf(searchRegex, ind, &match);

        if (match.hasMatch() || match.hasPartialMatch()) {

            QString ref = textBlock.left(match.capturedStart());
            int line = ref.count("\n");
            int charNr = line ? match.capturedStart() - ref.lastIndexOf("\n") - 1
                              : match.capturedStart();

            setPosAbsolute(chunk, line+startLine, charNr);
            setPosAbsolute(chunk, line+startLine, charNr + match.capturedLength(), QTextCursor::KeepAnchor);
            scrollToPosition();

            continueFind = false;
            return true;
        }

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
        ChunkMetrics *cm = chunkMetrics(chunkNr);
        if (cm->lineCount <= 0)
            if (!getChunk(chunkNr)) return nullptr;
        chunkLineDelta += cm->lineCount;
    }
    int cmLineCount = chunkMetrics(chunkNr)->lineCount;
    while (chunkLineDelta >= cmLineCount) {
        chunkLineDelta -= cmLineCount;
        if (chunkNr >= chunkCount()-1) return nullptr;
        cmLineCount = chunkMetrics(++chunkNr)->lineCount;
        if (cmLineCount <= 0) {
            if (!getChunk(chunkNr)) return nullptr;
        }
    }
    if (lineInChunk) *lineInChunk = chunkLineDelta;
    return getChunk(chunkNr);
}

void AbstractTextMapper::updateSearchSelection()
{
    // sort positions
    if (mPosition < mAnchor) {
        mSearchSelectionStart = mPosition;
        mSearchSelectionEnd = mAnchor;
    } else {
        mSearchSelectionStart = mAnchor;
        mSearchSelectionEnd = mPosition;
    }
    mIsSearchSelectionActive = mSearchSelectionStart != mSearchSelectionEnd;
}

QPoint AbstractTextMapper::searchSelectionStart() {
    return convertPos(mSearchSelectionStart);
}

QPoint AbstractTextMapper::searchSelectionEnd() {
    return convertPos(mSearchSelectionEnd);
}

void AbstractTextMapper::setLineMarkers(const QList<int> lines)
{
    mLineMarkers = lines;
}

QList<int> AbstractTextMapper::lineMarkers()
{
    return mLineMarkers;
}

void AbstractTextMapper::clearSearchSelection()
{
    mSearchSelectionStart = CursorPosition();
    mSearchSelectionEnd = CursorPosition();
    setSearchSelectionActive(false);
}

void AbstractTextMapper::setSearchSelectionActive(bool active)
{
    mIsSearchSelectionActive = active;
}

bool AbstractTextMapper::hasSearchSelection()
{
    return mIsSearchSelectionActive;
}

void AbstractTextMapper::updateBytesPerLine(const ChunkMetrics &chunkLines) const
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
    if (!size()) return QString();
    if (!mPosition.isValid() || !mAnchor.isValid() || mPosition == mAnchor) return QString();
    QByteArray all;
    CursorPosition pFrom = qMin(mAnchor, mPosition);
    CursorPosition pTo = qMax(mAnchor, mPosition);
    all.reserve(int(pTo.absLineStart - pFrom.absLineStart) + 2*pTo.charNr - pFrom.charNr);
    Chunk *chunk = getChunk(pFrom.chunkNr);
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
        all.append(mCodec ? mCodec->toUnicode(raw).toUtf8() : raw);
        if (chunk->nr == chunkCount()-1) break;

        chunk = getChunk(chunk->nr + 1);
    }
    return mCodec ? mCodec->toUnicode(all) : all;
}

QString AbstractTextMapper::positionLine() const
{
    Chunk *chunk = getChunk(mPosition.chunkNr);
    if (chunk && mPosition.localLine >= 0) {
        int from = chunk->lineBytes.at(mPosition.localLine);
        int to = chunk->lineBytes.at(mPosition.localLine + 1) - mDelimiter.length();
        QByteArray raw;
        raw.setRawData(static_cast<const char*>(chunk->bArray)+from, uint(to - from));
        return mCodec ? mCodec->toUnicode(raw) : raw;
//        return decode.decode(raw);
    }
    return QString();
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
                   uint(chunk->lineBytes.at(chunkLineNr+1) - chunk->lineBytes.at(chunkLineNr) - mDelimiter.size()));
    return mCodec ? mCodec->toUnicode(raw) : QString(raw);
//    return decode(raw);
}

int AbstractTextMapper::lastChunkWithLineNr() const
{
    return mLastChunkWithLineNr;
}

void AbstractTextMapper::initTopLine()
{
    if (size()) {
        mTopLine = LinePosition();
    }
}

void AbstractTextMapper::initChunkCount(int count) const
{
    mChunkMetrics.clear();
    mChunkMetrics.squeeze();
    mChunkMetrics.reserve(count);
    for (int i = mChunkMetrics.size(); i < count; ++i) {
        // initialize elements
        mChunkMetrics << ChunkMetrics(i);
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
    res.append(decode(raw));
    res.append(mCodec ? mCodec->toUnicode(raw) : QString(raw));
    return res;
}

int AbstractTextMapper::findChunk(int lineNr)
{
    int clFirst = 0;
    int clLast = lastChunkWithLineNr();
    ChunkMetrics *cm = chunkMetrics(clLast);
    if (!cm || lineNr < 0 || lineNr >= cm->startLineNr + cm->lineCount)
        return -1;

    while (clFirst < clLast) {
        if (lineNr >= chunkMetrics(clLast)->startLineNr) return clLast;
        cm = chunkMetrics(clFirst);
        if (lineNr < cm->startLineNr + cm->lineCount) return clFirst;
        int cl = (clFirst + clLast) / 2;
        if (lineNr < chunkMetrics(cl)->startLineNr)
            clLast = cl;
        else
            clFirst = cl;
    }
    return clLast;
}

void AbstractTextMapper::setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode)
{
    int lineInChunk = -1;
    if (debugMode()) localLineNr = localLineNr / 2;
    bool toEnd = (localLineNr > 0) && (charNr == -1);
    if (toEnd) --localLineNr;
    Chunk * chunk = chunkForRelativeLine(localLineNr, &lineInChunk);
    if (!chunk) return;
    if (charNr == -2) charNr = mCursorColumn;
    else if (toEnd) {
        charNr = chunk->lineBytes.at(lineInChunk+1) - chunk->lineBytes.at(lineInChunk) - mDelimiter.size();
    }
    setPosAbsolute(chunk, lineInChunk, charNr, mode);
}

void AbstractTextMapper::setPosToAbsStart(QTextCursor::MoveMode mode)
{
    if (!chunkCount()) return;
    Chunk *chunk = getChunk(0, false);
    setPosAbsolute(chunk, 0, 0, mode);
}

void AbstractTextMapper::setPosToAbsEnd(QTextCursor::MoveMode mode)
{
    if (!chunkCount()) return;
    Chunk *chunk = getChunk(chunkCount()-1, false);
    int lastLine = chunk->lineCount()-1;
    setPosAbsolute(chunk, lastLine, chunk->lineBytes.at(lastLine) - chunk->lineBytes.at(lastLine-1), mode);
}

void AbstractTextMapper::setPosAbsolute(AbstractTextMapper::Chunk *chunk, int lineInChunk, int charNr, QTextCursor::MoveMode mode)
{
    mCursorColumn = charNr;
    if (!chunk) {
        mPosition = CursorPosition();
        mAnchor = CursorPosition();
        return;
    }
    // requesting the line moved the chunk to the top, so we can use the last chunk here
    mPosition.chunkNr = chunk->nr;
    mPosition.localLine = qBound(0, lineInChunk, chunk->lineBytes.size()-2);
    if (mPosition.localLine != lineInChunk)
        DEB() << "Wrong lineInChunk! " << lineInChunk << " [0-" << chunk->lineBytes.size()-2 << "]";
    mPosition.localLineStart = chunk->lineBytes.at(mPosition.localLine);
    mPosition.lineLen = chunk->lineBytes.at(mPosition.localLine+1) - mPosition.localLineStart - mDelimiter.size();
    mPosition.absLineStart = chunk->bStart + mPosition.localLineStart;
    mPosition.charNr = charNr;
    if (mode == QTextCursor::MoveAnchor) {
        mAnchor = mPosition;
    }
}

void AbstractTextMapper::emitBlockCountChanged()
{
    emit blockCountChanged();
}

void AbstractTextMapper::removeChunk(int chunkNr)
{
    Chunk *chunk = getChunk(mMaxTopLine.chunkNr);
    CursorPosition topLine;
    if (chunk && mPosition.isValid() && mPosition.chunkNr >= 0) {
        // generate a CursorPosition for mMaxTopLine
        topLine = mPosition;
        setPosAbsolute(chunk, mMaxTopLine.localLine, 0, QTextCursor::KeepAnchor);
        qSwap(topLine, mPosition);
    }

    if (mFindChunk > chunkNr) --mFindChunk;

    getChunk(chunkNr, true);

    // move stored ChunkLines data
    const ChunkMetrics &clRem = mChunkMetrics.at(chunkNr);
    for (int i = chunkNr+1; i < mChunkMetrics.size(); ++i) {
        ChunkMetrics &cl = mChunkMetrics[i];
        if (cl.linesStartPos) cl.linesStartPos -= clRem.linesByteSize;
        if (cl.startLineNr) cl.startLineNr -= clRem.lineCount;
        --cl.chunkNr;
    }
    mChunkMetrics.removeAt(chunkNr);

    // shift position, anchor and topline if necessary
    QVector<CursorPosition*> cps;
    if (mPosition.isValid()) cps << &mPosition;
    if (mAnchor.isValid()) cps << &mAnchor;
    if (topLine.isValid()) cps << &topLine;
    for (CursorPosition *cp: qAsConst(cps)) {
        if (cp->chunkNr > chunkNr) {
            --cp->chunkNr;
        } else if (cp->chunkNr == 0) {
            Chunk *chunk = getChunk((chunkCount() < chunkNr+1) ? chunkNr+1 : chunkNr-1);
            if (!chunk) {
                *cp = CursorPosition();
            } else {
                cp->chunkNr = chunkNr;
                if (chunk->nr > chunkNr) {
                    cp->localLine = 0;
                    cp->localLineStart = 0;
                    cp->lineLen = chunk->lineBytes.at(1) - cp->localLineStart - mDelimiter.size();
                    cp->absLineStart = chunk->bStart + cp->localLineStart;
                    cp->charNr = 0;
                } else {
                    cp->localLine = chunk->lineCount()-1;
                    cp->localLineStart = chunk->lineBytes.at(cp->localLine);
                    cp->lineLen = chunk->lineBytes.at(cp->localLine+1) - cp->localLineStart - mDelimiter.size();
                    cp->absLineStart = chunk->bStart + cp->localLineStart;
                    cp->charNr = cp->lineLen;
                }
            }
        }
    }

    // FIRST: remove desired chunk
    internalRemoveChunk(chunkNr);

    // THEN: calc new maxTopLine
    updateMaxTop();
    if (mTopLine > mMaxTopLine)
        mTopLine = mMaxTopLine;
}

void AbstractTextMapper::internalRemoveChunk(int chunkNr)
{
    Q_UNUSED(chunkNr)
}

void AbstractTextMapper::dumpPos() const
{
    DEB() << "anc:  chunk " << mAnchor.chunkNr << ",  p " << (mAnchor.absLineStart+mAnchor.effectiveCharNr());
    DEB() << "pos:  chunk " << mPosition.chunkNr << ",  p " << (mPosition.absLineStart+mPosition.effectiveCharNr());
    DEB() << "top:  chunk " << mTopLine.chunkNr << ",  line " << mTopLine.localLine;
    DEB() << "max:  chunk " << mMaxTopLine.chunkNr << ",  line " << mMaxTopLine.localLine;
}

bool AbstractTextMapper::atTail()
{
    return (lineCount() >= 0 && mMaxTopLine == mTopLine);
}

AbstractTextMapper::ChunkMetrics *AbstractTextMapper::chunkMetrics(int chunkNr) const
{
    if (chunkNr >= 0 && chunkNr < chunkCount()) {
        while (mChunkMetrics.size() < chunkCount()) {
            mChunkMetrics << ChunkMetrics();
        }
        return &mChunkMetrics[chunkNr];
    }
    return nullptr;
}

void AbstractTextMapper::selectAll()
{
    mAnchor.chunkNr = 0;
    mAnchor.localLineStart = 0;
    mAnchor.localLine = 0;
    mAnchor.absLineStart = 0;
    mAnchor.charNr = 0;
    mAnchor.lineLen = 0; // wrong size but irrelevant in this special case
    Chunk *chunk = getChunk(chunkCount()-1);
    if (!chunk || chunk->lineBytes.size() < 2) {
        mPosition = mAnchor;
        return;
    }
    mPosition.chunkNr = chunk->nr;
    mPosition.localLine = chunk->lineBytes.size()-2;
    mPosition.localLineStart = chunk->lineBytes.at(mPosition.localLine);
    mPosition.lineLen = chunk->lineBytes.at(mPosition.localLine+1) - mPosition.localLineStart - mDelimiter.size();
    mPosition.absLineStart = chunk->bStart + mPosition.localLineStart;
    mPosition.charNr = line(chunk, mPosition.localLine).length();
}

void AbstractTextMapper::clearSelection()
{
    mAnchor = CursorPosition();
}

QPoint AbstractTextMapper::convertPosLocal(const CursorPosition &pos) const
{
    // no position or it starts before the topLine
    if (pos.chunkNr < 0)
        return QPoint(0, cursorInvalid);
    if (pos.chunkNr < mTopLine.chunkNr ||
            (pos.chunkNr == mTopLine.chunkNr && pos.localLine < mTopLine.localLine)) {
        // position is before the start of the buffer
        return QPoint(0, cursorBeforeStart);
    }

    int lineNr = -mTopLine.localLine;
    int chunkNr = mTopLine.chunkNr;
    while (pos.chunkNr >= chunkNr) {
        // count forward
        ChunkMetrics *cm = chunkMetrics(chunkNr);
        lineNr += (cm && pos.chunkNr > chunkNr) ? cm->lineCount : pos.localLine;
        if (lineNr > visibleLineCount()) {
            // position is beyond the end of the buffer
            return QPoint(lines(visibleLineCount(), 1).length(), cursorBeyondEnd);
        }
        if (pos.chunkNr == chunkNr) {
            return QPoint(qMin(pos.charNr, pos.lineLen), lineNr);
        }
        ++chunkNr;
    }
    return QPoint(lines(visibleLineCount(), 1).length(), cursorBeyondEnd);
//    return QPoint(lines(visibleLineCount()-1, 1).length() - mDelimiter.size(), visibleLineCount()-1);
}


void AbstractTextMapper::setDebugMode(bool debug)
{
    mDebugMode = debug;
    updateMaxTop();
    emitBlockCountChanged();
}

QPoint AbstractTextMapper::convertPos(const CursorPosition &pos) const
{
    if (pos.chunkNr < 0) return QPoint(0, cursorInvalid);
    int debLine = debugMode() ? 1 : 0;
    ChunkMetrics *cm = chunkMetrics(pos.chunkNr);
    int line = 0;
    if (cm->startLineNr < 0) {
        qint64 absPos = cm->linesStartPos + pos.localLineStart;
        double estimateLine = absPos / mBytesPerLine;
        line = -int(estimateLine);
    } else {
        line = cm->startLineNr + pos.localLine;
    }
    QPoint res;
    res.setY(line + debLine);
    res.setX(qMin(pos.charNr, pos.lineLen/*-mDelimiter.size()*/));
    return res;
}

QPoint AbstractTextMapper::position(bool local) const
{
    if (!mPosition.isValid()) return QPoint(-1, cursorInvalid);
    return (local ? convertPosLocal(mPosition) : convertPos(mPosition));
}


QPoint AbstractTextMapper::anchor(bool local) const
{
    if (!mPosition.isValid() || !mAnchor.isValid()) return QPoint(-1, cursorInvalid);
    return local ? convertPosLocal(mAnchor) : convertPos(mAnchor);
}


bool AbstractTextMapper::hasSelection() const
{
    return (mPosition.isValid() && mAnchor.isValid() && !(mPosition == mAnchor));
}

int AbstractTextMapper::selectionSize() const
{
    if (!mPosition.isValid() || !mAnchor.isValid() || mPosition == mAnchor) return 0;
    qint64 selSize = qAbs( qAbs(mPosition.absLineStart)+mPosition.effectiveCharNr()
                           - qAbs(mAnchor.absLineStart)+mAnchor.effectiveCharNr() );
    if (selSize >= std::numeric_limits<int>::max() / 20) return -1;
    return int(selSize);
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
