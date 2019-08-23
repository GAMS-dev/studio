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
#include "memorymapper.h"
#include "file/dynamicfile.h"
#include "logger.h"

namespace gams {
namespace studio {

enum BaseFormat {old, debug, error, lstLink, fileLink};
static int CErrorBound = 50;        // The count of errors created at the beginning and the end (each the count)
static int CDirectErrors = 3;       // The count of errors created immediately
static int CParseLinesMax = 23;     // The maximum count of gathered lines befor updating the display
static int CRefreshTimeMax = 50;    // The maximum time (in ms) to wait until the output is updated (after changed)

MemoryMapper::MemoryMapper(QObject *parent) : AbstractTextMapper (parent)
{
    mState.deep = true;
    mRunFinishedTimer.setInterval(10);
    mRunFinishedTimer.setSingleShot(true);
    connect(&mRunFinishedTimer, &QTimer::timeout, this, &MemoryMapper::runFinished);
    mMarksHead.reserve(CErrorBound);
    mMarksTail.setCapacity(CErrorBound);
    // old
    QTextCharFormat fmt;
    fmt.setForeground(QColor(165,165,165));
    mBaseFormat << fmt;
    // debug
    fmt = QTextCharFormat();
    fmt.setForeground(QColor(120,150,100));
    mBaseFormat << fmt;
    // error
    fmt = QTextCharFormat();
    fmt.setAnchor(true);
    fmt.setForeground(Qt::darkRed);
    fmt.setUnderlineColor(Qt::darkRed);
    fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    mBaseFormat << fmt;
    // lstLink
    fmt = QTextCharFormat();
    fmt.setForeground(Qt::blue);
    fmt.setUnderlineColor(Qt::blue);
    fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    mBaseFormat << fmt;
    // fileLink
    fmt = QTextCharFormat();
    fmt.setForeground(Qt::darkGreen);
    fmt.setUnderlineColor(Qt::darkGreen);
    fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    mBaseFormat << fmt;
}

void MemoryMapper::setLogParser(LogParser *parser)
{
    mLogParser = parser;
}

qint64 MemoryMapper::size() const
{
    if (mSize < 0) {
        recalcSize();
    }
    return mSize;
}

void MemoryMapper::recalcSize() const
{
    mSize = 0;
    for (Chunk* chunk: mChunks) {
        mSize += chunk->size();
    }
}

void MemoryMapper::invalidateSize()
{
    mSize = -1;
}

AbstractTextMapper::Chunk *MemoryMapper::addChunk(bool startUnit)
{
    // IF we already have an empty chunk at the end, take it
    if (mChunks.size() && !mChunks.last()->size()) {
        // IF the Unit contains more than this empty chunk, cut it and append a new Unit
        if (startUnit && mUnits.size() && mUnits.last().chunkCount > 1) {
            --mUnits.last().chunkCount;
            mUnits << Unit(mChunks.last());
            ++mUnits.last().chunkCount;
        }
        return mChunks.last();
    }
    // create the new chunk
    Chunk *chunk = new Chunk();
    chunk->bArray.resize(chunkSize());
    chunk->bStart = mChunks.size() ? mChunks.last()->bStart + mChunks.last()->size() : 0;
    chunk->lineBytes << 0;
    chunk->nr = chunkCount();
    mChunks << chunk;
    invalidateLineOffsets(chunk);

    if (startUnit || !mUnits.size()) {
        mUnits << Unit(mChunks.last());
        mShrinkLineCount = 0;
    }
    ++mUnits.last().chunkCount;
    return mChunks.last();
}

void MemoryMapper::shrinkLog()
{
    const QByteArray ellipsis(QString("\n...\n\n").toLatin1().data());

    Chunk * chunk = mUnits.last().firstChunk;
    if (chunk->nr == mChunks.last()->nr || chunk->lineCount() < 2)
        return; // only one chunk in current unit

    if (!mShrinkLineCount) {
        // the first chunk in the unit starts at bArray[0]
        while (chunk->size()+ellipsis.size() > chunkSize() && chunk->lineBytes.size() > 1) {
            chunk->lineBytes.removeLast();
        }
        // replace last line of first chunk by an ellipsis
        invalidateSize();
        chunk->bArray.replace(chunk->lineBytes.last(), ellipsis.length(), ellipsis);
        chunk->lineBytes << (chunk->lineBytes.last()+1) << (chunk->lineBytes.last() + ellipsis.length()-1)
                             << (chunk->lineBytes.last() + ellipsis.length());
        updateChunkMetrics(chunk);
    }

    // remove some lines at the start of the second active chunk
    chunk = mChunks[chunk->nr+1];
    int linesToRemove = (chunk->lineBytes.size() > 4 ? 4 : chunk->lineCount());
    invalidateSize();
    chunk->lineBytes.remove(0, linesToRemove);
    mShrinkLineCount += linesToRemove;

    // remove chunk if it is empty
    if (!chunk->size()) {
        mChunks.removeAt(chunk->nr);
        --mUnits.last().chunkCount;
        for (int i = chunk->nr; i < mChunks.size(); ++i)
            mChunks.at(i)->nr = i;
        chunk = mChunks.size() > chunk->nr ? mChunks.at(chunk->nr) : nullptr;
        updateChunkMetrics(chunk, true);
    }

    // update internal data to new sizes
    while (chunk) {
        updateChunkMetrics(chunk);
        chunk = mChunks.size() > chunk->nr+1 ? mChunks[chunk->nr+1] : nullptr;
    }
    recalcLineCount();
}

void MemoryMapper::recalcLineCount()
{
    mLineCount = 0;
    for (const Unit &u: mUnits) {
        if (u.folded) ++mLineCount;
        else {
            for (int i = 0; i < u.chunkCount; ++i) {
                mLineCount += mChunks.at(u.firstChunk->nr + i)->lineCount();
            }
        }
    }
    updateMaxTop();
    emitBlockCountChanged();
}

MemoryMapper::LineRef MemoryMapper::logLineToRef(const int &lineNr)
{
    // lineNr: relative line nr in current run
    LineRef res;
    res.chunk = mUnits.last().firstChunk;
    res.relLine = lineNr;

    // takes care of possible ellipsis in first chunk of the run
    int headLines = mShrinkLineCount ? res.chunk->lineCount()-3 : res.chunk->lineCount();

    // if line is in first chunk (before ellipsis)
    if (res.relLine < headLines) return res;

    res.relLine = lineNr - mShrinkLineCount;
    while (res.relLine >= res.chunk->lineCount()) {
        res.relLine -= headLines;
        res.chunk = nextChunk(res.chunk);
        if (!res.chunk) break;
        headLines = res.chunk->lineCount();
    }
    return res;
}

AbstractTextMapper::Chunk *MemoryMapper::nextChunk(AbstractTextMapper::Chunk *chunk)
{
    if (!chunk) return nullptr;
    int i = mChunks.indexOf(chunk);
    if (i+1 < mChunks.size()) return mChunks.at(i+1);
    return nullptr;
}

int MemoryMapper::currentRunLines()
{
    int res = -1;
    Chunk *chunk = mUnits.last().firstChunk;
    while (chunk) {
        res += chunk->lineCount();
        chunk = nextChunk(chunk);
    }
    return mShrinkLineCount ? res + mShrinkLineCount - 3 : res;
}

void MemoryMapper::updateChunkMetrics(Chunk *chunk, bool cutRemain)
{
    if (mChunks.size()-1 > chunk->nr) {
        qint64 newBStart = chunk->bStart + chunk->size();
        mChunks[chunk->nr+1]->bStart = newBStart;
    }
    invalidateLineOffsets(chunk, cutRemain);
}

int MemoryMapper::lineCount() const
{
    return mLineCount * (debugMode() ? 2 : 1);
}

void MemoryMapper::startRun()
{
    addChunk(true);         // prepare chunk and unit for new run
    mMarkers.clear();       // LineRefs to remembered marks of current run (persists until next run)
    mMarksHead.clear();     // temp top lines with marks (converted to markers at end of run)
    mMarksTail.clear();     // temp bottom lines with marks (converted to markers at end of run)
    mShrinkLineCount = 0;   // count of lines removed by shrinking

    mErrCount = 0;
    mAddedLines = 0;
    mDisplayLastLineLen = 0;
    mDisplayLinesOverwrite = false;
    mInputState = InputState();
    appendEmptyLine();
}

void MemoryMapper::endRun()
{
    fetchDisplay();
    mLastLineLen = 0;
    mLastLineIsOpen = false;
    fetchLog();
    runFinished();
}

int MemoryMapper::firstErrorLine()
{
    if (mMarkers.isEmpty()) return -1;
    int res = 0;
    Chunk *chunkOfErr1 = mMarkers.at(0).chunk;
    for (Chunk *chunk : mChunks) {
        if (chunk == chunkOfErr1) break;
        res += chunk->lineCount();
    }
    res += mMarkers.at(0).relLine;
    return res;
}

void MemoryMapper::runFinished()
{
    TRACETIME();
    mMarksTail.normalizeIndexes();
    mMarkers.clear();
    mMarkers.reserve(mMarksHead.size() + mMarksTail.size());
    for (const int &lineNr: mMarksHead) {
        LineRef ref = logLineToRef(lineNr);
        if (ref.chunk) mMarkers << ref;
    }
    for (int i = mMarksTail.firstIndex(); i <= mMarksTail.lastIndex(); ++i) {
        LineRef ref = logLineToRef(mMarksTail.at(i));
        if (ref.chunk) mMarkers << ref;
    }

    for (int i = CDirectErrors; i < mMarkers.size(); ++i) {
        createErrorMarks(mMarkers.at(i));
    }

    mMarksHead.clear();
    mMarksTail.clear();
    recalcLineCount();
    PEEKTIME() << " ms FINISH for " << mMarkers.size() << " Markers";
}

void MemoryMapper::createErrorMarks(MemoryMapper::LineRef ref)
{
    QByteArray data = lineData(ref);
    QString rawLine;
    bool hasError = false;
    LogParser::MarksBlockState mbState;
    QString line = mLogParser->parseLine(data, rawLine, hasError, mbState);
    LogParser::MarksBlockState mbFollowState = mbState;
    if (mbState.errData.errNr > 0) {
        // compile-time error have descriptions in the following lines
        while (true) {
            ref = nextRef(ref);
            data = lineData(ref);
            if (!data.startsWith("   ")) break;
            if (!mbState.errData.text.isEmpty())
                mbState.errData.text.append('\n');
            mbState.errData.text += data;
        }
        if (!mbState.errData.text.isEmpty())
            emit mLogParser->setErrorText(mbState.errData.lstLine, mbState.errData.text);
    }
    emit createMarks(mbState.marks);
}

void MemoryMapper::appendLineData(const QByteArray &data, Chunk *&chunk)
{
    if (!chunk->lineCount())
        appendEmptyLine();
    int lastLineStart = chunk->lineBytes.at(chunk->lineCount()-1);
    int lastLineEnd = chunk->lineBytes.last()-1;
    Chunk* changedChunk = chunk;
    if (lastLineEnd + data.length() +1 > chunkSize()) {
        // move last line data to new chunk
        QByteArray part;
        Chunk *newChunk = addChunk();
        if (lastLineEnd > lastLineStart) {
            part.setRawData(chunk->bArray.data() + lastLineStart, uint(lastLineEnd-lastLineStart));
            newChunk->bArray.replace(0, lastLineEnd-lastLineStart, part);
        }
        newChunk->lineBytes << (lastLineEnd-lastLineStart+1);
        chunk->lineBytes.removeLast();
        newChunk->bStart = chunk->bStart + chunk->size();
        chunk = newChunk;
        lastLineStart = 0;
        lastLineEnd = chunk->lineBytes.last()-1;
    }

    invalidateSize();
    chunk->bArray.replace(lastLineEnd, data.length(), data);
    chunk->lineBytes.last() = lastLineEnd+data.length()+1;
    chunk->bArray[lastLineEnd+data.length()] = '\n';
    updateChunkMetrics(changedChunk);
    updateOutputCache();
}

void MemoryMapper::updateOutputCache()
{
    if (!mDisplayQuickFormats.size())
        mDisplayCacheChanged.start();

    Chunk *chunk = mChunks.last();
    if (!chunk || !chunk->lineCount())
        return;
    int start = chunk->lineBytes.at(chunk->lineCount()-1);
    int end = chunk->lineBytes.last()-1; // -1 to skip the trailing LF

    QString line;
    int lastLinkStart = -1;
    LineFormat fmt;
    int lstLine = -1;
    mLogParser->quickParse(chunk->bArray, start, end, line, lastLinkStart, lstLine);
    if (mCurrentLstLineRef >= 0) {
        if (end >= start+3 && chunk->bArray.at(start)==' ' && chunk->bArray.at(start+1)==' ') {
            if (!mCurrentErrText.isEmpty())
                mCurrentErrText.append('\n');
            mCurrentErrText += line;
        } else {
            emit mLogParser->setErrorText(mCurrentLstLineRef, mCurrentErrText);
            mCurrentErrText.clear();
            mCurrentLstLineRef = -1;
        }
    }
    if (mErrCount < CErrorBound-1 && lstLine >= 0)
        mCurrentLstLineRef = lstLine;

    if (lastLinkStart >= start) {
        if (lastLinkStart > start+line.length() || line.startsWith("*** Error")) {
            fmt = LineFormat(4, line.length(), mBaseFormat.at(error));
            if (lastLinkStart > start+line.length()) {
                fmt.extraLstFormat = &mBaseFormat.at(lstLink);
            }
            int lineNr = currentRunLines();
            if (mErrCount < CErrorBound) {
                if (mMarksHead.isEmpty() || mMarksHead.last() != lineNr) {
                    ++mErrCount;
                    mMarksHead << lineNr;
                    if (mErrCount < CDirectErrors) {
                        createErrorMarks(logLineToRef(lineNr));
                    }
                }
            } else if (mMarksTail.isEmpty() || mMarksTail.last() != lineNr) {
                ++mErrCount;
                mMarksTail.append(lineNr);
            }
        } else if (chunk->bArray.mid(start+line.length()+1, 3) == "LST") {
            fmt = LineFormat(4, line.length(), mBaseFormat.at(lstLink));
        } else {
            fmt = LineFormat(4, line.length(), mBaseFormat.at(fileLink));
        }
    }

    if (mLastLineIsOpen && mLastLineLen > line.length()) {
        appendEmptyLine();
        mLastLineLen = 0;
        mLastLineIsOpen = false;
    }

    // update log-file cache
    mNewLogLines << line;

    // update display cache
    if (mDisplayNewLines.length()) {
        mDisplayNewLines.replace(mDisplayNewLines.length()-1, line);            // extend (replace) last Line
        mDisplayQuickFormats.replace(mDisplayQuickFormats.length()-1, fmt);  // update last format
    } else {
        // nothing to replace - append new part of the line
        mDisplayNewLines << line.right(line.length()-mLastLineLen);
        mDisplayQuickFormats << fmt;
    }
    mLastLineLen = line.length();
    if (mDisplayLinesOverwrite) {
        // last line has to be overwritten - update immediately
        fetchDisplay();
    }

}

void MemoryMapper::appendEmptyLine()
{
    // update cached lines in edit and log
    while (mDisplayNewLines.length() > visibleLineCount()) {
        mDisplayNewLines.removeAt(0);
        mDisplayQuickFormats.removeAt(0);
    }

    if (mDisplayQuickFormats.length() && mDisplayCacheChanged.elapsed() > CRefreshTimeMax)
        fetchDisplay();
    if (mNewLogLines.length() >= CParseLinesMax)
        fetchLog();

    // update chunk (switch to new if filled up)
    Chunk *chunk = mChunks.last();
    if (chunk->lineBytes.last() + 1 > chunkSize())
        chunk = addChunk();
    invalidateSize();
    chunk->lineBytes << chunk->lineBytes.last()+1;
    chunk->bArray[chunk->lineBytes.last()-1] = '\n';

    // update output cache (states)
    if (mDisplayNewLines.isEmpty()) {
        mDisplayLastLineLen = 0;
        mDisplayLinesOverwrite = false;
    } else {
        mDisplayNewLines << QString();
        mDisplayQuickFormats << LineFormat();
    }
    mLastLineIsOpen = false;
    mLastLineLen = 0;
}

void MemoryMapper::clearLastLine()
{
    Chunk *chunk = mChunks.last();
    if (chunk->lineCount() < 2) return;

    // update internal data
    int start = chunk->lineBytes.at(chunk->lineCount()-1);
    if (start+1 < chunk->lineBytes.last()) {
        invalidateSize();
        chunk->lineBytes.last() = start+1;
        chunk->bArray[start] = '\n';
    }
    // update output-cache
    if (!mDisplayNewLines.isEmpty()) {
        fetchDisplay();
        mDisplayNewLines << QString();
        mDisplayQuickFormats << LineFormat();
    }
    mDisplayLastLineLen = 0;
    mDisplayLinesOverwrite = true;
    mLastLineLen = 0;
}

void MemoryMapper::fetchLog()
{
    emit appendLines(mNewLogLines);
    mNewLogLines.clear();
}

void MemoryMapper::fetchDisplay()
{
    emit appendDisplayLines(mDisplayNewLines, mDisplayLastLineLen, mDisplayLinesOverwrite, mDisplayQuickFormats);
    mDisplayNewLines.clear();
    mDisplayQuickFormats.clear();
    mDisplayLastLineLen = mLastLineLen;
    mDisplayLinesOverwrite = false;
}

void MemoryMapper::addProcessData(const QByteArray &data)
{
    Q_ASSERT_X(mChunks.size(), Q_FUNC_INFO, "Need to call startRun() before adding data.");
    Chunk *chunk = mChunks.last();
    int len = 0;
    int start = 0;
    QByteArray midData;

    for (int i = 0 ; i < data.length() ; ++i) {
        // check for line breaks
        if (data.at(i) == '\r') {
            len = i-start;
            if (i+1 < data.size() && data.at(i+1) == '\n') {
                // normal line break in Windows format - "\r\n"
                ++i;
                if (len) {
                    midData.setRawData(data.data()+start, uint(len));
                    appendLineData(midData, chunk);
                }
                start = i + 1;
                appendEmptyLine();
            } else {
                // concealing standalone CR - "\r"
                start = i + 1;
                if (len || mLastLineIsOpen) {
                    updateOutputCache();
                }
                clearLastLine();
            }
        } else if (data.at(i) == '\n') {
            // normal line break in Linux/Mac format - "\n"
            len = i-start;
            if (len) {
                midData.setRawData(data.data()+start, uint(len));
                appendLineData(midData, chunk);
            }
            start = i + 1;
            appendEmptyLine();
        }
    }
    if (start < data.length()) {
        len = data.length()-start;
        if (len) {
            midData.setRawData(data.data()+start, uint(len));
            appendLineData(midData, chunk);
            mLastLineIsOpen = true;
        }
    }
    if (size() - mUnits.last().firstChunk->bStart > chunkSize() * 2)
        shrinkLog();
    else {
        updateChunkMetrics(chunk);
        recalcLineCount();
    }
}

void MemoryMapper::reset()
{
    AbstractTextMapper::reset();
    mChunks.clear();
    mUnits.clear();
    invalidateSize();
    mLineCount = 0;
    emit blockCountChanged();
}

QString MemoryMapper::lines(int localLineNrFrom, int lineCount) const
{
    return AbstractTextMapper::lines(localLineNrFrom, lineCount);
}

QString MemoryMapper::lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const
{
    formats.reserve(lineCount);
    if (debugMode()) {
        localLineNrFrom /= 2;
        lineCount /= 2;
    }
    int activationLine;
    QByteArray data = rawLines(localLineNrFrom, lineCount, mUnits.last().firstChunk->nr, activationLine);
    if (debugMode()) activationLine *= 2;

    QStringList res;
    LogParser::MarksBlockState mbState;
    LineFormat *actErrFormat = nullptr;
    mbState.deep = false;
    QString rawLine;
    int from = 0;
    int to = 0;
    int next = -1;
    while (to < data.length()) {
        if (data.at(to) == '\n') next = to+1;
        else if (data.at(to) == '\r') {
            if (to == data.length()-1)
                next = to+1;
            else if (data.at(to) != '\n') {
                next = to+1;
            } else
                next = to+2;
        }
        if (next < 0) {
            ++to;
            continue;
        }
        int len = to-from;
        QString line;
        if (len == 0) {
            if (debugMode()) {
                res << "";
                formats << LineFormat();
            }
            res << "";
            formats << LineFormat();
        } else {
            bool hasError = false;
            QByteArray lineData = data.mid(from, len);
//            QString line = mLogParser->quickParse(lineData, rawLine, hasMark, hasError);
            QString line = mLogParser->parseLine(lineData, rawLine, hasError, mbState);
            if (debugMode()) {
                res << rawLine;
                formats << LineFormat(0, rawLine.length(),mBaseFormat.at(debug));
            }
            res << line;
            if (res.size() < activationLine) {
                formats << LineFormat(0, line.length(), mBaseFormat.at(old));
            } else if (mbState.marks.hasMark()) {
                if (mbState.marks.hasErr()) {
                    QString toolTip = mbState.errData.text.isEmpty() ? mbState.marks.hRef : mbState.errData.text;
                    formats << LineFormat(4, line.length(), mBaseFormat.at(error), toolTip, mbState.marks.errRef);
                    if (!mbState.marks.hRef.isEmpty()) {
                        formats.last().extraLstFormat = &mBaseFormat.at(lstLink);
                        formats.last().extraLstHRef = mbState.marks.hRef;
                    }
                    actErrFormat = &formats.last();
                } else if (hasError) {
                    formats << LineFormat(4, line.length(), mBaseFormat.at(error), mbState.errData.text, mbState.marks.hRef);
                    if (!mbState.marks.hRef.isEmpty()) {
                        formats.last().extraLstFormat = &mBaseFormat.at(lstLink);
                        formats.last().extraLstHRef = mbState.marks.hRef;
                    }
                    actErrFormat = &formats.last();
                } else if (mbState.marks.hRef.startsWith("FIL:")) {
                    formats << LineFormat(4, line.length(), mBaseFormat.at(fileLink), mbState.errData.text, mbState.marks.hRef);
                } else if (mbState.marks.hRef.startsWith("LST:")) {
                    formats << LineFormat(4, line.length(), mBaseFormat.at(lstLink), mbState.errData.text, mbState.marks.hRef);
                }
            } else if (hasError) {
                formats << LineFormat(0, line.length(),mBaseFormat.at(error));
            } else
                formats << LineFormat();
//            if (actErrFormat) {
//                if (mbState.inErrorText) {
//                    actErrFormat->format.setToolTip(mbState.errData.text);
//                } else {
//                    actErrFormat = nullptr;
//                }
//            }
        }

        from = next;
        to = next;
        next = -1;
    }
    res << "";
    return res.join("\n");
}

void MemoryMapper::dump()
{
//    int iCh = 0;
    DEB() << "\n";
    DEB() << "---- size: " << mSize ;
//    int sum = 0;
//    for (Chunk *chunk : mChunks) {
//        for (int lineNr = 0; lineNr < chunk->lineBytes.size()-1; ++lineNr) {
//            QString line;
//            for (int i = chunk->lineBytes.at(lineNr); i < chunk->lineBytes.at(lineNr+1); ++i) {
//                if (chunk->bArray.at(i) == '\r') line += "\\r";
//                else if (chunk->bArray.at(i) == '\n') line += "\\n";
//                else line += chunk->bArray.at(i);
//            }
//            DEB() << iCh << " DATA: " << line;
//        }
//        DEB() << iCh << " size: " << chunk->size();
//        sum += chunk->size();
//        ++iCh;
//    }
    for (const Unit &u : mUnits) {
        DEB() << "  UNIT: from " << u.firstChunk->nr << "+" << u.chunkCount-1 << "  size1: " << u.firstChunk->size();
    }
    dumpPos();
}

int MemoryMapper::knownLineNrs() const
{
    return lineCount();
}

void MemoryMapper::setDebugMode(bool debug)
{
    AbstractTextMapper::setDebugMode(debug);
}

int MemoryMapper::chunkCount() const
{
    return mChunks.size();
}

AbstractTextMapper::Chunk *MemoryMapper::getChunk(int chunkNr) const
{
    return mChunks.at(chunkNr);
}

MemoryMapper::LineRef MemoryMapper::nextRef(const MemoryMapper::LineRef &ref)
{
    if (!ref.chunk) return LineRef();
    LineRef res = ref;
    if (res.relLine < res.chunk->lineCount()-1) {
        ++res.relLine;
    } else {
        res.chunk = nextChunk(res.chunk);
        res.relLine = 0;
    }
    return res;
}

QByteArray MemoryMapper::lineData(const MemoryMapper::LineRef &ref)
{
    if (!ref.chunk) return QByteArray();
    int byteFrom = ref.chunk->lineBytes.at(ref.relLine);
    int byteTo = ref.chunk->lineBytes.at(ref.relLine+1);
    while ((ref.chunk->bArray.at(byteTo) == '\n' || ref.chunk->bArray.at(byteTo) == '\r') && byteTo > byteFrom)
        --byteTo;
    return ref.chunk->bArray.mid(byteFrom, byteTo - byteFrom -1);
}

} // namespace studio
} // namespace gams
