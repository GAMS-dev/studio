/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include <QTextDocument>
#include <QMutex>
#include <QTimer>
//#include "syntax.h"

namespace gams {
namespace studio {

///
/// class TextMapper
/// Opens a file into chunks of QByteArrays that are loaded on request. Uses indexes to build the lines for the
/// model on the fly.
///
class TextMapper: public QObject
{
    Q_OBJECT
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
        qint64 linesStartPos = 0;
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
        int localLine = -1;
        int localLinePos = -1;
        int lineLen = -1;
    };

public:
    TextMapper(QObject *parent = nullptr);
    ~TextMapper();

    QTextCodec *codec() const;
    void setCodec(QTextCodec *codec);

    bool openFile(const QString &fileName, bool initAnchor);
    bool reopenFile();
    void closeAndReset(bool initAnchor);
    qint64 size() const { return mSize; }
    QString delimiter() { return mDelimiter; }
    bool peekChunksForLineNrs(int chunkCount);

    bool setMappingSizes(int bufferedLines = 60, int chunkSizeInBytes = 1024*1024, int chunkOverlap = 1024);
    void setVisibleLineCount(int visibleLines);
    int visibleLineCount() { return mVisibleLineCount; }
    bool setVisibleTopLine(double region);
    bool setVisibleTopLine(int lineNr);
    int moveVisibleTopLine(int lineDelta);
    void scrollToPosition();

    int topChunk() const; // TODO (JM) deprecated!

    int visibleOffset() const;
    int absTopLine() const;
    int lineCount() const;
    int knownLineNrs() const;

    QString lines(int localLineNrFrom, int lineCount) const;
    bool findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind);

    QString selectedText() const;
    void copyToClipboard();

    void setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    void selectAll();
    QPoint position(bool local = false) const;
    QPoint anchor(bool local = false) const;
    bool hasSelection() const;
    int selectionSize() const;
    inline int chunkCount() const { return int(qMax(0LL,size()-1)/mChunkSize) + 1; }

private slots:
    void closeFile();

private:
    void initDelimiter(Chunk *chunk) const;
    bool updateMaxTop();
    Chunk *getChunk(int chunkNr) const;
    Chunk *loadChunk(int chunkNr) const;
    void deleteChunkIfUnused(Chunk *&chunk);
    void updateLineOffsets(Chunk *chunk) const;
    Chunk *chunkForRelativeLine(int lineDelta, int *lineInChunk = nullptr) const;
    void updateBytesPerLine(const ChunkLines &chunkLines) const;
    QPoint convertPosLocal(const CursorPosition &pos) const;
    QPoint convertPos(const CursorPosition &pos) const;
    Chunk *chunkForLine(int absLine, int *lineInChunk) const;
    bool setTopLine(int lineNr);
    int findChunk(int lineNr);
    bool setTopOffset(qint64 byteNr);
    QString line(Chunk *chunk, int chunkLineNr) const;
    QString lines(Chunk *chunk, int startLine, int &lineCount);
    void setPosAbsolute(Chunk *chunk, int lineInChunk, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

private:
    mutable QFile mFile;                // mutable to provide consistant logical const-correctness
    mutable QByteArray mDelimiter;
    mutable QVector<Chunk*> mChunks;
    mutable QVector<ChunkLines> mChunkLineNrs;
    mutable int mLastChunkWithLineNr = -1;
    mutable double mBytesPerLine = 20.0;
    mutable QMutex mMutex;
    mutable QTimer mTimer;

    LinePosition mTopLine;
    LinePosition mMaxTopLine;
    int mVisibleTopLine = 0;
    int mBufferedLineCount = 0;
    int mVisibleLineCount = 0;
    qint64 mSize = 0;
    CursorPosition mAnchor;
    CursorPosition mPosition;
    int mFindChunk = 0;

    QTextCodec *mCodec = nullptr;
    int mMaxChunks = 5;
    int mChunkSize = 1024*1024;
    int mMaxLineWidth = 1024;
};

} // namespace studio
} // namespace gams

#endif // TEXTMAPPER_H
