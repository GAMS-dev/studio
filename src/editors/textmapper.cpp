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
    clear();
}

TextMapper::~TextMapper()
{
    if (mFile.isOpen()) {
        mFile.close();
    }
}

QTextCodec *TextMapper::codec() const
{
    return mCodec;
}

void TextMapper::setCodec(QTextCodec *codec)
{
    mCodec = codec;
//    DEB() << "Codec in TextMapper changed to " << codec->name();
}


bool TextMapper::openFile(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        clear();
        mFile.setFileName(fileName);
        if (!mFile.open(QFile::ReadOnly)) {
            DEB() << "Could not open file " << fileName;
            return false;
        }
        mOversizeMapper.setSize(mFile.size());
        mChunkLineNrs.reserve(int(mFile.size()/mChunkSize)+1);
        Chunk *chunk = getChunk(0);
        if (chunk->isValid()) {
            mTopLine.lineCount = chunk->lineCount();
            mTopLine.localLine = 0;
        }
        return chunk->isValid();
    }
    return false;
}

void TextMapper::cloneWithLineNrs()
{
//    QFile f(mFile.fileName()+"~");
//    if (f.open((QFile::WriteOnly))) {
//        int digits = 1;
//        if (int maxLine = qAbs(lineCount()) > 9) {
//            while (maxLine > 9) {
//                maxLine /= 10;
//                ++digits;
//            }
//        }
//        int absLine = 0;
//        for (int cNr = 0; cNr < chunkCount(); ++cNr) {
//            Chunk *chunk = getChunk(cNr);
//            for (int lin = 0; lin < chunk->lineCount(); ++lin) {
//                QString lineStr = QString::number(lin+absLine);
//                if (lineStr.length() < digits)
//                    lineStr = "\n" + QString('0', digits-lineStr.length()) + lineStr;
//                else lineStr = "\n" + lineStr + " ";
//                f.write(lineStr.toLocal8Bit());
//                QByteArray raw;
//                raw.setRawData(static_cast<const char*>(chunk->bArray)+chunk->lineBytes.at(lin),
//                               uint(mChunks.last()->lineBytes.at(lin+1)
//                                    - mChunks.last()->lineBytes.at(lin) - mDelimiter.size()));
//                f.write(raw);
//            }
//            absLine += chunk->lineCount();
//        }
//        f.close();
//    }
}


void TextMapper::clear()
{
    if (mFile.isOpen()) {
        for (Chunk *block: mChunks) {
            mFile.unmap(block->map);
        }
        mFile.close();
        mChunks.clear();
    }
    mLastChunkWithLineNr = -1;
    mBytesPerLine = 20.0;
    mChunkLineNrs.clear();
    mChunkLineNrs.squeeze();
    mDelimiter.clear();
    mOversizeMapper.setSize(0);
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
        cl->linesStart = chunk->start + chunk->lineBytes.first();
        cl->linesByteSize = chunk->lineBytes.last() - chunk->lineBytes.first();
        if (cl->chunkNr == 0) {
            cl->lineOffset = 0;
            if (mLastChunkWithLineNr < cl->chunkNr) {
                mLastChunkWithLineNr = cl->chunkNr;
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
        if (!newChunk) {
            DEB() << "can't find chunk nr: " << chunkNr << "   (max is " << chunkCount() << ")";
            return nullptr;
        }
        mChunks << newChunk;

    } else if (foundIndex < mChunks.size()-1) {
        mChunks.move(foundIndex, mChunks.size()-1);
    }
    return mChunks.last();
}


TextMapper::Chunk* TextMapper::loadChunk(int chunkNr) const
{
    qint64 chunkStart = chunkNr * mChunkSize;
    if (chunkStart < 0 || chunkStart >= mOversizeMapper.size) return nullptr;

    // map file
    qint64 cStart = chunkStart - mMaxLineWidth;
    if (cStart < 0) cStart = 0;                                     // crop at start of file
    qint64 cEnd = chunkStart + mChunkSize;
    if (cEnd > mOversizeMapper.size) cEnd = mOversizeMapper.size;   // crop at end of file
    uchar* map = mFile.map(cStart, cEnd - cStart);
    if (!map) return nullptr;

    // mapping succeeded initialise chunk
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
    if (res->start + res->size == mOversizeMapper.size)
        res->lineBytes << (res->size + mDelimiter.size());

    updateLineOffsets(res);
    return res;
}


bool TextMapper::setMappingSizes(int visibleLines, int chunkSizeInBytes, int chunkOverlap)
{
    // check constraints
    mVisibleLines = qBound(10, visibleLines, 200);
    mMaxLineWidth = qBound(100, chunkOverlap, 10000);
    mChunkSize = qMax(chunkOverlap *8, chunkSizeInBytes);
    return (mVisibleLines != visibleLines || mMaxLineWidth != chunkOverlap || mChunkSize != chunkSizeInBytes);
}

bool TextMapper::setTopOffset(int byteBlockNr, int remain)
{
    qint64 byteNr = qBound(0LL, mOversizeMapper.map(byteBlockNr, remain), mOversizeMapper.size-1);
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
        int byte = chunk->lineBytes.at(lineNr - mChunkLineNrs.at(chunkNr).lineOffset);
        setTopOffset(mOversizeMapper.map(chunk->start + byte));
        return true;
    }
    return false;
}

int TextMapper::fileSizeInByteBlocks(int *remain)
{
    if (remain) *remain = mOversizeMapper.remain(mOversizeMapper.size);
    return mOversizeMapper.map(mOversizeMapper.size);
}

int TextMapper::relTopLine() const
{
    return mTopLine.localLine;
}

int TextMapper::absTopLine() const
{
    Chunk *chunk = getChunk(mTopLine.chunkNr);
    if (!chunk) return -1;
    const ChunkLines &topCL = mChunkLineNrs.at(chunk->nr);
    if (topCL.lineOffset < 0) {
        qint64 absPos = topCL.linesStart + chunk->lineBytes.at(mTopLine.localLine);
        double estimateLine = absPos / mBytesPerLine;
        return -int(estimateLine);
    }
    return topCL.lineOffset + mTopLine.localLine;
}

int TextMapper::lineCount() const
{
    if (mLastChunkWithLineNr == chunkCount()-1) {
        return mChunkLineNrs.last().lineOffset + mChunkLineNrs.last().lineCount;  // counted
    }
    return -int(mOversizeMapper.size / mBytesPerLine) - 1; // estimated
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

void TextMapper::dumpTopChunk(int maxlen)
{
    Chunk *res = mChunks.last();
    DEB() << "Chunk " << res->nr << ": '"
          << res->bArray.left((maxlen-5)/2) << "' ... '" << res->bArray.right((maxlen-5)/2) << "'";
}

const char* TextMapper::rawLine(int localLineNr, int *lineInChunk) const
{
    int licData;
    if (!lineInChunk) lineInChunk = &licData;
    if (mChunks.isEmpty()) return QByteArray();
    Chunk *chunk1 = chunkForRelativeLine(localLineNr, lineInChunk);
    // get the text of the line
    QByteArray raw;
    raw.setRawData(static_cast<const char*>(chunk1->bArray)+chunk1->lineBytes.at(*lineInChunk),
                   uint(mChunks.last()->lineBytes.at(*lineInChunk+1)
                        - mChunks.last()->lineBytes.at(*lineInChunk) - mDelimiter.size()));
    return raw;
}

QString TextMapper::line(int localLineNr, int *lineInChunk) const
{
    int licData;
    if (!lineInChunk) lineInChunk = &licData;
    if (mChunks.isEmpty()) return QString();
    Chunk *chunk1 = chunkForRelativeLine(localLineNr, lineInChunk);
    // get the text of the line
    QByteArray raw;
    raw.setRawData(static_cast<const char*>(chunk1->bArray)+chunk1->lineBytes.at(*lineInChunk),
                   uint(mChunks.last()->lineBytes.at(*lineInChunk+1)
                        - mChunks.last()->lineBytes.at(*lineInChunk) - mDelimiter.size()));
    return mCodec ? mCodec->toUnicode(raw) : QString(raw);
}

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

TextMapper::Chunk* TextMapper::chunkForRelativeLine(int lineDelta, int *lineInChunk) const
{
    if (lineInChunk) *lineInChunk = -1;
    int chunkNr = mTopLine.chunkNr;
    int chunkLineDelta = lineDelta + mTopLine.localLine; // add the offset of the top-line

    // TODO(JM) FIRST: check, if the next chunk has been initialized


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
        if (chunkNr >= mChunkLineNrs.size() || mChunkLineNrs.at(chunkNr).lineCount == 0) {
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
        absKnownLinesSize += chunkLines.linesStart;
    }
    mBytesPerLine = absKnownLinesSize / absKnownLines;
}

int TextMapper::absPos(int absLineNr, int charNr, int *remain)
{
    int lineInChunk;
    Chunk *chunk = chunkForLine(absLineNr, &lineInChunk);
    int rawCharNr = 0;
    if (charNr > 0) {
        QString text = line(chunk, lineInChunk).left(charNr);
        rawCharNr = mCodec ? mCodec->fromUnicode(text).length() : text.length();
    }
    qint64 pos = chunk->start + chunk->lineBytes.at(lineInChunk) + rawCharNr;
    if (remain) *remain = mOversizeMapper.remain(pos);
    return mOversizeMapper.map(pos);
}

int TextMapper::relPos(int localLineNr, int charNr)
{
    int lineInChunk;
    QString text = line(localLineNr, &lineInChunk).left(charNr);
    int len = mCodec ? mCodec->fromUnicode(text).length() : text.length();
    qint64 pos = mChunks.last()->start + mChunks.last()->lineBytes.at(lineInChunk) + len;
    pos -= mTopLine.absStart;
    if (qAbs(pos) >= std::numeric_limits<int>::max()) {
        DEB() << "WARNING: maxint exceeded on calculating TextMapper::relPos("
              << localLineNr << ", " << charNr << ") = " << pos;
    }
    return int(pos);
}

void TextMapper::copyToClipboard()
{
    if (mChunks.isEmpty()) return;
    if (mPosition.chunkNr < 0 || mAnchor.chunkNr < 0 || mPosition == mAnchor) return;
    QByteArray all;
    CursorPosition pFrom = qMin(mAnchor, mPosition);
    CursorPosition pTo = qMax(mAnchor, mPosition);
    all.reserve(int(pTo.absLinePos - pFrom.absLinePos) + 2*pTo.charNr - pFrom.charNr);
    Chunk *chunk = getChunk(pFrom.chunkNr);
    while (chunk && chunk->nr <= pTo.chunkNr) {
        int from = chunk->lineBytes.at(0);
        if (chunk->nr == pFrom.chunkNr) {
            QString text = line(chunk, pFrom.localLineNr).left(pFrom.charNr);
            from = chunk->lineBytes.at(pFrom.localLineNr)
                    + (mCodec ? mCodec->fromUnicode(text).length() : text.length());
        }
        int to = chunk->lineBytes.at(chunk->lineCount());
        if (chunk->nr == pTo.chunkNr) {
            QString text = line(chunk, pTo.localLineNr).left(pTo.charNr);
            to = chunk->lineBytes.at(pTo.localLineNr)
                    + (mCodec ? mCodec->fromUnicode(text).length() : text.length());
        }
        QByteArray raw;
        raw.setRawData(static_cast<const char*>(chunk->bArray)+from, uint(to - from));
        all.append(mCodec ? mCodec->toUnicode(raw) : QString(raw));
        if (chunk->nr == chunkCount()-1) break;

        chunk = getChunk(chunk->nr + 1);
    }

    QClipboard *clip = QGuiApplication::clipboard();
    clip->setText(mCodec ? mCodec->toUnicode(all) : all);
}

QPoint TextMapper::convertPos(const CursorPosition &pos) const
{
    if (pos.chunkNr < 0) return QPoint(-1,0);
    const ChunkLines &cl = mChunkLineNrs.at(pos.chunkNr);
    int line = 0;
    if (cl.lineOffset < 0) {
        qint64 absPos = mChunkLineNrs.at(pos.chunkNr).linesStart + pos.localLineBytes;
        double estimateLine = absPos / mBytesPerLine;
        line = -int(estimateLine);
    } else {
        line = cl.lineOffset + pos.localLineNr;
        DEB() << " pos.localLineNr: " << pos.localLineNr << " cl.lineOffset;: " << cl.lineOffset;
    }
    QPoint res;
    res.setY(line);
    res.setX(pos.charNr);
    return res;
}

TextMapper::Chunk* TextMapper::chunkForLine(int absLine, int *lineInChunk) const
{
    int chunkNr;
    qint64 pos = 0;
    if (absLine > knownLineNrs()) { // estimate line position
        const ChunkLines &cl = mChunkLineNrs.at(mLastChunkWithLineNr);
        qint64 posFrom = cl.linesStart + cl.linesByteSize;
        qint64 posTo = mOversizeMapper.size;
        double factor = double(absLine - knownLineNrs()) / (qAbs(lineCount()) - knownLineNrs());
        pos = qint64((posTo - posFrom) * factor) + posFrom;
        chunkNr = int(pos / mChunkSize);
    } else { // find real line position
        double factor = double(absLine) / qAbs(lineCount());
        pos = qint64(mOversizeMapper.size * factor);
        chunkNr = int(pos / mChunkSize);
        while (absLine < mChunkLineNrs.at(chunkNr).lineOffset)
            --chunkNr;
        while (mChunkLineNrs.size() > chunkNr && mChunkLineNrs.at(chunkNr).lineOffset >= 0
               && absLine >= mChunkLineNrs.at(chunkNr).lineOffset + mChunkLineNrs.at(chunkNr).lineCount)
            ++chunkNr;
    }
    Chunk *res = getChunk(chunkNr);
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

double TextMapper::getBytesPerLine() const
{
    return mBytesPerLine;
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

void TextMapper::getPosAndAnchor(QPoint &pos, QPoint &anchor) const
{
    if (mPosition.chunkNr < 0) {
        pos = QPoint(-1,-1);
        anchor = QPoint(-1,-1);
    }
    pos = convertPos(mPosition);
    if (mAnchor.chunkNr < 0)
        anchor = pos;
    else
        anchor = convertPos(mAnchor);
}

void TextMapper::setRelPos(int localLineNr, int charNr, QTextCursor::MoveMode mode)
{
//    TRACE();
//    DEB() << "SELECT - line: " << localLineNr << " char: " << charNr << "  mode: " << mode;
    int lineInChunk;
    Chunk * chunk = chunkForRelativeLine(localLineNr, &lineInChunk);
    if (!chunk) {
        mPosition.chunkNr = -1;
        mAnchor.chunkNr = -1;
        return;
    }
//    QString text = line(localLineNr, &lineInChunk).left(charNr);
//    int len = mCodec ? mCodec->fromUnicode(text).length() : text.length();
    // requesting the line moves the chunk to the top, so we can use the last chunk here
    mPosition.chunkNr = mChunks.last()->nr;
    mPosition.localLineNr = lineInChunk;
    mPosition.localLineBytes = mChunks.last()->lineBytes.at(lineInChunk);
    mPosition.lineLen = mChunks.last()->lineBytes.at(lineInChunk+1) - mPosition.localLineBytes - mDelimiter.length();
    mPosition.absLinePos = mChunks.last()->start + mPosition.localLineBytes;
    mPosition.charNr = charNr;
    if (mode == QTextCursor::MoveAnchor) mAnchor = mPosition;
}

void TextMapper::selectAll()
{
    mAnchor.chunkNr = 0;
    mAnchor.localLineBytes = 0;
    mAnchor.localLineNr = 0;
    mAnchor.absLinePos = 0;
    mAnchor.charNr = 0;
    mAnchor.lineLen = 0; // wrong size but irrelevant in this special case
    Chunk *chunk = getChunk(chunkCount()-1);
    if (!chunk || chunk->lineBytes.size() < 2) {
        mPosition = mAnchor;
        return;
    }
    mPosition.chunkNr = chunk->nr;
    mPosition.localLineNr = chunk->lineBytes.size()-2;
    mPosition.localLineBytes = chunk->lineBytes.at(mPosition.localLineNr);
    mPosition.lineLen = chunk->lineBytes.at(mPosition.localLineNr+1) - mPosition.localLineBytes - mDelimiter.size();
    mPosition.absLinePos = chunk->start + mPosition.localLineBytes;
    mPosition.charNr = line(chunk, mPosition.localLineNr).length();
}

bool TextMapper::hasSelection()
{
    return !((mPosition.chunkNr < 0) || (mAnchor.chunkNr < 0) || (mPosition == mAnchor));
}

int TextMapper::selectionSize()
{
    if ((mPosition.chunkNr < 0) || (mAnchor.chunkNr < 0) || (mPosition == mAnchor)) return 0;
    qint64 selSize = qAbs( qAbs(mPosition.absLinePos)+mPosition.effectiveCharNr()
                           - qAbs(mAnchor.absLinePos)+mAnchor.effectiveCharNr() );
    if (selSize >= std::numeric_limits<int>::max()/2) return -1;
    return int(selSize);
}

TextMapper::ProgressAmount TextMapper::peekChunksForLineNrs(int chunkCount)
{
    ProgressAmount res;
    res.all = mOversizeMapper.map(mOversizeMapper.size);
    int known = mLastChunkWithLineNr;
    Chunk *chunk = nullptr;
    for (int i = 1; i <= chunkCount; ++i) {
        chunk = loadChunk(known + i);
        if (!chunk) break;
    }
    if (!chunk) res.part = res.all;
    else res.part = mOversizeMapper.map(chunk->start+chunk->size);
    return res;
}

int TextMapper::moveTopByLines(int linesDelta)
{
    if (!linesDelta) return 0;
    int add = linesDelta < 0 ? -1 : 1;
    int remain = 0;
    mTopLine.localLine += linesDelta;

    while (mTopLine.localLine != qBound(0, mTopLine.localLine, mTopLine.lineCount-1)) {
        Chunk *chunk = getChunk(mTopLine.chunkNr+add);
        if (chunk) {
            if (add > 0) mTopLine.localLine -= mTopLine.lineCount;
            mTopLine.chunkNr = chunk->nr;
            mTopLine.lineCount = chunk->lineCount();
            if (add < 0) mTopLine.localLine += mTopLine.lineCount;
        } else {
            remain = mTopLine.localLine;
            if (add > 0) remain -= mTopLine.lineCount;
            mTopLine.localLine = (add < 0) ? 0 : mTopLine.lineCount-1;
        }
    }
    int diffLine = linesDelta - mTopLine.localLine;
    mTopLine.localLine = qBound(0, mTopLine.localLine, mTopLine.lineCount-1);
    return diffLine;
}

const OversizeMapper &TextMapper::sizeMapper() const
{
    return mOversizeMapper;
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
    DEB() << "delimiter size: " << mDelimiter.length();
}


} // namespace studio
} // namespace gams
