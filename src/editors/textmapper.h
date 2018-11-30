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
        // TODO(JM) check size-overflow AFTER calculation
        return int(((value>size) ? size : value) / bytesInBlock);
    }
    inline int remain(const qint64 &value) const {
        // TODO(JM) check size-overflow AFTER calculation
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
    enum Change { Nothing, Selection=1, Scroll=2, Buffer=4, All=7 };
    typedef QFlags<Change> Changes;

private:

    struct Chunk {  // a mapped part of a file
        int nr = -1;
        qint64 start = -1;
        int size = 0;
        uchar* map = nullptr;
        QByteArray bArray; // TODO(JM) may be redundant
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
        bool operator <(const CursorPosition &other) const {
            return absLinePos + charNr < other.absLinePos + other.charNr; }
        int effectiveCharNr() const { return qMin(charNr, lineLen); }
        int chunkNr = -1;
        qint64 absLinePos = -1;
        int charNr = -1;
        int localLineNr = -1;
        int localLinePos = -1;
        int lineLen = -1;
    };

public:
    TextMapper(QObject *parent = nullptr);
    ~TextMapper();

    QTextCodec *codec() const;
    void setCodec(QTextCodec *codec);

    bool openFile(const QString &fileName);
    qint64 size() const { return mOversizeMapper.size; }
    ProgressAmount peekChunksForLineNrs(int chunkCount);

    bool setMappingSizes(int bufferedLines = 60, int chunkSizeInBytes = 1024*1024, int chunkOverlap = 1024);
    void setVisibleLineCount(int visibleLines);
    bool setVisibleTopLine(double region);
    bool setVisibleTopLine(int lineNr);
    int moveVisibleTopLine(int lineDelta);

    Changes popChanges();
    int visibleOffset() const;
    int absTopLine() const;
    int lineCount() const;
    int knownLineNrs() const;

    QString lines(int localLineNrFrom, int lineCount) const;

    void copyToClipboard();

    void setRelPos(int localLineNr, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    void selectAll();
    QPoint position() const;
    QPoint anchor() const;
    bool hasSelection() const;
    int selectionSize() const;

public:  // test supporting methods (DELETE LATER) <<<<<<<<<<<<<<<<<<<<<
    int topChunk() const;
    void dumpTopChunk(int maxlen);
    const char *rawLine(int localLineNr, int *lineInChunk = nullptr) const;
    int lastChunkWithLines() const { return mLastChunkWithLineNr; }
    int findChunk(int lineNr);
    inline int chunkCount() const { return int(qMax(0LL,size()-1)/mChunkSize) + 1; }
    int fileSizeInByteBlocks(int *remain = nullptr);
    QString line(int localLineNr, int *lineInChunk = nullptr) const;
    int absPos(int absLineNr, int charNr = 0, int *remain = nullptr);
    int relPos(int localLineNr, int charNr = 0);

public:  // to-be-private methods (MOVE TO private) <<<<<<<<<<<<<<<<<<<<
    bool setTopOffset(int byteBlockNr, int remain = 0);
    bool setTopOffset(qint64 byteNr);
    bool setTopLine(int lineNr);
    int moveTopLine(int lineDelta);


private:
    void initDelimiter(Chunk *chunk) const;
    void clear();
    bool updateMaxTop();
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
    LinePosition mMaxTopLine;
    int mVisibleTopLine = 0;
    int mBufferedLineCount = 0;
    int mVisibleLineCount = 0;
    OversizeMapper mOversizeMapper;
    CursorPosition mAnchor;
    CursorPosition mPosition;
    Changes mChanges = Nothing;

    QTextCodec *mCodec = nullptr;
    int mMaxChunks = 5;
    int mChunkSize = 1024*1024;
    int mMaxLineWidth = 1024;
};

} // namespace studio
} // namespace gams

#endif // TEXTMAPPER_H
