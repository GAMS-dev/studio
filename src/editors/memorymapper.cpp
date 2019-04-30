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
#include "exception.h"
#include "locators/settingslocator.h"
#include "studiosettings.h"

namespace gams {
namespace studio {

static int cActiveChunks = 2;

MemoryMapper::MemoryMapper(QObject *parent) : AbstractTextMapper (parent)
{
    moveToRecent();
}

void MemoryMapper::setLogParser(LogParser *parser)
{
    mLogParser = parser;
}

void MemoryMapper::setLogFile(DynamicFile *logFile)
{
    mLogFile = logFile;
}

qint64 MemoryMapper::size() const
{
    return 0;
}

void MemoryMapper::moveToRecent()
{
    if (mChunks.size()) {
        mRecent << Recent(mChunks.size()-1);
    }
    // prepare new section
}

AbstractTextMapper::Chunk *MemoryMapper::addChunk()
{
    Chunk *chunk = new Chunk();
    chunk->bArray.resize(chunkSize());
    chunk->start = mChunks.size() ? mChunks.last()->start + mChunks.last()->size : 0;
    chunk->nr = chunkCount();
    mChunks << chunk;

    int lastInactive = mRecent.size() ? mRecent.last().index : 0;
    if (mChunks.size() - lastInactive > cActiveChunks) {

        // TODO(JM) handle 2nd chunk if too many active chunks are present

        // when deleting middle chunk:
        //    a. mark last line of first active chunk with "..."
        //    b. append new data at last chunk
        //    c. reduce second active chunk

    }

    return chunk;
}

bool MemoryMapper::setMappingSizes(int bufferedLines, int chunkSizeInBytes, int chunkOverlap)
{
    return AbstractTextMapper::setMappingSizes(bufferedLines, chunkSizeInBytes, chunkOverlap);
}

void MemoryMapper::startRun()
{
    moveToRecent();
}

void MemoryMapper::endRun()
{ }

QString MemoryMapper::lines(int localLineNrFrom, int lineCount) const
{
    QByteArray data;
    LogParser::ExtractionState state = LogParser::Outside;

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
            QStringList lines = mLogParser->parseLine(lineData, state, hasError, nullptr); // 1 line (2 lines on debugging)

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
}

// addProcessData appends last chunk

// parse() triggers to parse on from last pos

void MemoryMapper::addProcessData(const QByteArray &data)
{
    bool conceal = false;
    int from = 0;
    int to = 0;
    int next = -1;
    while (to < data.length()) {
        // get kind of line-break (concealing / single(linux) / pair(win) / last-char)
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
        int len = next - from;
        Chunk *chunk = mChunks.last();
        int pos = conceal ? chunk->lineBytes.at(lineCount()) : chunk->lineBytes.last();
        if (pos + len > chunkSize()) {
            if (conceal) {
                chunk->lineBytes.removeLast();
                chunk->size = mConcealPos+1;
                --mParsed.relLine;
            }
            chunk = addChunk();
            pos = 0;
        }

        QByteArray lineData;
        lineData.setRawData(data.data()+from, static_cast<uint>(len));
        chunk->bArray.replace(pos, lineData.length(), lineData);
        chunk->lineBytes << pos + lineData.length();

        from = next;
        to = next;
        conceal = false;
    }
    parseRemain();
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
    // if no chunks OR all chunks belong to recent runs -> skip
    if (!mChunks.size() || mRecent.last().index == mChunks.size()-1)
        return QByteArray();

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
        LogParser::ExtractionState state = LogParser::Outside;
        bool hasError = false;
        QStringList lines = mLogParser->parseLine(data, state, hasError, mState); // 1 line (2 lines on debugging)

        // TODO(JM) extract textMarks from mState

    }
    return res;
}

} // namespace studio
} // namespace gams
