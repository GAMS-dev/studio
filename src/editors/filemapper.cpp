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
#include "filemapper.h"
#include "exception.h"
#include "logger.h"
#include <QFile>
#include <QTextStream>
#include <QGuiApplication>
#include <QClipboard>

namespace gams {
namespace studio {

FileMapper::FileMapper(QObject *parent): AbstractTextMapper(parent)
{
    mTimer.setInterval(200);
    mTimer.setSingleShot(true);
    connect(&mTimer, &QTimer::timeout, this, &FileMapper::closeFile);
    mPeekTimer.setSingleShot(true);
    connect(&mPeekTimer, &QTimer::timeout, this, &FileMapper::peekChunksForLineNrs);
    closeAndReset(true);
}

FileMapper::~FileMapper()
{
    closeFile();
}

bool FileMapper::openFile(const QString &fileName, bool initAnchor)
{
    if (!fileName.isEmpty()) {
        closeAndReset(initAnchor);
        mFile.setFileName(fileName);
        if (!mFile.open(QFile::ReadOnly)) {
            DEB() << "Could not open file " << fileName;
            return false;
        }
        mSize = mFile.size();
        int chunkCount = int(mFile.size()/chunkSize())+1;
        chunkLineNrs().reserve(chunkCount);
        for (int i = chunkLineNrs().size(); i < chunkCount; ++i) {
            // initialize elements
            chunkLineNrs() << ChunkLines(i);
        }
        Chunk *chunk = getChunk(0);
        if (chunk && chunk->isValid()) {
            if (initAnchor) initTopLine();
            updateMaxTop();
            mPeekTimer.start(100);
            return true;
        }
    }
    return false;
}

bool FileMapper::reopenFile()
{
    QString fileName = mFile.fileName();
    if (!size() && !fileName.isEmpty()) {
        return openFile(fileName, false);
    }
    return size();
}

void FileMapper::closeAndReset(bool initAnchor)
{
    for (Chunk *block: chunks()) {
        mFile.unmap(block->map);
    }
    closeFile();
    mSize = 0;
    AbstractTextMapper::closeAndReset(initAnchor);
}


FileMapper::Chunk *FileMapper::getChunk(int chunkNr) const
{
    int foundIndex = -1;
    for (int i = chunks().size()-1; i >= 0; --i) {
        if (chunks().at(i)->nr == chunkNr) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex < 0) {
        if (chunks().size() == maxChunks()) {
            Chunk * delChunk = chunks().takeFirst();
            mFile.unmap(delChunk->map);
            delete delChunk;
        }
        Chunk *newChunk = loadChunk(chunkNr);
        if (!newChunk) return nullptr;
        chunks() << newChunk;

    } else if (foundIndex < chunks().size()-1) {
        chunks().move(foundIndex, chunks().size()-1);
    }
    return chunks().last();
}


FileMapper::Chunk* FileMapper::loadChunk(int chunkNr) const
{
    qint64 chunkStart = qint64(chunkNr) * chunkSize();
    if (chunkStart < 0 || chunkStart >= size()) return nullptr;

    // map file
    qint64 cStart = chunkStart - maxLineWidth();
    if (cStart < 0) cStart = 0;                                     // crop at start of file
    qint64 cEnd = chunkStart + chunkSize();
    if (cEnd > size()) cEnd = size();   // crop at end of file
    uchar* map = nullptr;
    {
        QMutexLocker locker(&mMutex);
        if (mFile.isOpen() || mFile.open(QFile::ReadOnly)) {
            map = mFile.map(cStart, cEnd - cStart);
            mTimer.start();
        } else {
            DEB() << "Could not open file " << mFile.fileName();
        }
    }
    if (!map) return nullptr;

    // mapping succeeded: initialise chunk
    Chunk *res = new Chunk();
    res->nr = chunkNr;
    res->map = map;
    res->start = cStart;
    res->size = int(cEnd - cStart);
    res->bArray.setRawData(reinterpret_cast<char*>(res->map), uint(res->size));

    // if delimiter isn't initialized
    if (delimiter().isEmpty()) initDelimiter(res);

    // create index for linebreaks
    if (!delimiter().isEmpty()) {
        int lines = res->bArray.count(delimiter().at(0));
        res->lineBytes.reserve(lines+2);
        res->lineBytes << 0;
        for (int i = 0; i < res->size; ++i) {
            if (res->bArray.at(i) == delimiter().at(0)) {
                if (res->start+i+delimiter().size() <= chunkStart) {
                    // first [lf] before chunkStart
                    res->lineBytes[0] = (i + delimiter().size());
                } else {
                    res->lineBytes << (i + delimiter().size());
                }
            }
        }
    }
    if (res->start + res->size == size())
        res->lineBytes << (res->size + delimiter().size());

    updateLineOffsets(res);
    return res;
}

void FileMapper::closeFile()
{
    QMutexLocker locker(&mMutex);
    mTimer.stop();
    if (mFile.isOpen()) mFile.close();
}

int FileMapper::lineCount() const
{
    int res = 0;
    try {
        res = AbstractTextMapper::lineCount();
    } catch (Exception *) {
        EXCEPT() << "File too large " << mFile.fileName();
    }
    return int(res);
}

void FileMapper::deleteChunkIfUnused(Chunk *&chunk)
{
    if (!isMapped(chunk)) {
        mFile.unmap(chunk->map);
        delete chunk;
        chunk = nullptr;
    }
}

void FileMapper::peekChunksForLineNrs()
{
    // peek and keep timer alive if not done
    int known = lastChunkWithLineNr();
    Chunk *chunk = nullptr;
    for (int i = 1; i <= 4; ++i) {
        chunk = loadChunk(known + i);
        if (!chunk) break;
        deleteChunkIfUnused(chunk);
    }
    if (lastChunkWithLineNr() < this->chunkCount()-1) mPeekTimer.start(50);

    QVariant val = mPeekTimer.property("val");
    val = (val.isValid() && val.canConvert(QMetaType::Int)) ? ((val.toInt()+1) % 5) : 0;
    mPeekTimer.setProperty("val", val);
    emit loadAmountChanged(knownLineNrs());
    if (val.toInt() == 0 || knownLineNrs() == lineCount()) {
        emit blockCountChanged(lineCount());
        emit selectionChanged();
    }
}

} // namespace studio
} // namespace gams
