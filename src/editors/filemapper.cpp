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
#include "filemapper.h"
#include "exception.h"
#include "logger.h"
#include <QFile>
#include <QGuiApplication>
#include <QClipboard>
#include <QtConcurrent>

namespace gams {
namespace studio {

static const int CMaxChunksInCache = 5;

FileMapper::FileMapper(QObject *parent): ChunkTextMapper(parent)
{
    mTimer.setInterval(200);
    mTimer.setSingleShot(true);
    connect(&mTimer, &QTimer::timeout, this, &FileMapper::closeFile);
    mPeekTimer.setSingleShot(true);
    connect(&mPeekTimer, &QTimer::timeout, this, &FileMapper::peekChunksForLineNrs);
    closeAndReset();
}

FileMapper::~FileMapper()
{
    closeFile();
}

bool FileMapper::openFile(const QString &fileName, bool initAnchor)
{
    if (!fileName.isEmpty()) {
        closeAndReset();
        if (initAnchor) setPosAbsolute(nullptr, 0, 0);
        QElapsedTimer et;
        et.start();
        mFile.setFileName(fileName);
        mSize = mFile.size();
        if (!mFile.open(QFile::ReadOnly)) {
            DEB() << "Could not open file " << fileName;
            return false;
        }
        int chunkCount = int(mFile.size()/chunkSize())+1;
        initChunkCount(chunkCount);
        Chunk *chunk = getChunk(0);
        if (chunk && chunk->isValid()) {
            emit blockCountChanged();
            if (initAnchor) initTopLine();
            updateMaxTop();
            mPeekTimer.start(100);
            return true;
        }
    }
    return false;
}

bool FileMapper::reload()
{
    QString fileName = mFile.fileName();
    if (!size() && !fileName.isEmpty()) {
        return openFile(fileName, false);
    }
    return size();
}

void FileMapper::closeAndReset()
{
    while (mChunkCache.size()) {
        chunkUncached(mChunkCache.takeFirst());
    }
    closeFile();
    mFile.setFileName(mFile.fileName()); // JM: Workaround for file kept locked (close wasn't enough)

    mSize = 0;
    ChunkTextMapper::reset();
    setPosAbsolute(nullptr, 0, 0);
    stopPeeking();
}

FileMapper::Chunk* FileMapper::getChunk(int chunkNr, bool cache) const
{
    // if the Chunk is cached, return it directly
    Chunk *res = getFromCache(chunkNr);
    if (res) return res;

    // determine start of the chunk
    qint64 chunkStart = qint64(chunkNr) * chunkSize();
    if (chunkStart < 0 || chunkStart >= size()) return nullptr;

    // read relevant part of file
    qint64 cStart = chunkStart - maxLineWidth();
    if (cStart < 0) cStart = 0;                                     // crop at start of file
    qint64 cEnd = chunkStart + chunkSize();
    if (cEnd > size()) cEnd = size();   // crop at end of file

    int bSize = 0;
    QByteArray bArray;
    if (!mFile.isOpen() && !mFile.open(QFile::ReadOnly)) {
        DEB() << "Could not open file " << mFile.fileName();
        return nullptr;
    } else {
        QMutexLocker locker(&mMutex);
        mFile.seek(cStart);
        bArray.resize(chunkSize()+maxLineWidth());
        bSize = int(mFile.read(bArray.data(), cEnd - cStart));
    }
    mTimer.start();

    // mapping succeeded: initialize chunk
    res = new Chunk();
    res->nr = chunkNr;
    res->bStart = cStart;
    res->bArray = bArray;

    // if delimiter isn't initialized
    if (delimiter().isEmpty()) initDelimiter(res);

    // create index for linebreaks
    if (!delimiter().isEmpty()) {
        int lines = res->bArray.count(delimiter().at(0));
        res->lineBytes.reserve(lines+2);
        res->lineBytes << 0;
        for (int i = 0; i < bSize; ++i) {
            if (res->bArray.at(i) == delimiter().at(0)) {
                if (res->bStart+i+delimiter().size() <= chunkStart) {
                    // first [lf] before chunkStart
                    res->lineBytes[0] = (i + delimiter().size());
                } else {
                    res->lineBytes << (i + delimiter().size());
                }
            }
        }
    }
    if (cEnd == size())
        res->lineBytes << (bSize + delimiter().size());

    updateLineOffsets(res);
    if (cache) {
        if (mChunkCache.size() == CMaxChunksInCache)
            chunkUncached(mChunkCache.takeFirst());
        mChunkCache << res;
    }
    return res;
}

void FileMapper::chunkUncached(ChunkTextMapper::Chunk *chunk) const
{
    if (!chunk) return;
    chunk->bArray.resize(0);
    chunk->bArray.squeeze();
    delete chunk;
}

void FileMapper::startRun()
{
    closeAndReset();
}

void FileMapper::endRun()
{
    reload();
}

void FileMapper::closeFile()
{
    QMutexLocker locker(&mMutex);
    mTimer.stop();
    if (mFile.isOpen()) {
        mFile.close();
    }
}

ChunkTextMapper::Chunk *FileMapper::getFromCache(int chunkNr) const
{
    int foundIndex = -1;
    for (int i = mChunkCache.size()-1; i >= 0; --i) {
        if (mChunkCache.at(i)->nr == chunkNr) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex < 0) return nullptr;
    if (foundIndex < mChunkCache.size()-1)
        mChunkCache.move(foundIndex, mChunkCache.size()-1);
    return mChunkCache.last();
}

int FileMapper::lineCount() const
{
    int res = 0;
    try {
        res = ChunkTextMapper::lineCount();
    } catch (Exception *) {
        EXCEPT() << "File too large " << mFile.fileName();
    }
    return int(res);
}

void FileMapper::peekChunksForLineNrs()
{
    // peek and keep timer alive if not done
    int known = lastChunkWithLineNr();
    for (int i = 1; i <= 4; ++i) {
        Chunk *chunk = getChunk(known + i, false);
        if (!chunk) break;
        if (!mChunkCache.contains(chunk))
            chunkUncached(chunk);
    }
    if (lastChunkWithLineNr() < this->chunkCount()-1) mPeekTimer.start(50);

    QVariant val = mPeekTimer.property("val");
    val = (val.isValid() && val.canConvert(QMetaType(QMetaType::Int))) ? ((val.toInt()+1) % 5) : 0;
    mPeekTimer.setProperty("val", val);
    emit loadAmountChanged(knownLineNrs());
    if (val.toInt() == 0 || knownLineNrs() == lineCount()) {
        emit blockCountChanged();
        emit selectionChanged();
    }
}

void FileMapper::reset()
{
//    while (!mChunkCache.isEmpty()) {
//        Chunk* chunk = mChunkCache.takeLast();
//        chunkUncached(chunk);
//    }
//    AbstractTextMapper::reset();
    closeAndReset();
}

void FileMapper::stopPeeking()
{
    mPeekTimer.stop();
    mPeekTimer.setProperty("val", 0);
    emit loadAmountChanged(knownLineNrs());
    emit blockCountChanged();
    emit selectionChanged();
}

QString FileMapper::fileName() const {
    return mFile.fileName();
}

} // namespace studio
} // namespace gams
