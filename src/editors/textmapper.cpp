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

namespace gams {
namespace studio {

TextMapper::TextMapper()
{
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
        return chunk->isValid();
    }
    return false;
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
    mLastKnownChunk = -1;
    mChunkLineNrs.clear();
    mChunkLineNrs.squeeze();
    mDelimiter.clear();
    mOversizeMapper.setSize(0);
}


TextMapper::Chunk *TextMapper::getChunk(int chunkNr) const
{
    int foundIndex = -1;
    for (int i = 0; i < mChunks.size(); ++i) {
        if (mChunks.at(i)->nr == chunkNr) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex < 0) {
        if (mChunks.size() == 10) {
            Chunk * delChunk = mChunks.takeFirst();
            mFile.unmap(delChunk->map);
            delete delChunk;
        }
        Chunk *newChunk = loadChunk(chunkNr);
        if (!newChunk) return nullptr;
        mChunks << newChunk;

        // update line-offsets as far as possible
        for (int i = mChunkLineNrs.size(); i <= chunkNr; ++i) {
            // fill missing elements
            mChunkLineNrs << ChunkLines(chunkNr, 0);
        }
        ChunkLines *cl = &mChunkLineNrs[chunkNr];
        cl->lineCount = mChunks.last()->lineBytes.size();
        if (chunkNr == 0) {
            cl->lineOffset = 0;
        } else if (mChunkLineNrs.last().lineOffset < 0) {
            // if offset of previous chunk is known, calculate offset for this
            cl->lineOffset = mChunkLineNrs.last().lineOffset + mChunkLineNrs.last().lineCount;
        }
        int cloInd = chunkNr;
        ChunkLines *prevCl = cl;
        while (++cloInd < mChunkLineNrs.size()) {
            cl = &mChunkLineNrs[cloInd];
            if (cl->lineOffset >= 0                    // value already set
                    || prevCl->lineCount < 0           // unknown line count of previous (not read until now)
                    || prevCl->lineOffset < 0) break;  // unknown offset of previous (not all chunks from start were read)
            cl->lineOffset = prevCl->lineOffset + prevCl->lineCount;
        }
        if (mLastKnownChunk < chunkNr)
            mLastKnownChunk = chunkNr;

    } else if (foundIndex < mChunks.size()-1) {
        mChunks.move(foundIndex, mChunks.size()-1);
    }


    return mChunks.last();
}


TextMapper::Chunk* TextMapper::loadChunk(int chunkIndex) const
{
    qint64 chunkStart = chunkIndex * mChunkSize;
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
    res->nr = chunkIndex;
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
            if (res->bArray[i] == mDelimiter.at(0)) {
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
        res->lineBytes << (res->size-1);
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

bool TextMapper::setTopOffset(int byteBlockNr)
{
    qint64 byteNr = qBound(0LL, mOversizeMapper.map(byteBlockNr), mOversizeMapper.size-1);
    Chunk *chunk = getChunk(int(byteNr / mChunkSize));
    if (!chunk) return false;

    int localByteNr = int(byteNr - chunk->start);
    mTopOffset.chunkNr = chunk->nr;
    mTopOffset.lineCount = chunk->lineBytes.size();
    mTopOffset.localLine = 0;
    for (int i = 0; i < chunk->lineBytes.size(); ++i) {
        if (chunk->lineBytes.at(i) <= localByteNr) {
            mTopOffset.localLine = i;
        } else {
            break;
        }
    }
    return true;
}

int TextMapper::topLine() const
{
    return mTopOffset.localLine;
}

int TextMapper::topChunk() const
{
    Chunk *chunk = mChunks.isEmpty() ? nullptr : mChunks.last();
    return chunk ? chunk->nr : -1;
}

QString TextMapper::line1_idx_line2()
{
    QStringList res;
    Chunk *chunk = mChunks.isEmpty() ? nullptr : mChunks.last();
    if (chunk) res << QString::number(chunk->lineBytes[0]+chunk->start)
            << QString::number(qint64(chunk->nr)*mChunkSize)
            << QString::number(chunk->lineBytes[1]+chunk->start);
    return res.join(',');
}

QString TextMapper::line(int localLineNr) const
{
    Chunk *chunk = nullptr;
    int chunkLine = localLineNr + mTopOffset.localLine;
    if (chunkLine < mTopOffset.lineCount)
        chunk = getChunk(mTopOffset.chunkNr);
    else {
        chunk = getChunk(mTopOffset.chunkNr+1);
        if (!chunk) return QString();
        chunkLine -= mTopOffset.lineCount;
        // TODO (JM) maybe we need a look to the next chunk(s)
        if (chunkLine >= chunk->lineBytes.size())
            return QString();
    }
    // get the text of the line
    QTextStream in(mChunks.last()->bArray);
    in.setCodec(mCodec);
    in.seek(mChunks.last()->lineBytes.at(chunkLine));
    return in.read(mChunks.last()->lineBytes.at(chunkLine+1) - mChunks.last()->lineBytes.at(chunkLine) - mDelimiter.size());
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
}


} // namespace studio
} // namespace gams
