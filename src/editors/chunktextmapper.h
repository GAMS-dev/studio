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
#ifndef CHUNKTEXTMAPPER_H
#define CHUNKTEXTMAPPER_H

#include <QObject>
#include <QVector>
#include <QSet>
//#include <QStringConverter>
#include <QTextCursor>
#include <QTextDocument>
#include <QPoint>

#include "abstracttextmapper.h"
#include "search/result.h"

namespace gams {
namespace studio {

class FileMeta;
///
/// class AbstractTextMapper
/// Maps text data into chunks of QByteArrays that are loaded on request. Uses indexes to build the lines for the
/// model on the fly.
///
class ChunkTextMapper: public AbstractTextMapper
{
    Q_OBJECT
private:
    /// class LinePosition
    /// Data of a line start
    ///
    struct LinePosition {
        bool operator == (const LinePosition &other) const {
            return chunkNr == other.chunkNr && localLine == other.localLine;
        }
        bool operator > (const LinePosition &other) const {
            return chunkNr > other.chunkNr || (chunkNr == other.chunkNr && localLine > other.localLine);
        }
        int chunkNr = 0;
        qint64 absLineStart = 0;
        int localLine = 0;
    };

    /// class CursorPosition
    /// Caches necessary data of a position in the text to avoid reloading the chunk
    ///
    struct CursorPosition {
        bool operator ==(const CursorPosition &other) const {
            return chunkNr == other.chunkNr && absLineStart == other.absLineStart && charNr == other.charNr; }
        bool operator !=(const CursorPosition &other) const {
            return chunkNr != other.chunkNr || absLineStart != other.absLineStart || charNr != other.charNr; }
        bool operator <(const CursorPosition &other) const {
            return absLineStart + charNr < other.absLineStart + other.charNr; }
        bool operator >(const CursorPosition &other) const {
            return absLineStart + charNr > other.absLineStart + other.charNr; }
        int effectiveCharNr() const { return qMin(charNr, lineLen); }
        bool isValid() const { return chunkNr >= 0; }
        int chunkNr = -1;
        qint64 absLineStart = -1;
        int charNr = -1;
        int localLine = -1;
        int localLineStart = -1;
        int lineLen = -1;
    };

protected:
    /// class ChunkMetrics
    /// Stores necessary size and line count of a chunk to avoid reloading the chunk
    ///
    struct ChunkMetrics {
        ChunkMetrics(int nr = 0, int lines = -1, int lineOffset = -1)
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

protected:
    /// class Chunk
    /// Stores content, global position, and all starts of lines of a part of the text
    ///
    struct Chunk {  // a mapped part of a file OR a standalone part of memory
        int nr = -1;
        qint64 bStart = -1;
        QByteArray bArray;
        QVector<int> lineBytes;
        QPoint markedRegion;
        int size() {
            return lineBytes.size() > 1 ? lineBytes.last() - lineBytes.first() : 0;
        }
        bool isValid() const { return bStart >= 0;}
        int lineCount() const { return lineBytes.size()-1; }
    };

public:
    qint64 size() const override;

    void setVisibleLineCount(int visibleLines) override;
    bool setVisibleTopLine(double region) override;
    bool setVisibleTopLine(int lineNr) override;
    int moveVisibleTopLine(int lineDelta) override;
    int visibleTopLine() const override;
    void scrollToPosition() override;

    int lineCount() const override;
    int knownLineNrs() const override;

    QString lines(int localLineNrFrom, int lineCount) const override;
    QString lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const override;
    bool findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind) override;

    QString selectedText() const override;
    QString positionLine() const override;

    void setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) override;
    void setPosToAbsStart(QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) override;
    void setPosToAbsEnd(QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) override;
    void selectAll() override;
    void clearSelection() override;
    QPoint position(bool local = false) const override;
    QPoint anchor(bool local = false) const override;
    bool hasSelection() const override;
    int selectionSize() const override;
    bool atTail() override;
    void updateSearchSelection() override;
    void clearSearchSelection() override;
    QPoint searchSelectionStart() override;
    QPoint searchSelectionEnd() override;

    void dumpPos() const override;
    bool setMappingSizes(int visibleLines = 20, int chunkSizeInBytes = 1024*1024, int chunkOverlap = 1024);

public slots:
    void reset() override;

protected:
    ChunkTextMapper(QObject *parent = nullptr);

    virtual int chunkCount() const = 0;
    virtual ChunkMetrics* chunkMetrics(int chunkNr) const;
    QByteArray rawLines(int localLineNrFrom, int lineCount, int chunkBorder, int &borderLine) const;
    virtual Chunk *getChunk(int chunkNr, bool cache = true) const = 0;
    void initDelimiter(Chunk *chunk) const;
    bool updateMaxTop() override;
    qint64 lastTopAbsPos();
    void invalidateLineOffsets(Chunk *chunk, bool cutRemain = false) const;
    void updateLineOffsets(Chunk *chunk) const;
    int chunkSize() const;
    int maxLineWidth() const;
    void initChunkCount(qsizetype count) const;
    virtual int lastChunkWithLineNr() const;
    void initTopLine();
    void setPosAbsolute(Chunk *chunk, int lineInChunk, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor); // CC
    void removeChunk(int chunkNr);
    virtual void internalRemoveChunk(int chunkNr);
    LinePosition topLine() const { return mTopLine; }
    Chunk *chunkForRelativeLine(int lineDelta, int *lineInChunk = nullptr) const;

private:
    QString lines(Chunk *chunk, int startLine, int &lineCount) const;
    QString line(Chunk *chunk, int chunkLineNr) const;
    bool setTopLine(const Chunk *chunk, int localLine);
    void updateBytesPerLine(const ChunkMetrics &chunkMetrics) const;
    int maxChunksInCache() const;
    int findChunk(int lineNr);
    QPoint convertPos(const CursorPosition &pos) const;
    QPoint convertPosLocal(const CursorPosition &pos) const;

private:
    mutable QByteArray mDelimiter;
    mutable QVector<ChunkMetrics> mChunkMetrics;
    mutable int mLastChunkWithLineNr = -1;
    mutable double mBytesPerLine = 20.0;

    LinePosition mTopLine;
    LinePosition mMaxTopLine;
    CursorPosition mAnchor;
    CursorPosition mPosition;
    CursorPosition mSearchSelectionStart;
    CursorPosition mSearchSelectionEnd;
    bool mIsSearchSelectionActive = false;
    int mVisibleLineCount = 0;
    int mFindChunk = 0;
    int mCursorColumn = 0;
    int mMaxChunksInCache = 5;
    int mChunkSize = 1024*1024;
    int mMaxLineWidth = 1024;
    bool mDebugMode = false;
};

} // namespace studio
} // namespace gams

#endif // CHUNKTEXTMAPPER_H
