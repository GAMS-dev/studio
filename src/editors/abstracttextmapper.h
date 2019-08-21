#ifndef ABSTRACTTEXTMAPPER_H
#define ABSTRACTTEXTMAPPER_H

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

#include <QObject>
#include <QTextCodec>
#include <QVector>
#include <QSet>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
//#include "syntax.h"

namespace gams {
namespace studio {

struct LineFormat {
    LineFormat() {}
    LineFormat(const LineFormat &other) { *this = other; }
    LineFormat(int _start, int _end, QTextCharFormat _format)
        : start(_start), end(_end), format(_format) {}
    LineFormat(int _start, int _end, QTextCharFormat _format, QString tip, QString ref)
        : start(_start), end(_end), format(_format) {
        format.setAnchorHref(ref);
        format.setToolTip(tip);
    }
    LineFormat &operator=(const LineFormat &other) {
        start = other.start; end = other.end; format = other.format;
        extraLstFormat = other.extraLstFormat; extraLstHRef = other.extraLstHRef;
        return *this;
    }
    int start = -1;
    int end = -1;
    QTextCharFormat format;
    const QTextCharFormat *extraLstFormat = nullptr;
    QString extraLstHRef;
};

///
/// class AbstractTextMapper
/// Maps text data into chunks of QByteArrays that are loaded on request. Uses indexes to build the lines for the
/// model on the fly.
///
class AbstractTextMapper: public QObject
{
    Q_OBJECT

private:
    struct LinePosition {
        int chunkNr = 0;
        qint64 absStart = 0;
        int localLine = 0;
//        int lineCount = 0;
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

    struct ChunkLines {
        ChunkLines(int nr = 0, int lines = -1, int lineOffset = -1)
            : chunkNr(nr), lineCount(lines), startLineNr(lineOffset) {}
        inline bool isKnown() const { return lineCount >= 0; }
        inline bool hasLineNrs() const { return lineCount >= 0 && startLineNr >= 0; }
        inline qint64 linesEndPos() const { return linesStartPos + linesByteSize; }
        int chunkNr = 0;
        qint64 linesStartPos = 0;
        int linesByteSize = 0;
        int lineCount = -1;
        int startLineNr = -1;
    };

    struct BufferMeter {
        BufferMeter(int _visibleLines = 0, int _linesTotal = 0): visibleLines(_visibleLines), linesTotal(_linesTotal) {}
        int size() { return qMin(visibleLines*3, linesTotal); }
        int centerTop() { return (size()-visibleLines)/2; }
        int maxTop() { return qMax(size()-visibleLines, 0); }
        int visibleLines;
        int linesTotal;
    };

protected:
    struct Chunk {  // a mapped part of a file OR a standalone part of memory
        int nr = -1;
        qint64 bStart = -1;
        QByteArray bArray;
        QVector<int> lineBytes;
        int size() {
            return lineBytes.size() > 1 ? lineBytes.last() - lineBytes.first() : 0;
        }
        bool isValid() const { return bStart >= 0;}
        int lineCount() const { return lineBytes.size()-1; }
    };

public:
    ~AbstractTextMapper();

    QTextCodec *codec() const;                                  // share FM + MM (FileMapper + MemoryMapper)
    void setCodec(QTextCodec *codec);                           // share FM + MM

    bool isEmpty() const;
    virtual void startRun() = 0;
    virtual void endRun() = 0;
    virtual void createSection();
    virtual qint64 size() const;                                    // share FM + MM
    virtual QByteArray& delimiter() const { return mDelimiter; }        // share FM + MM
    virtual void reset();

    virtual bool setMappingSizes(int visibleLines = 20, int chunkSizeInBytes = 1024*1024, int chunkOverlap = 1024); // share FM + MM
    virtual void setVisibleLineCount(int visibleLines);                 // share FM + MM
    virtual int visibleLineCount() const;                               // share FM + MM
    virtual bool setVisibleTopLine(double region);                      // share FM + MM
    virtual bool setVisibleTopLine(int lineNr);                         // share FM + MM
    virtual int moveVisibleTopLine(int lineDelta);                      // share FM + MM
    virtual void scrollToPosition();                                    // share FM + MM

    int topChunk() const; // TODO(JM) deprecated!

    virtual int visibleOffset() const;                                  // share FM + MM
    virtual int absTopLine() const;                                     // share FM + MM
    virtual int lineCount() const;                                      // share FM + MM    // 2FF
    virtual int knownLineNrs() const;                                   // share FM + MM

    virtual QString lines(int localLineNrFrom, int lineCount) const;    // share FM + MM    //    CC?
    virtual QString lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const;
    virtual bool findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind);   // share FM + MM

    virtual QString selectedText() const;                               // share FM + MM    //    CC?
    virtual void copyToClipboard();                                     // share FM + MM

    virtual void setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor); // share FM + MM
    virtual void selectAll();                                           // share FM + MM
    virtual QPoint position(bool local = false) const;                  // share FM + MM
    virtual QPoint anchor(bool local = false) const;                    // share FM + MM
    virtual bool hasSelection() const;                                  // share FM + MM
    virtual int selectionSize() const;                                  // share FM + MM
    int bufferedLines() const;
    virtual void setDebugMode(bool debug);
    bool debugMode() const { return mDebugMode; }
    void dumpMetrics();
    virtual void invalidate();

signals:
    void blockCountChanged();
    void loadAmountChanged(int knownLineCount);
    void selectionChanged();
    void contentChanged();

protected:
    AbstractTextMapper(QObject *parent = nullptr);

    virtual int chunkCount() const { return int(qMax(0LL,size()-1)/mChunkSize) + 1; }
    virtual QByteArray rawLines(int localLineNrFrom, int lineCount, int chunkBorder, int &borderLine) const;
    Chunk *setActiveChunk(int chunkNr) const;
    Chunk *activeChunk();
    void uncacheChunk(Chunk *&chunk);
    virtual Chunk *getChunk(int chunkNr) const = 0;
    void initDelimiter(Chunk *chunk) const;
    virtual bool updateMaxTop();
    qint64 lastTopAbsPos();
    virtual void chunkUncached(Chunk *&chunk) const;
    void invalidateLineOffsets(Chunk *chunk, bool cutRemain = false) const;
    void updateLineOffsets(Chunk *chunk) const;
    bool isCached(Chunk *chunk);
    int chunkSize() const;
    int maxLineWidth() const;
    void initChunkCount(int count) const;
    int lastChunkWithLineNr() const;
    void initTopLine();
    void setPosAbsolute(Chunk *chunk, int lineInChunk, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor); // CC
    void emitBlockCountChanged();

    void dumpPos();

private:
    QString lines(Chunk *chunk, int startLine, int &lineCount) const;
    QString line(Chunk *chunk, int chunkLineNr) const;
    bool setTopOffset(qint64 absPos);
    void updateBytesPerLine(const ChunkLines &chunkLines) const;
    int maxChunksInCache() const;
    int findChunk(int lineNr);
    Chunk *chunkForRelativeLine(int lineDelta, int *lineInChunk = nullptr) const;
    QPoint convertPos(const CursorPosition &pos) const;
    QPoint convertPosLocal(const CursorPosition &pos) const;

private:
    mutable QByteArray mDelimiter;
    mutable QVector<Chunk*> mChunkCache;
    mutable QVector<ChunkLines> mChunkLineNrs;
    mutable int mLastChunkWithLineNr = -1;
    mutable double mBytesPerLine = 20.0;

    LinePosition mTopLine;
    LinePosition mMaxTopLine;
    int mVisibleOffset = 0;
    int mVisibleLineCount = 0;
    CursorPosition mAnchor;
    CursorPosition mPosition;
    int mFindChunk = 0;

    QTextCodec *mCodec = nullptr;
    int mMaxChunksInCache = 5;
    int mChunkSize = 1024*1024;
    int mMaxLineWidth = 1024;
    bool mDebugMode = false;
};

} // namespace studio
} // namespace gams

#endif // ABSTRACTTEXTMAPPER_H
