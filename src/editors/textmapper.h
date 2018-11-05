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
#ifndef TEXTMAPPER_H
#define TEXTMAPPER_H

#include <QTextCodec>
#include "syntax.h"

namespace gams {
namespace studio {

///
/// class OversizeMapper
/// Maps the size of a file (qint64) to a block size (int) and a remain (int). By setting a size the OversizeMapper
/// calculates the minimum of necessary bytes in a block. If a file size is less than the maximum of an integer the
/// factor will be 1 and the remain always 0.
///
struct OversizeMapper {
    void setSize(const qint64 &_size) {
        size = _size;
        bytesInBlock = int((size - 1) / std::numeric_limits<int>::max()) + 1;
        lastBlockStart = (qint64(bytesInBlock) * std::numeric_limits<int>::max())-bytesInBlock;
    }
    inline qint64 map(const int &block, const int &remain = 0) const {
        return (qint64(block) * bytesInBlock) + remain;
    }
    inline int map(const qint64 &value) const {
        return int(value / bytesInBlock);
    }
    inline int remain(const qint64 &value) const {
        if (value >= lastBlockStart) return int(value - lastBlockStart);
        return int(value % bytesInBlock);
    }
    qint64 size;
    qint64 lastBlockStart;
    int bytesInBlock;
};

///
/// class TextMapper
/// Opens a file into chunks of QByteArrays that are loaded on request. Uses indexes to build the lines for the
/// model on the fly.
///
class TextMapper
{
    struct Chunk {
        int nr = -1;
        qint64 start = -1;
        int size = 0;
        uchar* map = nullptr;
        QByteArray bArray;
        QVector<int> lineBytes;
        qint64 firstLineNr = -1; // TODO: load lineOffset asynchronously
        bool isValid() const { return start >= 0;}
    };
    struct LinePosition {
        int chunkNr = 0;
        int localLine = 0;
        int lineCount = 0;
    };
    struct ChunkLines {
        ChunkLines()
            : chunkNr(0), lineCount(-1), lineOffset(-1) {}
        ChunkLines(int nr, int lines, qint64 lineOffset = -1)
            : chunkNr(nr), lineCount(lines), lineOffset(lineOffset) {}
        int chunkNr = 0;
        int lineCount = -1;
        qint64 lineOffset = -1;
    };

public:
    TextMapper();
    ~TextMapper();

    QTextCodec *codec() const;
    void setCodec(QTextCodec *codec);

    bool openFile(const QString &fileName);
    const OversizeMapper &sizeMapper() const;
    void clear();

    bool setMappingSizes(int visibleLines = 100, int chunkSizeInBytes = 1024*1024, int chunkOverlap = 1024);
    bool setTopOffset(int byteBlockNr);
    int topLine() const;
    int topChunk() const;
    QString line1_idx_line2();

    QString line(int localLineNr) const;

private:
    Chunk *getChunk(int chunkNr) const;
    Chunk *loadChunk(int chunkIndex) const;
    void initDelimiter(Chunk *chunk) const;

private:
    mutable QFile mFile;                // mutable to provide consistant logical const-correctness
    mutable QByteArray mDelimiter;
    mutable QVector<Chunk*> mChunks;
    mutable QVector<ChunkLines> mChunkLineNrs;
    mutable int mLastKnownChunk = -1;

    LinePosition mTopOffset;
    int mVisibleLines;
    int mTrailingLines;
    OversizeMapper mOversizeMapper;

    QTextCodec *mCodec = nullptr;
    int mMaxChunks = 5;
    int mChunkSize = 1024*1024;
    int mMaxLineWidth = 1024;
};

} // namespace studio
} // namespace gams

#endif // TEXTMAPPER_H
