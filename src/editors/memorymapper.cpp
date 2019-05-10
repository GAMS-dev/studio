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
//#include "logparser.h"
#include "logger.h"

namespace gams {
namespace studio {

MemoryMapper::MemoryMapper(QObject *parent) : AbstractTextMapper (parent)
{
    startRun();
}

//void MemoryMapper::setLogParser(LogParser *parser)
//{
//    mLogParser = parser;
//}

void MemoryMapper::setLogFile(DynamicFile *logFile)
{
    mLogFile = logFile;
}

qint64 MemoryMapper::size() const
{
    return mSize;
}

AbstractTextMapper::Chunk *MemoryMapper::addChunk(bool startUnit)
{
    if (!mChunks.size() ||  mChunks.last()->size() > 0) {
        Chunk *chunk = new Chunk();
        chunk->bArray.resize(chunkSize());
        chunk->bStart = mChunks.size() ? mChunks.last()->bStart + mChunks.last()->size() : 0;
        chunk->lineBytes << 0;
        chunk->nr = chunkCount();
        mChunks << chunk;
        invalidateLineOffsets(chunk);
    }
    if (!mUnits.size() || startUnit) {
        mUnits << Unit(mChunks.last());
        mShrunk = false;
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

    if (!mShrunk) {
        mSize -= chunk->size();

        // the first chunk in the unit starts at bArray[0]
        while (chunk->size()+ellipsis.size() > chunkSize() && chunk->lineBytes.size() > 1) {
            chunk->lineBytes.removeLast();
        }
        // replace last line of first chunk by an ellipsis
        chunk->bArray.replace(chunk->lineBytes.last(), ellipsis.length(), ellipsis);
        chunk->lineBytes << (chunk->lineBytes.last()+1) << (chunk->lineBytes.last() + ellipsis.length()-1)
                             << (chunk->lineBytes.last() + ellipsis.length());

        mSize += chunk->size();
        invalidateLineOffsets(chunk);
        mShrunk = true;
    }

    // remove some lines at the start of the second active chunk
    chunk = mChunks[chunk->nr+1];
    mSize -= chunk->size();
    chunk->lineBytes.remove(0,(chunk->lineBytes.size() > 4 ? 4 : chunk->lineBytes.size()-1));
    mSize += chunk->size();

    // remove chunk if it is empty
    if (!chunk->size()) {
        mChunks.removeAt(chunk->nr);
        --mUnits.last().chunkCount;
        for (int i = chunk->nr; i < mChunks.size(); ++i)
            mChunks.at(i)->nr = i;
        chunk = mChunks.size() > chunk->nr ? mChunks.at(chunk->nr) : nullptr;
        invalidateLineOffsets(chunk, true);
    }

    // update internal data to new sizes
    while (chunk) {
        invalidateLineOffsets(chunk);
        chunk = mChunks.size() > chunk->nr+1 ? mChunks[chunk->nr+1] : nullptr;
    }
    emit blockCountChanged(lineCount());
}

bool MemoryMapper::setMappingSizes(int bufferedLines, int chunkSizeInBytes, int chunkOverlap)
{
    return AbstractTextMapper::setMappingSizes(bufferedLines, chunkSizeInBytes, chunkOverlap);
}

void MemoryMapper::startRun()
{
    addChunk(true);
}

void MemoryMapper::endRun()
{

}

// addProcessData appends to last chunk

// parse() triggers to parse on from last pos

void MemoryMapper::addProcessData(const QByteArray &data)
{
    // different line-endings ("\r\n", "\n", "\r") are unified to "\n"
    // remark: mac seems to use unix line-endings right now ("\n") - previously used "\r"
    int len = 0;
    int start = 0;
    bool lf = false;
    Q_ASSERT_X(mChunks.size(), Q_FUNC_INFO, "Need to call startRun() before adding data.");
    Chunk *chunk = mChunks.last();

    for (int i = 0 ; i < data.length() ; ++i) {

        // check for line breaks
        if (data.at(i) == '\r') {
            len = i-start;
            if (i+1 < data.size() && data.at(i+1) == '\n') {
                // normal line
                ++i;
                lf = true;
            } else if (len || data.length() == i+1) {
                // old mac style line-ending (seems not to occur any more)
                lf = true;
            } else {
                ++start; // skip the leading '\r'
                // conceal previous line if it hasn't been closed (\n at the end)
                if (chunk->lineBytes.last() > 0 && chunk->bArray.at(chunk->lineBytes.last()-1) != '\n') {
                    mSize -= chunk->size();
                    chunk->lineBytes.removeLast();
                    mSize += chunk->size();
                    --mParsed.relLine;
                }
            }
        } else if (data.at(i) == '\n') {
            len = i-start;
            lf = true;
        }

        // at end without line break?
        if (data.length() == i+1 && !lf) {
            len = i-start+1;
        }

        // line content or empty line found?
        if (len > 0 || lf) {
            QByteArray part;
            int lenPlusLf = len + (lf?1:0);
            // check if line fits into chunk
            int lastLineEnd = chunk->lineBytes.last();
            bool prevLineOpen = (lastLineEnd > 0) && (chunk->bArray.at(lastLineEnd-1) != '\n');
            if (lastLineEnd + lenPlusLf > chunkSize()) {
                Chunk *newChunk = addChunk();
                // if previous line hasn't been finished (\n) it has to be extended, move it to the new chunk
                if (prevLineOpen) {
                    int lastLineStart = chunk->lineBytes.at(chunk->lineCount()-1);
                    part.setRawData(chunk->bArray.data() + lastLineStart, uint(lastLineEnd-lastLineStart));
                    newChunk->bArray.replace(0, lastLineEnd-lastLineStart, part);
                    chunk->lineBytes.removeLast();
                    newChunk->lineBytes << (lastLineEnd-lastLineStart);
                    --mParsed.relLine; // ensure reparsing the line
                }
                chunk = newChunk;
                lastLineEnd = chunk->lineBytes.last();
            }
            part.setRawData(data.data() + start, uint(len));
            chunk->bArray.replace(lastLineEnd, len, part);
            if (lf) chunk->bArray[lastLineEnd+len] = '\n';
            if (prevLineOpen) {
                // new line replaced previous one
                chunk->lineBytes.last() = (lastLineEnd+lenPlusLf);
                if (mParsed.chunkNr == mChunks.size()-1 && mParsed.relLine == chunk->lineBytes.size()-1)
                    --mParsed.relLine; // ensure reparsing the line
            } else {
                // new line was appended
                chunk->lineBytes << (lastLineEnd+lenPlusLf);
            }
            mSize += lenPlusLf;
            start = i+1;
            len = 0;
            lf = false;
        }
    }
    if (mSize - mUnits.last().firstChunk->bStart > chunkSize() * 2)
        shrinkLog();
    else
        invalidateLineOffsets(chunk);
    emit blockCountChanged(lineCount());
}

QString MemoryMapper::lines(int localLineNrFrom, int lineCount) const
{
    return AbstractTextMapper::lines(localLineNrFrom, lineCount);
    QByteArray data;
//    LogParser::ExtractionState state = LogParser::Outside;

    bool conceal = false;
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
                conceal = true;
            } else
                next = to+2;
        }
        if (next < 0) {
            ++to;
            continue;
        }
        int len = to-from;
        QString line;
        if (len > 0) {
            bool hasError = false;
            QByteArray lineData = data.mid(from, len);
//            QStringList lines = mLogParser->parseLine(lineData, state, hasError, nullptr); // 1 line (2 lines on debugging)

//            QTextCursor cursor(document());
//            cursor.movePosition(QTextCursor::End);
//            if (mConceal && !line.isNull()) {
//                cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
//                cursor.removeSelectedText();
//            }
//            if (lines.size() > 1) {
//                QTextCharFormat fmtk;
//                fmtk.setForeground(QColor(120,150,100));
//                cursor.insertText(lines.first(), fmtk);
//                QTextCharFormat fmt;
//                cursor.insertText("\n", fmt);
//            }
//            cursor.insertText(lines.last()+"\n");
//            if (mLogFile) mLogFile->appendLine(line);
//            if (mJumpToLogEnd) {
//                mJumpToLogEnd = false;
//                verticalScrollBar()->setValue(verticalScrollBar()->maximum());
//            }
        }
//        document()->setModified(false);

        from = next;
        to = next;
        conceal = false;
    }
    return QString();
}

void MemoryMapper::dump()
{
    int iCh = 0;
    DEB() << "\n";
    DEB() << "---- size: " << mSize ;
    int sum = 0;
    for (Chunk *chunk : mChunks) {
        for (int lineNr = 0; lineNr < chunk->lineBytes.size()-1; ++lineNr) {
            QString line;
            for (int i = chunk->lineBytes.at(lineNr); i < chunk->lineBytes.at(lineNr+1); ++i) {
                if (chunk->bArray.at(i) == '\r') line += "\\r";
                else if (chunk->bArray.at(i) == '\n') line += "\\n";
                else line += chunk->bArray.at(i);
            }
            DEB() << iCh << " DATA: " << line;
        }
        DEB() << iCh << " size: " << chunk->size();
        sum += chunk->size();
        ++iCh;
    }
    for (const Unit &u : mUnits) {
        DEB() << "  UNIT: from " << u.firstChunk->nr << "  size " << u.chunkCount;
    }
}

void MemoryMapper::setJumpToLogEnd(bool state)
{

}

void MemoryMapper::repaint()
{

}

int MemoryMapper::chunkCount() const
{
    return mChunks.size();
}

AbstractTextMapper::Chunk *MemoryMapper::getChunk(int chunkNr) const
{
    return mChunks.at(chunkNr);
}

QByteArray MemoryMapper::popNextLine()
{
    if (!mChunks.size()) return QByteArray();

    if (mParsed.chunkNr >= 0) {
        if (mParsed.relLine < mChunks.at(mParsed.chunkNr)->lineCount()-1) {
            ++mParsed.relLine;
        } else if (mParsed.chunkNr < mChunks.size()-1) {
            ++mParsed.chunkNr;
            mParsed.relLine = 0;
        } else {
            return QByteArray();
        }
    } else {
        mParsed.chunkNr = 0;
        mParsed.relLine = 0;
    }
    Chunk *chunk = mChunks.at(mParsed.chunkNr);
    int byteFrom = chunk->lineBytes.at(mParsed.relLine);
    int byteTo = chunk->lineBytes.at(mParsed.relLine+1);
    while ((chunk->bArray.at(byteTo) == '\n' || chunk->bArray.at(byteTo) == '\r') && byteTo > byteFrom)
        --byteTo;
    return chunk->bArray.mid(byteFrom, byteTo - byteFrom);
}

bool MemoryMapper::parseRemain()
{
    bool res = false;
    while (true) {
        QByteArray data = popNextLine();
        if (data.isNull()) break;
        res = true;
//        LogParser::ExtractionState state = LogParser::Outside;
        bool hasError = false;
//        QStringList lines = mLogParser->parseLine(data, state, hasError, mState); // 1 line (2 lines on debugging)

        // TODO(JM) extract textMarks from mState

    }
    return res;
}

} // namespace studio
} // namespace gams
