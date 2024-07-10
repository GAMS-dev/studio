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
#include "memorymapper.h"
#include "file/dynamicfile.h"
#include "logger.h"
#include "theme.h"

namespace gams {
namespace studio {

enum BaseFormat {old, debug, error, lstLink, fileLink, dirLink, baseFormatCount};
static int CErrorBound = 50;        // The count of errors created at the beginning and the end (each the count)
static int CDirectErrors = 3;       // The count of errors created immediately
static int CParseLinesMax = 23;     // The maximum count of gathered lines befor updating the display
static int CRefreshTimeMax = 100;    // The maximum time (in ms) to wait until the output is updated (after changed)
static int CKeptRunCount = 5;

MemoryMapper::MemoryMapper(QObject *parent) : ChunkTextMapper (parent)
{
    mRunFinishedTimer.setInterval(10);
    mRunFinishedTimer.setSingleShot(true);
    mNewLogLines.reserve(CParseLinesMax+1);
    connect(&mRunFinishedTimer, &QTimer::timeout, this, &MemoryMapper::runFinished);
    connect(&mPendingTimer, &QTimer::timeout, this, &MemoryMapper::processPending);
    mPendingTimer.setSingleShot(true);
    mPending = PendingNothing;
    mMarksHead.reserve(CErrorBound);
    mMarksTail.resize(CErrorBound);
    updateTheme();
    addChunk(true);
}

void MemoryMapper::updateTheme()
{
    while (mBaseFormat.size() < baseFormatCount) {
        mBaseFormat << QTextCharFormat();
    }
    mBaseFormat[old].setForeground(QColor(125,125,125));
    mBaseFormat[debug].setForeground(QColor(120,150,100));
    mBaseFormat[error].setAnchor(true);
    mBaseFormat[error].setForeground(Theme::color(Theme::Mark_errorFg));
    mBaseFormat[error].setUnderlineColor(Theme::color(Theme::Mark_errorFg));
    mBaseFormat[error].setUnderlineStyle(QTextCharFormat::WaveUnderline);
    mBaseFormat[lstLink].setForeground(Theme::color(Theme::Mark_listingFg));
    mBaseFormat[lstLink].setUnderlineColor(Theme::color(Theme::Mark_listingFg));
    mBaseFormat[lstLink].setUnderlineStyle(QTextCharFormat::SingleUnderline);
    mBaseFormat[fileLink].setForeground(Theme::color(Theme::Mark_fileFg));
    mBaseFormat[fileLink].setUnderlineColor(Theme::color(Theme::Mark_fileFg));
    mBaseFormat[fileLink].setUnderlineStyle(QTextCharFormat::SingleUnderline);
}

MemoryMapper::~MemoryMapper()
{
    while (!mChunks.isEmpty())
        removeChunk(mChunks.last()->nr);

    setLogParser(nullptr);
}

void MemoryMapper::setLogParser(LogParser *parser)
{
    if (mLogParser) delete mLogParser;
    mLogParser = parser;
}

LogParser *MemoryMapper::logParser()
{
    return mLogParser;
}

qint64 MemoryMapper::size() const
{
    if (mSize < 0) {
        mSize = 0;
        for (Chunk* chunk: mChunks) {
            mSize += chunk->size();
        }
    }
    return mSize;
}

void MemoryMapper::invalidateSize()
{
    mSize = -1;
}

void MemoryMapper::markLastLine()
{
    if (!mChunks.size()) return;
    Chunk *chunk = mChunks.last();
    if (chunk->markedRegion.x() < 0)
        chunk->markedRegion.rx() = chunk->lineCount()-1;
    if (chunk->markedRegion.y() < chunk->lineCount())
        chunk->markedRegion.ry() = chunk->lineCount();
}

QVector<bool> MemoryMapper::markedLines(int localLineNrFrom, int lineCount) const
{
    QVector<bool> res;
    if (!size()) return res;
    res.reserve(lineCount);
    QPair<int,int> interval = QPair<int,int>(localLineNrFrom, lineCount); // <start, size>
    while (interval.second) {
        QPair<int,int> chunkInterval;
        Chunk *chunk = chunkForRelativeLine(interval.first, &chunkInterval.first);
        if (!chunk) break;
        chunkInterval.second = qMin(interval.second, chunk->lineCount() - chunkInterval.first);
        for (int i = chunkInterval.first; i < chunkInterval.first + chunkInterval.second; ++i) {
            res << (chunk->markedRegion.x() >= 0 && chunk->markedRegion.x() <= i && chunk->markedRegion.y() > i);
        }
        interval.first += chunkInterval.second;
        interval.second -= chunkInterval.second;
        if (chunk->nr == chunkCount()-1) break;
    }
    return res;

}

ChunkTextMapper::Chunk *MemoryMapper::addChunk(bool startUnit)
{
    // IF we already have an empty chunk at the end, take it
    if (mChunks.size() && !mChunks.last()->size()) {
        // IF the Unit has content, cut it and append a new Unit
        if (startUnit && mUnits.size() && mUnits.last().chunkCount > 1) {
            --mUnits.last().chunkCount;
            mUnits << Unit(mChunks.last());
            ++mUnits.last().chunkCount;
        }
        return mChunks.last();
    }
    // ELSE create the new chunk
    Chunk *chunk = new Chunk();
    chunk->bArray.resize(chunkSize());
    chunk->bStart = mChunks.size() ? mChunks.last()->bStart + mChunks.last()->size() : 0;
    chunk->lineBytes << 0;
    chunk->nr = chunkCount();
    chunk->markedRegion = QPoint(-1, -1);
    mChunks << chunk;
    invalidateLineOffsets(chunk);

    if (startUnit || !mUnits.size()) {
        mUnits << Unit(mChunks.last());
        mShrinkLineCount = 0;
    }
    ++mUnits.last().chunkCount;
    while (mUnits.count() > CKeptRunCount) {
        removeChunk(mUnits.first().firstChunk->nr);
    }
    return mChunks.last();
}

void MemoryMapper::shrinkLog(qint64 minBytes)
{
    const QByteArray ellipsis(QString("\n...\n\n").toLatin1().data());

    Chunk * chunk = mUnits.last().firstChunk;
    if (chunk->nr == mChunks.last()->nr || chunk->lineCount() < 2)
        return; // only one chunk in current unit

    if (!mShrinkLineCount) {
        // the first chunk in the unit starts at bArray[0]
        while (chunk->size()+ellipsis.size() > chunkSize() && chunk->lineBytes.size() > 1) {
            chunk->lineBytes.removeLast();
            if (chunk->lineCount() < chunk->markedRegion.x())
                chunk->markedRegion = QPoint(-1, -1);
            else if (chunk->lineCount() < chunk->markedRegion.y())
                chunk->markedRegion.ry() = chunk->lineCount();
        }
        // replace last line of first chunk by an ellipsis
        invalidateSize();
        if (chunk->lineCount() == chunk->markedRegion.y())
            chunk->markedRegion.ry() += 3;
        chunk->bArray.replace(chunk->lineBytes.last(), ellipsis.length(), ellipsis);
        chunk->lineBytes << (chunk->lineBytes.last()+1) << (chunk->lineBytes.last() + ellipsis.length()-1)
                         << (chunk->lineBytes.last() + ellipsis.length());
        updateChunkMetrics(chunk);
    }

    // remove some lines at the start of the second active chunk
    chunk = mChunks[chunk->nr+1];
    int linesToRemove = 1;
    while (minBytes > chunk->lineBytes.at(linesToRemove)) {
        ++linesToRemove;
        if (linesToRemove > chunk->lineCount()) break;
    }
    invalidateSize();
    chunk->lineBytes.remove(0, linesToRemove);
    mShrinkLineCount += linesToRemove;
    if (chunk->lineCount() < chunk->markedRegion.x())
        chunk->markedRegion = QPoint(-1, -1);
    else if (chunk->lineCount() < chunk->markedRegion.y())
        chunk->markedRegion.ry() = chunk->lineCount();

    // remove chunk if it is empty
    if (!chunk->size()) {
        Chunk *delChunk = chunk;
        chunk = (mChunks.size() > chunk->nr+1) ? mChunks.at(chunk->nr+1) : nullptr;
        removeChunk(delChunk->nr);
        if (chunk)
            updateChunkMetrics(chunk, true);
    }

    // update internal data to new sizes
    while (chunk) {
        updateChunkMetrics(chunk);
        chunk = (mChunks.size() > chunk->nr+1) ? mChunks.at(chunk->nr+1) : nullptr;
    }
    recalcLineCount();
}

bool MemoryMapper::ensureSpace(qint64 bytes)
{
    if (size() - mUnits.last().firstChunk->bStart + bytes >= chunkSize() * 2) {
        shrinkLog(bytes);
        return true;
    }
    return false;
}

void MemoryMapper::recalcLineCount()
{
    int lineCount = 0;
    for (const Unit &u: std::as_const(mUnits)) {
        if (u.folded) ++lineCount;
        else {
            for (int i = 0; i < u.chunkCount; ++i) {
                lineCount += mChunks.at(u.firstChunk->nr + i)->lineCount();
            }
        }
    }
    if (mLineCount != lineCount) {
        mLineCount = lineCount;
        updateMaxTop();
        newPending(PendingBlockCountChange);
    }
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

ChunkTextMapper::Chunk *MemoryMapper::nextChunk(ChunkTextMapper::Chunk *chunk)
{
    if (!chunk) return nullptr;
    int i = mChunks.indexOf(chunk);
    if (i+1 < mChunks.size()) return mChunks.at(i+1);
    return nullptr;
}

ChunkTextMapper::Chunk *MemoryMapper::prevChunk(ChunkTextMapper::Chunk *chunk)
{
    if (!chunk) return nullptr;
    int i = mChunks.indexOf(chunk);
    if (i > 0) return mChunks.at(i-1);
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
    return mLineCount /* * (debugMode() ? 2 : 1) */;
}

void MemoryMapper::startRun()
{
    addChunk(true);         // prepare chunk and unit for new run
    mMarkers.clear();       // LineRefs to remembered marks of current run (persists until next run)
    mMarksHead.clear();     // temp top lines with marks (converted to markers at end of run)
    mMarksTail.clear();     // temp bottom lines with marks (converted to markers at end of run)
    mShrinkLineCount = 0;   // count of lines removed by shrinking

    mErrCount = 0;
    mInstantRefresh = false;
    mNewLines = 0;
    appendEmptyLine();
}

void MemoryMapper::endRun()
{
    processPending();
    mLastLineLen = 0;
    mLastLineIsOpen = false;
    fetchLog();
    runFinished();
//    dump();
}

int MemoryMapper::firstErrorLine()
{
    if (mMarkers.isEmpty()) return -1;
    int res = 0;
    Chunk *chunkOfErr1 = mMarkers.at(0).chunk;
    for (Chunk *chunk : std::as_const(mChunks)) {
        if (chunk == chunkOfErr1) break;
        res += chunk->lineCount();
    }
    res += mMarkers.at(0).relLine;
    return res;
}

void MemoryMapper::runFinished()
{
    mMarkers.clear();
    mMarkers.reserve(mMarksHead.size() + mMarksTail.size());
    for (const int &lineNr: std::as_const(mMarksHead)) {
        LineRef ref = logLineToRef(lineNr);
        if (ref.chunk) mMarkers << ref;
    }
    const QVector<int> tail = mMarksTail.data();
    for (const int &lineNr: tail) {
        LineRef ref = logLineToRef(lineNr);
        if (ref.chunk) mMarkers << ref;
    }

    for (int i = CDirectErrors; i < mMarkers.size(); ++i) {
        createErrorMarks(mMarkers.at(i), true);
    }

    mMarksHead.clear();
    mMarksTail.clear();
    recalcLineCount();
}

void MemoryMapper::createErrorMarks(MemoryMapper::LineRef ref, bool readErrorText)
{
    QByteArray data = lineData(ref);
    QString rawLine;
    bool hasError = false;
    LogParser::MarksBlockState mbState;
    mLogParser->parseLine(data, rawLine, hasError, mbState);
    if (!mbState.switchLst.isEmpty()) {
        emit switchLst(mbState.switchLst);
        mbState.switchLst = QString();
    }
    LogParser::MarksBlockState mbFollowState = mbState;
    if (readErrorText && mbState.errData.errNr > 0) {
        // compile-time error have descriptions in the following lines
        while (true) {
            ref = nextRef(ref);
            data = lineData(ref);
            if (!data.startsWith("   ")) break;
            if (mbState.errData.text.isEmpty()) {
                mbState.errData.text.append(QString("%1\t").arg(mbState.errData.errNr));
            } else
                mbState.errData.text.append("\n\t");
            mbState.errData.text += data.trimmed();
        }
        if (!mbState.errData.text.isEmpty())
            emit mLogParser->setErrorText(mbState.errData.lstLine, mbState.errData.text);
    }
    emit createMarks(mbState.marks);
}

void MemoryMapper::appendLineData(const QByteArray &data, Chunk *&chunk, bool mark)
{
    if (!chunk->lineCount())
        appendEmptyLine(mark);
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
        lastLineEnd = chunk->lineBytes.last()-1;
    }

    invalidateSize();
    chunk->bArray.replace(lastLineEnd, data.length(), data);
    chunk->lineBytes.last() = lastLineEnd+data.length()+1;
    chunk->bArray[lastLineEnd+data.length()] = '\n';
    updateChunkMetrics(changedChunk);
    parseNewLine();
}

void MemoryMapper::parseNewLine()
{
    if (!mNewLines)
        mDisplayCacheChanged.start();

    Chunk *chunk = mChunks.last();
    if (!chunk || !chunk->lineCount())
        return;
    int start = chunk->lineBytes.at(chunk->lineCount()-1);
    int end = chunk->lineBytes.last()-1; // -1 to skip the trailing LF

    QString line;
    int lastLinkStart = -1;
    int lstLine = -1;
    mLogParser->quickParse(chunk->bArray, start, end, line, lastLinkStart, lstLine);
    if (mCurrentLstLineRef >= 0) {
        if (end >= start+3 && chunk->bArray.at(start)==' ') {
            if (mCurrentErrText.isEmpty()) {
                mCurrentErrText.append(mCurrentErrorNr >= 0 ? QString("%1\t").arg(mCurrentErrorNr) : "\t");
            } else {
                mCurrentErrText.append("\n\t");
            }
            mCurrentErrText += line.trimmed();
        } else {
            emit mLogParser->setErrorText(mCurrentLstLineRef, mCurrentErrText);
            mCurrentErrText.clear();
            mCurrentLstLineRef = -1;
            mCurrentErrorNr = -1;
        }
    }
    if (mErrCount < CDirectErrors && lstLine >= 0) {
        mCurrentLstLineRef = lstLine;
        if (line.startsWith("*** Error ") && line.length() > 25) {
            int i = 9;
            while (line.size() > i+1 && line.at(i+1) == ' ') ++i;
            int len = 0;
            while (line.size() > i+len+1 && line.at(i+len+1) >= '0' && line.at(i+len+1) <= '9') ++len;
            bool ok = false;
            if (len > 0) mCurrentErrorNr = line.mid(i, len+1).toInt(&ok);
            if (!ok) mCurrentErrorNr = -1;
        }
    }

    if (lastLinkStart >= start) {
        if (lastLinkStart+6 < end && chunk->bArray.mid(lastLinkStart+1, 4) == "FIL:") {
            int i = lastLinkStart+6;
            QChar delim = chunk->bArray.at(i-1);
            if (delim == '\"' || delim == '\'') {
                int j = i;
                while (j < end && chunk->bArray.at(j) != delim) ++j;
                if (j < end) {
                    emit registerGeneratedFile(chunk->bArray.mid(i, j-i));
                }
            }
        }
        if (lastLinkStart > start+line.length() || line.startsWith("*** Error")) {
//            fmt = LineFormat(4, line.length(), mBaseFormat.at(error));
//            if (lastLinkStart > start+line.length()) {
//                fmt.extraLstFormat = &mBaseFormat.at(lstLink);
//            }
            int lineNr = currentRunLines();
            if (mErrCount < CErrorBound) {
                if (mMarksHead.isEmpty() || mMarksHead.last() != lineNr) {
                    mMarksHead << lineNr;
                    if (mErrCount < CDirectErrors) {
                        createErrorMarks(logLineToRef(lineNr), false);
                    }
                    ++mErrCount;
                }
            } else if (!mMarksTail.size() || mMarksTail.last() != lineNr) {
                ++mErrCount;
                mMarksTail.append(lineNr);
            }
//        } else if (chunk->bArray.mid(start+line.length()+1, 3) == "LST") {
//            fmt = LineFormat(4, line.length(), mBaseFormat.at(lstLink));
//        } else {
//            fmt = LineFormat(4, line.length(), mBaseFormat.at(fileLink));
        }
    }

    // update log-file cache
    mLogStack.setLine(line);

    if (mLastLineIsOpen && mLastLineLen > line.length()) {
        ensureSpace(1);
        appendEmptyLine(chunk->lineCount() == chunk->markedRegion.y());
        mLastLineLen = 0;
        mLastLineIsOpen = false;
    }

    mLastLineLen = line.length();
    if (mInstantRefresh) {
        // last line has to be overwritten - update immediately
        processPending();
    }
    ++mNewLines;
}

void MemoryMapper::appendEmptyLine(bool mark)
{
    mLogStack.commitLine();
    if (mLogStack.count() >= CParseLinesMax)
        fetchLog();

    // update chunk (switch to new if filled up)
    Chunk *chunk = mChunks.last();
    if (chunk->lineBytes.last() + 1 > chunkSize())
        chunk = addChunk();
    invalidateSize();
    if (mark) markLastLine();
    chunk->lineBytes << chunk->lineBytes.last()+1;
    chunk->bArray[chunk->lineBytes.last()-1] = '\n';
    updateChunkMetrics(chunk);

    mInstantRefresh = false;
    mLastLineIsOpen = false;
    mLastLineLen = 0;
    newPending(PendingContentChange);
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

    mLogStack.setLine();

    if (mNewLines)
        newPending(PendingContentChange);
    mInstantRefresh = true;
    mLastLineLen = 0;
}

void MemoryMapper::fetchLog()
{
    if (mLogStack.count())
        emit appendLines(mLogStack.popLines(), false);
}

void MemoryMapper::fetchDisplay()
{
    emit updateView();
    mNewLines = 0;
    mInstantRefresh = false;
}

void MemoryMapper::newPending(MemoryMapper::Pending pending)
{
    if (mPending == PendingNothing) {
        mDisplayCacheChanged.start();
    }
    mPending.setFlag(pending);

    if (mDisplayCacheChanged.elapsed() > CRefreshTimeMax) {
        processPending();
    } else if (!mPendingTimer.isActive() || mPendingTimer.remainingTime() < 10) {
        mPendingTimer.start(2*CRefreshTimeMax);
    }
}

void MemoryMapper::processPending()
{
    mPendingTimer.stop();
    if (mPending.testFlag(PendingBlockCountChange)) {
        emit blockCountChanged();
    }
    if (mPending.testFlag(PendingContentChange)) {
        fetchDisplay();
    }
    mPending = PendingNothing;
}

void MemoryMapper::addProcessLog(const QByteArray &data)
{
    Q_ASSERT_X(mChunks.size(), Q_FUNC_INFO, "Need to call startRun() before adding data.");
    Chunk *chunk = mChunks.last();
    int len = 0;
    QByteArray midData;
    bool cleaned = false;
    bool remote = data.startsWith("[]");
    int start = (remote ? 2 : 0);

    for (int i = start ; i < data.length() ; ++i) {
        // check for line breaks
        if (data.at(i) == '\r' || data.at(i) == '\n') {
            len = i-start;
            if (len) {
                midData.setRawData(data.data()+start, uint(len));
                ensureSpace(midData.size()+1);
                chunk = mChunks.last();
                appendLineData(midData, chunk, remote);
            }
            while (i < data.length() && (data.at(i) == '\r' || data.at(i) == '\n')) {
                if (data.at(i) == '\n') {
                    cleaned = ensureSpace(1);
                    chunk = mChunks.last();
                    appendEmptyLine(remote);
                }
                ++i;
            }
            if (i > 0 && data.at(i-1) == '\r' ) {
                clearLastLine();
                mInstantRefresh = true;
            }
            start = i;
        }
    }
    if (start < data.length()) {
        len = data.length()-start;
        if (len) {
            midData.setRawData(data.data()+start, uint(len));
            cleaned = ensureSpace(midData.size()+1);
            chunk = mChunks.last();
            appendLineData(midData, chunk, remote);
            mLastLineIsOpen = true;
        }
    }
    if (!cleaned) {
        updateChunkMetrics(chunk);
        recalcLineCount();
    }
}

void MemoryMapper::reset()
{
    while (!mChunks.isEmpty())
        removeChunk(mChunks.last()->nr);
    mUnits.clear();
    invalidateSize();
    mLineCount = 0;
    ChunkTextMapper::reset();
    addChunk(true);
    newPending(PendingBlockCountChange);
    newPending(PendingContentChange);
}

QString MemoryMapper::lines(int localLineNrFrom, int lineCount) const
{
    return ChunkTextMapper::lines(localLineNrFrom, lineCount);
}

QString MemoryMapper::lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const
{
    formats.reserve(lineCount);
    if (debugMode()) {
        localLineNrFrom /= 2;
    }
    int activationLine;
    QByteArray data = rawLines(localLineNrFrom, lineCount, mUnits.last().firstChunk->nr, activationLine);
    QVector<bool> markLines = markedLines(localLineNrFrom, lineCount);
    if (debugMode()) activationLine *= 2;

    QStringList res;
    LogParser::MarksBlockState mbState;
//    LineFormat *actErrFormat = nullptr;
    mbState.deep = false;
    QString rawLine;
    int from = 0;
    int to = 0;
    int next = -1;
    int i = 0;
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
            if (!mbState.switchLst.isEmpty()) {
                emit switchLst(mbState.switchLst);
                mbState.switchLst = QString();
            }
            if (debugMode()) {
                res << rawLine;
                formats << LineFormat(0, rawLine.length(), mBaseFormat.at(debug));
            }
            res << line;
            if (res.size() < activationLine) {
                formats << LineFormat(0, line.length(), mBaseFormat.at(old));
            } else if (mbState.marks.hasMark()) {
                if (mbState.marks.hasErr()) {
//                    QString toolTip = mbState.errData.text.isEmpty() ? mbState.marks.hRef : mbState.errData.text;
                    QString ref = mbState.marks.errRef.isEmpty() ? mbState.marks.hRef : mbState.marks.errRef;
                    formats << LineFormat(4, line.length(), mBaseFormat.at(error), ref);
                    if (!mbState.marks.hRef.isEmpty()) {
                        formats.last().extraLstFormat = &mBaseFormat.at(lstLink);
                        formats.last().extraLstHRef = mbState.marks.hRef;
                    }
//                    actErrFormat = &formats.last();
                } else if (mbState.marks.hRef.startsWith("FIL:")) {
                    formats << LineFormat(4, line.length(), mBaseFormat.at(fileLink), mbState.marks.hRef);
                } else if (mbState.marks.hRef.startsWith("LST:") || mbState.marks.hRef.startsWith("LS2:")) {
                    formats << LineFormat(4, line.length(), mBaseFormat.at(lstLink), mbState.marks.hRef);
                } else if (mbState.marks.hRef.startsWith("DIR:")) {
                    formats << LineFormat(4, line.length(), mBaseFormat.at(fileLink), mbState.marks.hRef);
                }
            } else if (hasError) {
                formats << LineFormat(0, line.length(),mBaseFormat.at(error));
            } else {
                formats << LineFormat();
            }
        }
        if (markLines.at(i))
            formats.last().lineMarked = true;

        from = next;
        to = next;
        next = -1;
        ++i;
    }

//    res << "";
    return res.join("\n");
}

void MemoryMapper::dump()
{
//    int iCh = 0;
    DEB() << "\n";
    DEB() << "---- size: " << mSize << "  lineCount: " << lineCount();
    for (Chunk *chunk : std::as_const(mChunks)) {
//        for (int lineNr = 0; lineNr < chunk->lineBytes.size()-1; ++lineNr) {
//            QString line;
//            for (int i = chunk->lineBytes.at(lineNr); i < chunk->lineBytes.at(lineNr+1); ++i) {
//                if (chunk->bArray.at(i) == '\r') line += "\\r";
//                else if (chunk->bArray.at(i) == '\n') line += "\\n";
//                else line += chunk->bArray.at(i);
//            }
//            DEB() << chunk->nr << " DATA: " << line;
//        }
        DEB() << chunk->nr << " from: " << chunk->bStart
              << " size: " << chunk->size()
              << "  lineCount " << chunk->lineCount() << "  metr:" << chunkMetrics(chunk->nr)->lineCount;
        int len = chunk->lineCount() ? chunk->lineBytes.at(1) - chunk->lineBytes.at(0) : 0;
        if (len)
            DEB() << "   starts with: " << chunk->bArray.mid(chunk->lineBytes.at(0), len);
    }
//    for (const Unit &u : mUnits) {
//        DEB() << "  UNIT: from " << u.firstChunk->nr << "+" << u.chunkCount-1 << "  size1: " << u.firstChunk->size();
//    }
//    dumpPos();
}

int MemoryMapper::knownLineNrs() const
{
    return lineCount();
}

QString MemoryMapper::extractLstRef(LineRef lineRef)
{
    if (!lineRef.chunk) return QString();
    int lastCh = lineRef.chunk->lineBytes.at(lineRef.relLine+1)-2;
    if (lastCh - lineRef.chunk->lineBytes.at(lineRef.relLine) < 7) return QString();
    if (lineRef.chunk->bArray.at(lastCh) != ']') return QString();
    int firstCh = lastCh-1;
    while (firstCh >= lineRef.chunk->lineBytes.at(lineRef.relLine) && lineRef.chunk->bArray.at(firstCh) != '[')
        --firstCh;
    QString res = lineRef.chunk->bArray.mid(firstCh+1, lastCh-firstCh-1);
    if (!res.startsWith("LST:") && !res.startsWith("LS2:")) return QString();
    return res;
}

QString MemoryMapper::findClosestLst(const int &localLine)
{
    LineRef backRef;
    // convert localLine to LineRef
    backRef.chunk = mChunks.at(topLine().chunkNr);
    backRef.relLine = topLine().localLine + localLine;
    while (backRef.chunk && backRef.relLine >= backRef.chunk->lineCount()) {
        backRef.relLine -= backRef.chunk->lineCount();
        backRef.chunk = nextChunk(backRef.chunk);
    }
    if (backRef.chunk && backRef.chunk->nr < mUnits.last().firstChunk->nr)
        return QString();
    LineRef foreRef = backRef;
    // take previous line while in error description (line starts with space)
    while(backRef.chunk &&
          backRef.chunk->bArray.at(backRef.chunk->lineBytes.at(backRef.relLine)) == ' ')
        backRef = prevRef(backRef);
    // take next line while in error description (line starts with space)
    while(foreRef.chunk &&
          foreRef.chunk->bArray.at(foreRef.chunk->lineBytes.at(foreRef.relLine)) == ' ')
        foreRef = nextRef(foreRef);

    // look for next lst-link in both directions
    QString href;
    int count = 0;
    while (backRef.chunk || foreRef.chunk) {
        href = extractLstRef(backRef);
        if (!href.isEmpty()) return href;
        href = extractLstRef(foreRef);
        if (!href.isEmpty()) return href;
        if (++count > 1000) break;
        if (backRef.chunk) backRef = prevRef(backRef);
        if (backRef.chunk && backRef.chunk->nr < mUnits.last().firstChunk->nr) backRef.chunk = nullptr;
        if (foreRef.chunk) foreRef = nextRef(foreRef);
    }
    return QString();
}

int MemoryMapper::chunkCount() const
{
    return mChunks.size();
}

//AbstractTextMapper::ChunkMetrics *MemoryMapper::chunkMetrics(int chunkNr) const
//{
//    ChunkMetrics
//}

void MemoryMapper::internalRemoveChunk(int chunkNr)
{
    Chunk *chunk = mChunks.at(chunkNr);
    // adjust the unit containing the removed chunk
    int delUnit = -1;
    for (int i = 0; i < mUnits.size() ; ++i) {
        Unit &u = mUnits[i];
        // chunk belongs to this unit
        if (u.firstChunk->nr <= chunkNr && u.firstChunk->nr + u.chunkCount > chunkNr) {
            if (u.chunkCount == 1)
                delUnit = i;
            else {
                if (u.firstChunk->nr == chunkNr)
                    u.firstChunk = getChunk(chunkNr+1);
                --u.chunkCount;
            }
        }
    }
    if (delUnit >= 0)
        mUnits.remove(delUnit);
    // remove chunk and adjust chunk-numbers
    mChunks.removeAt(chunkNr);
    for (int i = chunkNr; i < mChunks.size(); ++i) {
        --mChunks[i]->nr;
        mChunks[i]->bStart -= chunk->size();
    }
    delete chunk;
}

int MemoryMapper::lastChunkWithLineNr() const
{
    return mChunks.count()-1;
}

int MemoryMapper::visibleLineCount() const
{
    return ChunkTextMapper::visibleLineCount() / (debugMode() ? 2 : 1);
}

ChunkTextMapper::Chunk *MemoryMapper::getChunk(int chunkNr, bool cache) const
{
    Q_UNUSED(cache)
    if (chunkNr >= 0 && mChunks.size() > chunkNr)
        return mChunks.at(chunkNr);
    return nullptr;
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

MemoryMapper::LineRef MemoryMapper::prevRef(const MemoryMapper::LineRef &ref)
{
    if (!ref.chunk) return LineRef();
    LineRef res = ref;
    if (res.relLine > 0) {
        --res.relLine;
    } else {
        res.chunk = prevChunk(res.chunk);
        res.relLine = res.chunk ? res.chunk->lineCount()-1 : 0;
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
