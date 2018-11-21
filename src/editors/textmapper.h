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

#include <QObject>
#include <QTextCodec>
#include <QVector>
#include <QFile>
#include <QSet>
#include <QTextCursor>
//#include "syntax.h"

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
    }
    inline qint64 map(const int &block, const int &remain = 0) const {
        return (qint64(block) * bytesInBlock) + remain;
    }
    inline int map(const qint64 &value) const {
        return int(((value>size) ? size : value) / bytesInBlock);
    }
    inline int remain(const qint64 &value) const {
        return int( ((value>size) ? size : value) % bytesInBlock);
    }
    qint64 size;
    int bytesInBlock;
};

///
/// class TextMapper
/// Opens a file into chunks of QByteArrays that are loaded on request. Uses indexes to build the lines for the
/// model on the fly.
///
class TextMapper: public QObject
{
    Q_OBJECT
public:
    struct ProgressAmount { int part = 0; int all = 0; };

private:

    struct Chunk {  // a mapped part of a file
        int nr = -1;
        qint64 start = -1;
        int size = 0;
        uchar* map = nullptr;
        QByteArray bArray;
        QVector<int> lineBytes;
        bool isValid() const { return start >= 0;}
        int lineCount() const { return lineBytes.size()-1; }
    };
    struct LinePosition {
        int chunkNr = 0;
        qint64 absStart = 0;
        int localLine = 0;
        int lineCount = 0;
    };
    struct ChunkLines {
        ChunkLines(int nr = 0, int lines = -1, int lineOffset = -1)
            : chunkNr(nr), lineCount(lines), lineOffset(lineOffset) {}
        inline bool isKnown() const { return lineCount >= 0; }
        inline bool hasLineNrs() const { return lineCount >= 0 && lineOffset >= 0; }
        int chunkNr = 0;
        qint64 linesStart = 0;
        int linesByteSize = 0;
        int lineCount = -1;
        int lineOffset = -1;
    };
    struct CursorPosition {
        bool operator ==(const CursorPosition &other) const {
            return chunkNr == other.chunkNr && absLinePos == other.absLinePos && charNr == other.charNr; }
        int chunkNr = -1;
        qint64 absLinePos = -1;
        int charNr = -1;
        int localLineNr = -1;
        int localLineBytes = -1;
    };

public:
    TextMapper(QObject *parent = nullptr);
    ~TextMapper();

    QTextCodec *codec() const;
    void setCodec(QTextCodec *codec);

    bool openFile(const QString &fileName);
    void cloneWithLineNrs();
    const OversizeMapper &sizeMapper() const;
    void clear();

    bool setMappingSizes(int visibleLines = 100, int chunkSizeInBytes = 1024*1024, int chunkOverlap = 1024);
    bool setTopOffset(int byteBlockNr, int remain = 0);
    bool setTopLine(int lineNr);
    int fileSizeInByteBlocks(int *remain = nullptr);
    int relTopLine() const;
    int absTopLine() const;
    int lineCount() const;
    int knownLineNrs() const;

    QString line(int localLineNr, int *lineInChunk = nullptr) const;
    QString lines(int localLineNrFrom, int lineCount) const;
    int moveTopByLines(int linesDelta);

    int absPos(int absLineNr, int charNr = 0, int *remain = nullptr);
    int relPos(int localLineNr, int charNr = 0);

    void getPosAndAnchor(QPoint &pos, QPoint &anchor) const;
    void setRelPos(int localLineNr, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    void selectAll();
    bool hasSelection();
    int selectionSize();

    ProgressAmount peekChunksForLineNrs(int chunkCount);
    double getBytesPerLine() const;

    // test supporting methods
    int topChunk() const;
    void dumpTopChunk(int maxlen);
    const char *rawLine(int localLineNr, int *lineInChunk = nullptr) const;
    int lastChunkWithLines() const { return mLastChunkWithLineNr; }
    int findChunk(int lineNr);
    inline int chunkCount() const { return int(mOversizeMapper.size/mChunkSize) + 1; }

private:
    void initDelimiter(Chunk *chunk) const;
    Chunk *getChunk(int chunkNr) const;
    Chunk *loadChunk(int chunkNr) const;
    void updateLineOffsets(Chunk *chunk) const;
    Chunk *chunkForRelativeLine(int lineDelta, int *lineInChunk = nullptr) const;
    void updateBytesPerLine(const ChunkLines &chunkLines) const;
    QPoint convertPos(const CursorPosition &pos) const;
    Chunk *chunkForLine(int absLine, int *lineInChunk) const;
    QString line(Chunk *chunk, int chunkLineNr) const;

private:
    mutable QFile mFile;                // mutable to provide consistant logical const-correctness
    mutable QByteArray mDelimiter;
    mutable QVector<Chunk*> mChunks;
    mutable QVector<ChunkLines> mChunkLineNrs;
    mutable int mLastChunkWithLineNr = -1;
    mutable double mBytesPerLine = 20.0;

    LinePosition mTopLine;
    int mVisibleLines;
    int mTrailingLines;
    OversizeMapper mOversizeMapper;
    CursorPosition mAnchor;
    CursorPosition mPosition;

    QTextCodec *mCodec = nullptr;
    int mMaxChunks = 5;
    int mChunkSize = 1024*1024;
    int mMaxLineWidth = 1024;
};

} // namespace studio
} // namespace gams

#endif // TEXTMAPPER_H
