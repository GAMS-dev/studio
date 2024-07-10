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
#include "textmapper.h"
#include "exception.h"
#include "logger.h"
#include <QFile>
#include <QTextStream>
#include <QGuiApplication>
#include <QClipboard>

namespace gams {
namespace studio {

TextMapper::TextMapper(QObject *parent): QObject(parent)
{
    mCodec = QTextCodec::codecForLocale();
    setMappingSizes();
    mTimer.setInterval(200);
    mTimer.setSingleShot(true);
    connect(&mTimer, &QTimer::timeout, this, &TextMapper::closeFile);
    closeAndReset(true);
}

TextMapper::~TextMapper()
{
    closeFile();
}

QTextCodec *TextMapper::codec() const
{
    return mCodec;
}

void TextMapper::setCodec(QTextCodec *codec)
{
    mCodec = codec;
}

bool TextMapper::openFile(const QString &fileName, bool initAnchor)
{
    if (!fileName.isEmpty()) {
        closeAndReset(initAnchor);
        mFile.setFileName(fileName);
        if (!mFile.open(QFile::ReadOnly)) {
            DEB() << "Could not open file " << fileName;
            return false;
        }
        mSize = mFile.size();
        int chunkCount = int(mFile.size()/mChunkSize)+1;
        mChunkLineNrs.reserve(chunkCount);
        for (int i = mChunkLineNrs.size(); i < chunkCount; ++i) {
            // initialize elements
            mChunkLineNrs << ChunkLines(i);
        }
        Chunk *chunk = getChunk(0);
        if (chunk && chunk->isValid()) {
            if (initAnchor) {
                mTopLine.lineCount = chunk->lineCount();
                mTopLine.localLine = 0;
            }
            updateMaxTop();
            return true;
        }
    }
    return false;
}

bool TextMapper::reopenFile()
{
    QString fileName = mFile.fileName();
    if (!size() && !fileName.isEmpty()) {
        return openFile(fileName, false);
    }
    return size();
}

bool TextMapper::updateMaxTop() // to be updated on change of size or mBufferedLineCount
{
    if (!mFile.isOpen()) return false;
    Chunk *chunk = getChunk(chunkCount()-1);
    if (!chunk || !chunk->isValid()) return false;

    int remainingLines = mBufferedLineCount;
    while (remainingLines > 0) {
        remainingLines -= chunk->lineCount();
        if (remainingLines <= 0) {
            mMaxTopLine.chunkNr = chunk->nr;
            mMaxTopLine.localLine = -remainingLines;
            mMaxTopLine.absStart = chunk->start + chunk->lineBytes.at(-remainingLines);
            mMaxTopLine.lineCount = chunk->lineCount();
            break;
        } else if (chunk->nr == 0) {
            mMaxTopLine.chunkNr = 0;
            mMaxTopLine.absStart = 0LL;
            mMaxTopLine.localLine = 0;
            mMaxTopLine.lineCount = remainingLines-mBufferedLineCount;
            break;
        }
        chunk = getChunk(chunk->nr -1);
    }
    return true;
}

void TextMapper::closeAndReset(bool initAnchor)
{
    mTimer.stop();
    for (Chunk *block: mChunks) {
        mFile.unmap(block->map);
    }
    mChunks.clear();
    closeFile();
    mLastChunkWithLineNr = -1;
    mBytesPerLine = 20.0;
    mChunkLineNrs.clear();
    mChunkLineNrs.squeeze();
    mDelimiter.clear();
    mSize = 0;
    if (initAnchor)
        setPosAbsolute(nullptr, 0, 0);
}


void TextMapper::updateLineOffsets(Chunk *chunk) const
{
    if (!chunk) return;
    for (int i = mChunkLineNrs.size(); i <= chunk->nr; ++i) {
        // fill missing elements
        mChunkLineNrs << ChunkLines(i);
    }
    ChunkLines *cl = &mChunkLineNrs[chunk->nr];
    if (cl->lineCount < 0) { // init ChunkLines on first visit
        cl->lineCount = chunk->lineCount();
        cl->linesStartPos = chunk->start + chunk->lineBytes.first();
        cl->linesByteSize = chunk->lineBytes.last() - chunk->lineBytes.first();
        if (cl->chunkNr == 0) { // only for chunk0
            cl->lineOffset = 0;
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
            cl->lineOffset = prevCl->lineOffset + prevCl->lineCount;
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

TextMapper::Chunk *TextMapper::getChunk(int chunkNr) const
{
    int foundIndex = -1;
    for (int i = mChunks.size()-1; i >= 0; --i) {
        if (mChunks.at(i)->nr == chunkNr) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex < 0) {
        if (mChunks.size() == mMaxChunks) {
            Chunk * delChunk = mChunks.takeFirst();
            mFile.unmap(delChunk->map);
            delete delChunk;
        }
        Chunk *newChunk = loadChunk(chunkNr);
        if (!newChunk) return nullptr;
        mChunks << newChunk;

    } else if (foundIndex < mChunks.size()-1) {
        mChunks.move(foundIndex, mChunks.size()-1);
    }
    return mChunks.last();
}


TextMapper::Chunk* TextMapper::loadChunk(int chunkNr) const
{
    qint64 chunkStart = qint64(chunkNr) * mChunkSize;
    if (chunkStart < 0 || chunkStart >= size()) return nullptr;

    // map file
    qint64 cStart = chunkStart - mMaxLineWidth;
    if (cStart < 0) cStart = 0;                                     // crop at start of file
    qint64 cEnd = chunkStart + mChunkSize;
    if (cEnd > size()) cEnd = size();   // crop at end of file
    uchar* map = nullptr;
    {
        QMutexLocker locker(&mMutex);
        if (mFile.isOpen() || mFile.open(QFile::ReadOnly)) {
            map = mFile.map(cStart, cEnd - cStart);
            mTimer.start();
        } else {
            DEB() << "Could not open file " << mFile.fileName();
        }
    }
    if (!map) return nullptr;

    // mapping succeeded: initialise chunk
    Chunk *res = new Chunk();
    res->nr = chunkNr;
    res->map = map;
    res->start = cStart;
    res->size = int(cEnd - cStart);
    res->bArray.setRawData(reinterpret_cast<char*>(res->map), uint(res->size));

    // if delimiter isn't initialized
    if (mDelimiter.isNull()) initDelimiter(res);

    // create index for linebreaks
    if (!mDelimiter.isNull()) {
        int lines = res->bArray.count(mDelimiter.at(0));
        res->lineBytes.reserve(lines+2);
        res->lineBytes << 0;
        for (int i = 0; i < res->size; ++i) {
            if (res->bArray.at(i) == mDelimiter.at(0)) {
                if (res->start+i+mDelimiter.size() <= chunkStart) {
                    // first [lf] before chunkStart
                    res->lineBytes[0] = (i + mDelimiter.size());
                } else {
                    res->lineBytes << (i + mDelimiter.size());
                }
            }
        }
    }
    if (res->start + res->size == size())
        res->lineBytes << (res->size + mDelimiter.size());

    updateLineOffsets(res);
    return res;
}


bool TextMapper::setMappingSizes(int bufferdLines, int chunkSizeInBytes, int chunkOverlap)
{
    // check constraints
    mVisibleLineCount = qBound(10, bufferdLines/3, 40);
    mBufferedLineCount = qBound(10, bufferdLines, 300);
    mMaxLineWidth = qBound(100, chunkOverlap, 10000);
    mChunkSize = qMax(chunkOverlap *8, chunkSizeInBytes);
    updateMaxTop();
    return (mBufferedLineCount != bufferdLines || mMaxLineWidth != chunkOverlap || mChunkSize != chunkSizeInBytes);
}

void TextMapper::setVisibleLineCount(int visibleLines)
{
    mVisibleLineCount = visibleLines;
}

bool TextMapper::setTopOffset(qint64 byteNr)
{
    byteNr = qBound(0LL, byteNr, size()-1);
    Chunk *chunk = getChunk(int(byteNr / mChunkSize));
    if (!chunk) return false;

    int localByteNr = int(byteNr - chunk->start);
    mTopLine.chunkNr = chunk->nr;
    mTopLine.lineCount = chunk->lineCount();
    mTopLine.localLine = 0;
    for (int i = 0; i < chunk->lineCount(); ++i) {
        if (chunk->lineBytes.at(i) > localByteNr)
            break;
        mTopLine.localLine = i;
    }
    mTopLine.absStart = chunk->start + chunk->lineBytes.at(mTopLine.localLine);
    return true;
}

bool TextMapper::setTopLine(int lineNr)
{
    int chunkNr = findChunk(lineNr);
    if (chunkNr >= 0) {
        Chunk* chunk = getChunk(chunkNr);
        if (!chunk) return false;
        int byte = chunk->lineBytes.at(lineNr - mChunkLineNrs.at(chunkNr).lineOffset);
        setTopOffset(chunk->start + byte);
        return true;
    }
    return false;
}

void TextMapper::closeFile()
{
    QMutexLocker locker(&mMutex);
    mTimer.stop();
    if (mFile.isOpen()) mFile.close();
}

bool TextMapper::setVisibleTopLine(double region)
{
    if (region < 0.0 || region > 1.0) return false;
    qint64 absPos = qint64(region * size());
    Chunk *chunk = getChunk(int(absPos / mChunkSize));
    if (!chunk) return false;

    // get the chunk-local line number for the visibleTopLine
    int localByteNr = int(absPos - chunk->start);
    int line = -1;
    for (int i = 0; i < chunk->lineCount(); ++i) {
        if (chunk->lineBytes.at(i) > localByteNr)
            break;
        line = i;
    }

    // count up to the buffer-topLine
    int bufferTopLine = line - mBufferedLineCount/3;
    while (bufferTopLine < 0 && chunk->nr > 0) {
        chunk = getChunk(chunk->nr-1);
        if (!chunk) return false;
        bufferTopLine += chunk->lineCount();
    }
    if (bufferTopLine < 0) {
        mVisibleTopLine = mBufferedLineCount/3 + bufferTopLine;
        bufferTopLine = 0;
    } else {
        mVisibleTopLine = mBufferedLineCount/3;
    }
    setTopOffset(chunk->start + chunk->lineBytes.at(bufferTopLine));
    return true;
}

bool TextMapper::setVisibleTopLine(int lineNr)
{
    if (lineNr < 0 || lineNr > knownLineNrs()) return false; // out of counted-lines region

    int maxVisLineNr = qAbs(lineCount()) - mVisibleLineCount;
    if (lineNr > maxVisLineNr)
        lineNr = maxVisLineNr;
    int tl = absTopLine();
    if (mVisibleTopLine >= 0 && tl >= 0 && lineNr == tl + mVisibleTopLine)
        return true; // nothing changed

    // calculate the topLine for this visibleTopLine
    tl = qBound(0, lineNr - mBufferedLineCount/3, qAbs(lineCount())-mBufferedLineCount);
    setTopLine(tl);
    mVisibleTopLine = lineNr - absTopLine();
    return true;
}

int TextMapper::moveVisibleTopLine(int lineDelta)
{
    if (!lineDelta) return 0;
    Chunk *chunk = getChunk(mTopLine.chunkNr);
    if (!chunk) return 0;

    if (mTopLine.absStart == 0) {
        // buffer is at absolute top
        int max = (mTopLine.absStart == mMaxTopLine.absStart) ? mBufferedLineCount-mVisibleLineCount-1
                                                              : mBufferedLineCount / 3;
        int localTop = qBound(0, mVisibleTopLine+lineDelta, max);
        lineDelta += mVisibleTopLine-localTop;
        mVisibleTopLine = localTop;
        if (!lineDelta) return mVisibleTopLine;
    }

//      if (mTopLine.absStart == mMaxTopLine.absStart) {
    if (mVisibleTopLine > mBufferedLineCount/3) {
        // buffer is at absolute end
        int localTop = qBound(mBufferedLineCount/3, mVisibleTopLine+lineDelta, mBufferedLineCount-mVisibleLineCount-3);
        lineDelta += mVisibleTopLine-localTop;
        mVisibleTopLine = localTop;
        if (!lineDelta) return mVisibleTopLine;
    }

    while (lineDelta < 0) { // move up
        lineDelta += mTopLine.localLine;
        if (lineDelta < 0) {
            if (chunk->nr == 0) {
                // top chunk reached: move mVisibleTopLine
                mTopLine.absStart = chunk->start;
                mTopLine.localLine = 0;
                mVisibleTopLine = qMax(0, mVisibleTopLine + lineDelta);
                return mVisibleTopLine;
            } else {
                // continue with previous chunk
                chunk = getChunk(chunk->nr - 1);
                if (!chunk) return 0;
                mTopLine.chunkNr = chunk->nr;
                mTopLine.lineCount = chunk->lineCount();
                mTopLine.localLine = mTopLine.lineCount;
                mTopLine.absStart = chunk->start + chunk->lineBytes.at(mTopLine.localLine);
            }
        } else {
            mTopLine.localLine = lineDelta;
            mTopLine.absStart = chunk->start + chunk->lineBytes.at(mTopLine.localLine);
            return mVisibleTopLine;
        }
    }

    while (lineDelta > 0) { // move down
        if (mTopLine.chunkNr == mMaxTopLine.chunkNr && mTopLine.localLine + lineDelta > mMaxTopLine.localLine) {
            // delta runs behind mMaxTopPos
            lineDelta -= mMaxTopLine.localLine - mTopLine.localLine;
            mTopLine = mMaxTopLine;
            // avoid scrolling to the end (keep some lines visible)
            mVisibleTopLine = qMin(lineDelta + mVisibleTopLine, mBufferedLineCount - mVisibleLineCount);
            return mVisibleTopLine;
        }
        lineDelta -= mTopLine.lineCount - mTopLine.localLine; // subtract remaining line-count
        if (lineDelta < 0) {
            // delta is in this chunk
            // TODO(JM) crop visible top line at end on file
            mTopLine.localLine = mTopLine.lineCount + lineDelta;
            mTopLine.absStart = chunk->start + chunk->lineBytes.at(mTopLine.localLine);
            return mVisibleTopLine;
        } else if (chunk->nr < chunkCount()-1) {
            // switch to next chunk
            chunk = getChunk(chunk->nr + 1);
            if (!chunk) return 0;
            mTopLine.chunkNr = chunk->nr;
            mTopLine.lineCount = chunk->lineCount();
            mTopLine.localLine = 0;
            mTopLine.absStart = chunk->start;
        }
    }
    return mVisibleTopLine;
}

void TextMapper::scrollToPosition()
{
    if (mPosition.chunkNr < 0) return;
    double region = double(mPosition.absLinePos + mPosition.effectiveCharNr()) / size();
    setVisibleTopLine(region);
    moveVisibleTopLine(-5);
}

int TextMapper::absTopLine() const
{
    Chunk *chunk = getChunk(mTopLine.chunkNr);
    if (!chunk) return 0;
    const ChunkLines &topCL = mChunkLineNrs.at(chunk->nr);
    if (topCL.lineOffset < 0) {
        qint64 absPos = topCL.linesStartPos + chunk->lineBytes.at(mTopLine.localLine);
        double estimateLine = absPos / mBytesPerLine;
        return -int(estimateLine);
    }
    return topCL.lineOffset + mTopLine.localLine;
}

int TextMapper::lineCount() const
{
    qint64 count = 0;
    if (mLastChunkWithLineNr == chunkCount()-1) {
        count = mChunkLineNrs.last().lineOffset + mChunkLineNrs.last().lineCount;  // counted
    } else {
        count = -qint64(size() / mBytesPerLine) - 1; // estimated
    }
    if (count >= std::numeric_limits<int>::max() || count <= std::numeric_limits<int>::min()) // TEST: || count > 500)
        EXCEPT() << "File too large " << mFile.fileName();
    return int(count);
}

int TextMapper::knownLineNrs() const
{
    if (mLastChunkWithLineNr < 0) return 0;
    ChunkLines cl = mChunkLineNrs.at(mLastChunkWithLineNr);
    if (cl.lineOffset < 0) return 0;
    return cl.lineOffset + cl.lineCount;
}

int TextMapper::topChunk() const
{
    Chunk *chunk = mChunks.isEmpty() ? nullptr : mChunks.last();
    return chunk ? chunk->nr : -1;
}

//QString TextMapper::line(int localLineNr, int *lineInChunk) const
//{
//    int licData;
//    if (!lineInChunk) lineInChunk = &licData;
//    if (mChunks.isEmpty()) return QString();
//    Chunk *chunk1 = chunkForRelativeLine(localLineNr, lineInChunk);
//    // get the text of the line
//    QByteArray raw;
//    raw.setRawData(static_cast<const char*>(chunk1->bArray)+chunk1->lineBytes.at(*lineInChunk),
//                   uint(mChunks.last()->lineBytes.at(*lineInChunk+1)
//                        - mChunks.last()->lineBytes.at(*lineInChunk) - mDelimiter.size()));
//    return mCodec ? mCodec->toUnicode(raw) : QString(raw);
//}

QString TextMapper::lines(int localLineNrFrom, int lineCount) const
{
    QString res;
    if (mChunks.isEmpty()) return QString();
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
        if (chunk->nr == chunkCount()-1) break;
    }
    return res;
    // get the text of the line
}

bool TextMapper::findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind)
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
        Chunk *chunk = loadChunk(mFindChunk);
        QString textBlock = lines(chunk, startLine, lineCount);
        QStringRef partRef(&textBlock);
        int ind = backwards ? -1 : 0;
        if (part == 1) {
            textBlock = textBlock.left(textBlock.lastIndexOf(mDelimiter) + mDelimiter.length() + refPos->charNr);
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
            deleteChunkIfUnused(chunk);
            return true;
        }
        deleteChunkIfUnused(chunk);

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

TextMapper::Chunk* TextMapper::chunkForRelativeLine(int lineDelta, int *lineInChunk) const
{
    if (lineInChunk) *lineInChunk = -1;
    int chunkNr = mTopLine.chunkNr;
    int chunkLineDelta = lineDelta + mTopLine.localLine; // add the offset of the top-line

    while (chunkLineDelta < 0) {
        --chunkNr;
        if (chunkNr < 0) return nullptr;
        if (mChunkLineNrs.at(chunkNr).lineCount <= 0)
            if (!getChunk(chunkNr)) return nullptr;
        chunkLineDelta += mChunkLineNrs.at(chunkNr).lineCount;
    }
    while (chunkLineDelta >= mChunkLineNrs.at(chunkNr).lineCount) {
        chunkLineDelta -= mChunkLineNrs.at(chunkNr).lineCount;
        ++chunkNr;
        if (chunkNr >= chunkCount()) return nullptr;
        if (chunkNr >= mChunkLineNrs.size() || mChunkLineNrs.at(chunkNr).lineCount <= 0) {
            if (!getChunk(chunkNr)) return nullptr;
        }
    }
    if (lineInChunk) *lineInChunk = chunkLineDelta;
    return getChunk(chunkNr);
}

void TextMapper::updateBytesPerLine(const ChunkLines &chunkLines) const
{
    int absKnownLines = chunkLines.lineCount;
    double absKnownLinesSize = chunkLines.linesByteSize;
    if (chunkLines.lineOffset >= 0) {
        absKnownLines += chunkLines.lineOffset;
        absKnownLinesSize += chunkLines.linesStartPos;
    }
    mBytesPerLine = absKnownLinesSize / absKnownLines;
}

//qint64 TextMapper::absPos(int absLineNr, int charNr)
//{
//    int lineInChunk;
//    Chunk *chunk = chunkForLine(absLineNr, &lineInChunk);
//    int rawCharNr = 0;
//    if (charNr > 0) {
//        QString text = line(chunk, lineInChunk).left(charNr);
//        rawCharNr = mCodec ? mCodec->fromUnicode(text).length() : text.length();
//    }
//    qint64 pos = chunk->start + chunk->lineBytes.at(lineInChunk) + rawCharNr;
//    return pos;
//}

//int TextMapper::relPos(int localLineNr, int charNr)
//{
//    int lineInChunk;
//    QString text = line(localLineNr, &lineInChunk).left(charNr);
//    int len = mCodec ? mCodec->fromUnicode(text).length() : text.length();
//    qint64 pos = mChunks.last()->start + mChunks.last()->lineBytes.at(lineInChunk) + len;
//    pos -= mTopLine.absStart;
//    if (qAbs(pos) >= std::numeric_limits<int>::max()) {
//        DEB() << "WARNING: maxint exceeded on calculating TextMapper::relPos("
//              << localLineNr << ", " << charNr << ") = " << pos;
//    }
//    return int(pos);
//}

QString TextMapper::selectedText() const
{
    if (mChunks.isEmpty()) return QString();
    if (mPosition.chunkNr < 0 || mAnchor.chunkNr < 0 || mPosition == mAnchor) return QString();
    QByteArray all;
    CursorPosition pFrom = qMin(mAnchor, mPosition);
    CursorPosition pTo = qMax(mAnchor, mPosition);
    all.reserve(int(pTo.absLinePos - pFrom.absLinePos) + 2*pTo.charNr - pFrom.charNr);
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
        all.append(mCodec ? mCodec->toUnicode(raw) : QString(raw));
        if (chunk->nr == chunkCount()-1) break;

        chunk = getChunk(chunk->nr + 1);
    }
    return mCodec ? mCodec->toUnicode(all) : all;
}

void TextMapper::copyToClipboard()
{
    QString text = selectedText();
    if (!text.isEmpty()) {
        QClipboard *clip = QGuiApplication::clipboard();
        clip->setText(text);
    }
}

TextMapper::Chunk* TextMapper::chunkForLine(int absLine, int *lineInChunk) const
{
    int chunkNr;
    qint64 pos = 0;
    if (absLine > knownLineNrs()) { // estimate line position
        const ChunkLines &cl = mChunkLineNrs.at(mLastChunkWithLineNr);
        qint64 posFrom = cl.linesStartPos + cl.linesByteSize;
        qint64 posTo = size();
        double factor = double(absLine - knownLineNrs()) / (qAbs(lineCount()) - knownLineNrs());
        pos = qint64((posTo - posFrom) * factor) + posFrom;
        chunkNr = qMin(int(pos / mChunkSize), chunkCount()-1);
    } else { // find real line position
        double factor = double(absLine) / qAbs(lineCount());
        pos = qint64(size() * factor);
        chunkNr = qMin(int(pos / mChunkSize), chunkCount()-1);
        while (absLine < mChunkLineNrs.at(chunkNr).lineOffset)
            --chunkNr;
        while (mChunkLineNrs.size() > chunkNr && chunkNr+1 < chunkCount() && mChunkLineNrs.at(chunkNr).lineOffset >= 0
               && absLine >= mChunkLineNrs.at(chunkNr).lineOffset + mChunkLineNrs.at(chunkNr).lineCount)
            ++chunkNr;
    }
    Chunk *res = getChunk(chunkNr);
    if (!res) return nullptr;
    if (lineInChunk) {
        int posInChunk = int(pos - res->start);
        *lineInChunk = 0;
        while (*lineInChunk+1 < res->lineBytes.size() && res->lineBytes.at(*lineInChunk+1) < posInChunk) {
            ++(*lineInChunk);
        }
    }
    return res;
}

QString TextMapper::line(TextMapper::Chunk *chunk, int chunkLineNr) const
{
    QByteArray raw;
    raw.setRawData(static_cast<const char*>(chunk->bArray)+chunk->lineBytes.at(chunkLineNr),
                   uint(mChunks.last()->lineBytes.at(chunkLineNr+1)
                        - mChunks.last()->lineBytes.at(chunkLineNr) - mDelimiter.size()));
    return mCodec ? mCodec->toUnicode(raw) : QString(raw);
}

QString TextMapper::lines(Chunk *chunk, int startLine, int &lineCount)
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

int TextMapper::visibleOffset() const
{
    return mVisibleTopLine;
}

int TextMapper::findChunk(int lineNr)
{
    int clFirst = 0;
    int clLast = mLastChunkWithLineNr;
    if (lineNr < 0 || lineNr >= mChunkLineNrs.at(clLast).lineOffset + mChunkLineNrs.at(clLast).lineCount) {
        return -1;
    }
    while (clFirst < clLast) {
        if (lineNr >= mChunkLineNrs.at(clLast).lineOffset) return clLast;
        if (lineNr < mChunkLineNrs.at(clFirst).lineOffset + mChunkLineNrs.at(clFirst).lineCount) return clFirst;
        int cl = (clFirst + clLast) / 2;
        if (lineNr < mChunkLineNrs.at(cl).lineOffset)
            clLast = cl;
        else
            clFirst = cl;
    }
    return clLast;
}

void TextMapper::setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode)
{
    int lineInChunk;
    Chunk * chunk = chunkForRelativeLine(localLineNr, &lineInChunk);
    setPosAbsolute(chunk, lineInChunk, charNr, mode);
}

void TextMapper::setPosAbsolute(TextMapper::Chunk *chunk, int lineInChunk, int charNr, QTextCursor::MoveMode mode)
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
    mPosition.lineLen = chunk->lineBytes.at(lineInChunk+1) - mPosition.localLinePos - mDelimiter.length();
    mPosition.absLinePos = chunk->start + mPosition.localLinePos;
    mPosition.charNr = charNr;
    if (mode == QTextCursor::MoveAnchor) mAnchor = mPosition;
}

void TextMapper::selectAll()
{
    mAnchor.chunkNr = 0;
    mAnchor.localLinePos = 0;
    mAnchor.localLine = 0;
    mAnchor.absLinePos = 0;
    mAnchor.charNr = 0;
    mAnchor.lineLen = 0; // wrong size but irrelevant in this special case
    Chunk *chunk = getChunk(chunkCount()-1);
    if (!chunk || chunk->lineBytes.size() < 2) {
        mPosition = mAnchor;
        return;
    }
    mPosition.chunkNr = chunk->nr;
    mPosition.localLine = chunk->lineBytes.size()-2;
    mPosition.localLinePos = chunk->lineBytes.at(mPosition.localLine);
    mPosition.lineLen = chunk->lineBytes.at(mPosition.localLine+1) - mPosition.localLinePos - mDelimiter.size();
    mPosition.absLinePos = chunk->start + mPosition.localLinePos;
    mPosition.charNr = line(chunk, mPosition.localLine).length();
}

QPoint TextMapper::convertPosLocal(const CursorPosition &pos) const
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
        if (lineNr >= mBufferedLineCount) {
            // position is beyond the end of the buffer
            return QPoint(lines(mBufferedLineCount-1, 1).length(), mBufferedLineCount-1);
        }
        if (pos.chunkNr == chunkNr) {
            return QPoint(pos.charNr, lineNr);
        }
        ++chunkNr;
    }
    return QPoint(lines(mBufferedLineCount-1, 1).length(), mBufferedLineCount-1);
}

QPoint TextMapper::convertPos(const CursorPosition &pos) const
{
    if (pos.chunkNr < 0) return QPoint(0,0);
    const ChunkLines &cl = mChunkLineNrs.at(pos.chunkNr);
    int line = 0;
    if (cl.lineOffset < 0) {
        qint64 absPos = mChunkLineNrs.at(pos.chunkNr).linesStartPos + pos.localLinePos;
        double estimateLine = absPos / mBytesPerLine;
        line = -int(estimateLine);
    } else {
        line = cl.lineOffset + pos.localLine;
    }
    QPoint res;
    res.setY(line);
    res.setX(pos.charNr);
    return res;
}

QPoint TextMapper::position(bool local) const
{
    if (mPosition.chunkNr < 0) return QPoint(-1,-1);
    return local ? convertPosLocal(mPosition) : convertPos(mPosition);
}


QPoint TextMapper::anchor(bool local) const
{
    if (mPosition.chunkNr < 0 || mAnchor.chunkNr < 0) return QPoint(-1,-1);
    return local ? convertPosLocal(mAnchor) : convertPos(mAnchor);
}


bool TextMapper::hasSelection() const
{
    return !((mPosition.chunkNr < 0) || (mAnchor.chunkNr < 0) || (mPosition == mAnchor));
}

int TextMapper::selectionSize() const
{
    if ((mPosition.chunkNr < 0) || (mAnchor.chunkNr < 0) || (mPosition == mAnchor)) return 0;
    qint64 selSize = qAbs( qAbs(mPosition.absLinePos)+mPosition.effectiveCharNr()
                           - qAbs(mAnchor.absLinePos)+mAnchor.effectiveCharNr() );
    if (selSize >= std::numeric_limits<int>::max() / 20) return -1;
    return int(selSize);
}

void TextMapper::deleteChunkIfUnused(Chunk *&chunk)
{
    if (chunk && !mChunks.contains(chunk)) {
        mFile.unmap(chunk->map);
        delete chunk;
        chunk = nullptr;
    }
}

bool TextMapper::peekChunksForLineNrs(int chunkCount)
{
    int known = mLastChunkWithLineNr;
    Chunk *chunk = nullptr;
    for (int i = 1; i <= chunkCount; ++i) {
        chunk = loadChunk(known + i);
        if (!chunk) break;
        deleteChunkIfUnused(chunk);
    }
    return mLastChunkWithLineNr < this->chunkCount()-1;
}

void TextMapper::initDelimiter(Chunk *chunk) const
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
