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
#include "fastfilemapper.h"
#include "logger.h"
#include "exception.h"

#include <QRegularExpression>
#include <QElapsedTimer>
#include <QtConcurrent>

namespace gams {
namespace studio {

const int CMaxCharsPerBlock = 1024*1024*256;

FastFileMapper::FastFileMapper(QObject *parent)
    : gams::studio::AbstractTextMapper(parent)
    , mCache(this)
{}

bool FastFileMapper::openFile(const QString &fileName, bool initAnchor)
{
    if (!fileName.isEmpty()) {
        closeAndReset();

        if (initAnchor) {
            mPosition = QPoint(0,-1);
            mAnchor = QPoint(0,-1);
        }

        mFile.setFileName(fileName);
        mSize = mFile.size();
        if (!mFile.open(QFile::ReadOnly)) {
            DEB() << "Could not open file " << fileName;
            return false;
        }
        mInterruptThreads = false;
        mLoading = QtConcurrent::run(&FastFileMapper::scanLF, this);
        updateMaxTop();
        setVisibleTopLine(0);
        emit loadAmountChanged(knownLineNrs());
        emit blockCountChanged();
        return true;
    }
    return false;
}

QString FastFileMapper::fileName() const
{
    return mFile.fileName();
}

qint64 FastFileMapper::size() const
{
    return mSize;
}

AbstractTextMapper::Kind FastFileMapper::kind() const
{
    return AbstractTextMapper::fileMapper;
}

void FastFileMapper::startRun()
{
    closeAndReset();
}

void FastFileMapper::endRun()
{
    reload();
}

bool FastFileMapper::setVisibleTopLine(double region)
{
    return setVisibleTopLine(int(region * lineCount()));
}

bool FastFileMapper::setVisibleTopLine(int lineNr)
{
    int validLineNr = qMax(0, qMin(lineNr, lineCount() - visibleLineCount()));
    mVisibleTopLine = validLineNr;
    return lineNr == validLineNr;
}

int FastFileMapper::moveVisibleTopLine(int lineDelta)
{
    int tl = visibleTopLine();
    setVisibleTopLine(tl + lineDelta);
    return tl + lineDelta - visibleTopLine();
}

int FastFileMapper::visibleTopLine() const
{
    return mVisibleTopLine;
}

void FastFileMapper::scrollToPosition()
{
    int localLine = position(true).y();
    if (localLine < visibleLineCount() / 5 || localLine == cursorBeyondEnd || localLine > (visibleLineCount() * 4) / 5)
        setVisibleTopLine(position().y() - visibleLineCount() / 2);
}

int FastFileMapper::lineCount() const
{
    QMutexLocker locker(&mMutex);
    qint64 count = 0;
    if (mLoadAmount >= 1.) {
        count = mLineByte.count() - 1;  // counted
    } else {
        count = -mLineByte.count() - 1 - (qint64(mLineByte.count() - 1) * (1 - mLoadAmount)); // estimated
    }
    if (count >= std::numeric_limits<int>::max() || count <= std::numeric_limits<int>::min())
        EXCEPT() << "Textdata contains too many lines";
    return int(count);
}

int FastFileMapper::knownLineNrs() const
{
    QMutexLocker locker(&mMutex);
    return mLineByte.count() - 1;
}

void FastFileMapper::waitForCountThread()
{
    mLoading.waitForFinished();
}

QString FastFileMapper::lines(int localLineNrFrom, int lineCount) const
{
    int lineNr = visibleTopLine() + localLineNrFrom;
    adjustLines(lineNr, lineCount);
    return mCache.getLines(lineNr, lineCount);
}

QString FastFileMapper::lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const
{
    int lineNr = visibleTopLine() + localLineNrFrom;
    adjustLines(lineNr, lineCount);
    QString res = mCache.getLines(lineNr, lineCount);
    if (!lineMarkers().isEmpty()) {
        int absTopLine = visibleTopLine();
        if (absTopLine >= 0) {
            QTextCharFormat fmt;
            fmt.setBackground(toColor(Theme::Edit_currentLineBg));
            fmt.setProperty(QTextFormat::FullWidthSelection, true);
            LineFormat markedLine(-1, -1, fmt);
            markedLine.lineMarked = true;
            formats.reserve(lineCount);
            for (int i = absTopLine; i < absTopLine + lineCount; ++i) {
                if (lineMarkers().contains(i))
                    formats << markedLine;
                else
                    formats << LineFormat();
            }
        }
    }
    return res;
}

bool FastFileMapper::findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind)
{
    int tempLineCount = lineCount();
    bool backwards = flags.testFlag(QTextDocument::FindBackward);

    // TODO(JM) Make this size-dependant instead of line-numbers
    int liSpan = 500; // maximal lines per call
    if (!continueFind) {
        mSearchPos = mPosition;
        if (hasSelection()) {
            if (backwards && isBefore(mAnchor, mPosition)) mSearchPos = mAnchor;
            if (!backwards && isBefore(mPosition, mAnchor)) mSearchPos = mAnchor;
            liSpan = qMin(liSpan, qAbs(mAnchor.y() - mPosition.y()));
        }
        mSearchEndPos = (mSearchPos == mPosition) ? mAnchor : mPosition;
    }
    if (backwards) {
        if (mSearchPos.y() - liSpan < 0)
            liSpan = mSearchPos.y();
        liSpan = -liSpan;
    } else {
        if (mSearchPos.y() + liSpan > tempLineCount)
            liSpan = tempLineCount - mSearchPos.y();
    }
    if (continueFind &&
        qMin(mSearchPos.y(), mSearchPos.y() + liSpan) < mSearchEndPos.y() &&
        qMax(mSearchPos.y(), mSearchPos.y() + liSpan) > mSearchEndPos.y())
        liSpan = mSearchEndPos.y() - mSearchPos.y();

    QRegularExpressionMatch match;
    const QString data = mCache.loadCache(mSearchPos.y(), liSpan);
    int index = backwards ? data.lastIndexOf(searchRegex, mSearchPos.x() - mCache.lineLength(mSearchPos.y()), &match)
                          : data.indexOf(searchRegex, mSearchPos.x(), &match);
    if (index >= 0) {
        QPoint pos = mCache.posForOffset(index);
        if (pos.y() != mSearchEndPos.y() || (backwards ? pos.x() < mSearchEndPos.x()
                                                       : pos.x() > mSearchEndPos.x() + match.capturedLength())) {
            mAnchor = pos;
            mPosition = mAnchor;
            mPosition.rx() += match.capturedLength();
            scrollToPosition();
            continueFind = false;
            mCache.reset();
            return true;
        }
    }

    if (mSearchEndPos.y() == (backwards ? mCache.firstCacheLine() : mCache.lastCacheLine())) {
        continueFind = false;
    } else {
        mSearchPos.setY(qMin(qMax(mSearchPos.y() + liSpan, 0), tempLineCount));
        mSearchPos.setX(0);
    }
    mCache.reset();
    return false;
}

QString FastFileMapper::selectedText() const
{
    if (mAnchor == mPosition || mPosition.y() <= AbstractTextMapper::cursorInvalid) return QString();
    PosAncState pas = posAncState();
    QPoint p1 = pas == PosBeforeAnc ? mPosition : mAnchor;
    QPoint p2 = pas == PosBeforeAnc ? mAnchor : mPosition;
    QString res = mCache.getLines(p1.y(), p2.y() - p1.y() + 1);
    int from = p1.x();
    int len = res.length() - from - mCache.lineLength(p2.y()) + p2.x();
    return res.sliced(from, len);
}

QString FastFileMapper::positionLine() const
{
    if (!lineCount())
        return QString();
    return mCache.getLines(mPosition.y(), 1);
}

void FastFileMapper::setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode)
{
    bool toEnd = (localLineNr >= 0) && (charNr == -1);
    bool keepCol = (charNr == -2);
    if (toEnd) --localLineNr;
    int absLine = qMax(0, qMin(visibleTopLine() + localLineNr, lineCount()-1));
    if (keepCol) charNr = mCursorColumn;
    charNr = toEnd ? mCache.lineLength(absLine)
                   : qMin(mCache.lineLength(absLine), qMax(0, charNr));
    if (!keepCol) mCursorColumn = charNr;
    mPosition = QPoint(charNr, absLine);
    if (mode == QTextCursor::MoveAnchor)
        mAnchor = mPosition;
}

void FastFileMapper::setPosToAbsStart(QTextCursor::MoveMode mode)
{
    mCursorColumn = 0;
    mPosition = QPoint();
    if (mode == QTextCursor::MoveAnchor)
        mAnchor = mPosition;
}

void FastFileMapper::setPosToAbsEnd(QTextCursor::MoveMode mode)
{
    mPosition = endPosition();
    mCursorColumn = mPosition.x();
    if (mode == QTextCursor::MoveAnchor)
        mAnchor = mPosition;
}

void FastFileMapper::selectAll()
{
    mAnchor = QPoint();
    mPosition = endPosition();
}

void FastFileMapper::clearSelection()
{
    if (mAnchor != mPosition)
        mAnchor = mPosition;
    else {
        mPosition = QPoint(0,-1);
        mAnchor = QPoint(0,-1);
    }
}

QPoint FastFileMapper::position(bool local) const
{
    if (!local)
        return mPosition;
    int localLine = mPosition.y() - visibleTopLine();
    if (localLine < 0) return QPoint(0, cursorBeforeStart);
    if (localLine > visibleLineCount()) return QPoint(0, cursorBeyondEnd);
    return QPoint(mPosition.x(), localLine);
}

QPoint FastFileMapper::anchor(bool local) const
{
    if (!local)
        return mAnchor;
    int localLine = mAnchor.y() - visibleTopLine();
    if (localLine < 0) return QPoint(0, cursorBeforeStart);
    if (localLine > visibleLineCount()) return QPoint(0, cursorBeyondEnd);
    return QPoint(mAnchor.x(), mAnchor.y() - visibleTopLine());
}

bool FastFileMapper::hasSelection() const
{
    return mPosition != mAnchor;
}

int FastFileMapper::selectionSize() const
{
    int lines = lineCount();
    if (mAnchor == mPosition || lines == 0) return 0;
    QPoint pFrom = mAnchor;
    QPoint pTo = mPosition;
    if (pFrom.y() < 0)
        pFrom = QPoint(0, 0);
    QMutexLocker locker(&mMutex);
    if (pFrom.y() >= lines)
        pFrom = QPoint(int(mLineByte.last() - mLineByte.at(lines) - delimiter().size()), lines - 1);

    if (pTo.y() < 0)
        pTo = QPoint(0, 0);
    if (pTo.y() >= lines)
        pTo = QPoint(int(mLineByte.last() - mLineByte.at(lines) - delimiter().size()), lines - 1);

    qint64 from = mLineByte.at(pFrom.y()) + pFrom.x();
    qint64 to = mLineByte.at(pTo.y()) + pTo.x();

    // Set a limit of maxint / 20 to protect the clipboard
    // To avoid time-consuming double codec-conversion: estimate with factor *2 (1 Byte in storage is 2 Bytes in Memory)
    int res =  qAbs(to - from) * 2;
    if (res > INT_MAX / 20) {
        DEB() << "Selection too big for clipboard";
        return 0;
    }
    return res;
}

bool FastFileMapper::atTail()
{
    int lines = lineCount();
    return lines >= 0 && mVisibleTopLine + visibleLineCount() >= lines;
}

void FastFileMapper::updateSearchSelection()
{
    if (posAncState() == PosBeforeAnc) {
        mSearchSelectionStart = mPosition;
        mSearchSelectionEnd = mAnchor;
    } else {
        mSearchSelectionStart = mAnchor;
        mSearchSelectionEnd = mPosition;
    }
    setSearchSelectionActive(mSearchSelectionStart != mSearchSelectionEnd);
}

void FastFileMapper::clearSearchSelection()
{
    mSearchSelectionStart = QPoint();
    mSearchSelectionEnd = QPoint();
    setSearchSelectionActive(false);
}

QPoint FastFileMapper::searchSelectionStart()
{
    return mSearchSelectionStart;
}

QPoint FastFileMapper::searchSelectionEnd()
{
    return mSearchSelectionEnd;
}

void FastFileMapper::dumpPos() const
{
    QMutexLocker locker(&mMutex);
    if (mAnchor.y() < 0)
        DEB() << "anc: " << mAnchor;
    else
        DEB() << "anc: " << mAnchor << ",  p " << (mLineByte.at(mAnchor.y()) + mAnchor.x());
    if (mPosition.y() < 0)
        DEB() << "anc: " << mPosition << "  - keepcol: " << mCursorColumn;
    else
        DEB() << "pos: " << mPosition << ",  p " << (mLineByte.at(mPosition.y()) + mPosition.x()) << "  - keepcol: " << mCursorColumn;
    DEB() << "top: " << mVisibleTopLine;
    DEB() << "max: " << visibleLineCount();
}

qint64 FastFileMapper::checkField(FastFileMapper::Field field) const
{
    switch (field) {
    case fVirtualLastLineEnd: {
        QMutexLocker locker(&mMutex);
        return mLineByte.size() ? mLineByte.last() : -1;
    }
    case fCacheFirst:
        return mCache.firstCacheLine();
    case fCacheLast:
        return mCache.lastCacheLine();
    case fPosLineStartInFile:
        if (mCache.linePos(mPosition.y()) < 0)
            return -1;
        else {
            QMutexLocker locker(&mMutex);
            return mCache.linePos(mPosition.y()) + mLineByte.at(mCache.firstCacheLine());
        }
    }
    return 0;
}

void FastFileMapper::reset()
{
    AbstractTextMapper::reset();
    mPosition = QPoint(0,-1);
    mAnchor = QPoint(0,-1);
    mVisibleTopLine = 0;
    {
        QMutexLocker locker(&mMutex);
        mInterruptThreads = true;
    }
    mLoading.waitForFinished();

    QMutexLocker locker(&mMutex);
    mCache.reset();
    mLineByte.clear();
    mLoadAmount = 0.;
    mBytesPerLine = 50;
    mInterruptThreads = false;
}

void FastFileMapper::closeFile()
{
    QMutexLocker locker(&mMutex);
    if (mFile.isOpen()) {
        mFile.close();
    }
}

void FastFileMapper::closeAndReset()
{
    closeFile();
    mFile.setFileName(mFile.fileName()); // JM: Workaround for file kept locked (close wasn't enough)
    mSize = 0;
    reset();
}


QByteArray scanDelim(char*data, int len)
{
    bool hasCR = false;
    // int zeros = 0;                                  // can be used to detect UTF-16 (zeros==1) or UTF-32 (zeros==3)
    for (int i = 0; i < len; ++i) {
        if (data[i] == '\r')
            hasCR = true;
        else if (data[i] == '\n')
            return QByteArray(hasCR ? "\r\n" : "\n");
        else if (data[i] == '\0')                      // skip zeros (UTF-16, UTF-32, or similar unicode encodings)
            /*++zeros*/;
        else {
            hasCR = false;
            // zeros = 0;
        }
    }
    return QByteArray();
}

QList<qint64> subScanLF(char*data, int len, qint64 offset)
{
    QList<qint64> res;
    res.reserve(len / 45);
    for (int i = 0; i < len; ++i) {
        if (data[i] == '\n')
            res.append(offset + i + 1);
    }
    return res;
}

bool FastFileMapper::scanLF()
{
    // TODO(JM) This is fast enough on SSD but should be threadded for non-blocking function on HDD
    const int threadMax = 16;
    setDelimiter("");
    bool hasDelim = false;
    QFile file(mFile.fileName());
    if (!file.open(QFile::ReadOnly)) {
        DEB() << "Couldn't open file for parsing: " << file.fileName();
        return false;
    }
    {
        QMutexLocker locker(&mMutex);
        mLineByte.clear();
        mLineByte.reserve(file.size() / 45);
        mLineByte << 0.;
    }
    QDataStream ds(&file);
    int len = 1024*512;
    char *data[threadMax];
    QList<qint64> dataOffsets;
    dataOffsets.reserve(threadMax);
    for (int i = 0; i < threadMax; ++i) {
        data[i] = new char[len];
        dataOffsets << 0;
    }
    qint64 start = 0;

    QList< QFuture< QList<qint64> > > fut;
    int threadCount = 0;
    while (!ds.atEnd()) {
        int len1 = ds.readRawData(data[threadCount], len);

        if (!hasDelim) {
            QByteArray delim = scanDelim(data[threadCount], len1);
            setDelimiter(delim.isEmpty() ? QByteArray("\n") : delim);
            hasDelim = true;
        }

        qint64 offset = start;
        dataOffsets.replace(threadCount, offset);
        fut << QtConcurrent::run(subScanLF, data[threadCount], len1, offset);
        start += len1;
        ++threadCount;
        if (threadCount >= threadMax || ds.atEnd())  {
            for (int i = 0; i < fut.count(); ++i) {
                QMutexLocker locker(&mMutex);
                mLineByte << fut.at(i).result();
            }
            fut.clear();
            threadCount = 0;
            QMutexLocker locker(&mMutex);
            mLoadAmount = qreal(start) / qreal(size());
            emit loadAmountChanged(mLineByte.count() - 1);
            if (mInterruptThreads)
                break;
        }
    }
    for (int i = 0; i < threadMax; ++i)
        delete data[i];

    QMutexLocker locker(&mMutex);
    mLineByte << file.size() + delimiter().size();
    mLineByte.squeeze();
    mLoadAmount = 1.;
    file.close();
    return true;
}

QPoint FastFileMapper::endPosition()
{
    int lines = lineCount();
    if (!lines) return QPoint();
    return QPoint(mCache.lineLength(lines-1), lines-1);
}

bool FastFileMapper::adjustLines(int &lineNr, int &count) const
{
    int lines = qAbs(lineCount());
    int fromLine = qMax(0, qMin(lineNr, lines-1));
    int toLine = qBound(0, count + lineNr, lines);
    if (fromLine == lineNr && count == toLine - fromLine)
        return true;
    lineNr = fromLine;
    count = toLine - fromLine;
    return false;
}

bool FastFileMapper::reload()
{
    QString fileName = mFile.fileName();
    if (!size() && !fileName.isEmpty()) {
        return openFile(fileName, false);
    }
    return size();
}

FastFileMapper::PosAncState FastFileMapper::posAncState() const
{
    if (mPosition == mAnchor)
        return PosEqualAnc;
    if (mPosition.y() != mAnchor.y())
        return mPosition.y() > mAnchor.y() ? PosAfterAnc : PosBeforeAnc;
    return mPosition.x() > mAnchor.x() ? PosAfterAnc : PosBeforeAnc;
}

bool FastFileMapper::isBefore(const QPoint &textPos1, const QPoint &textPos2)
{
    return textPos1.y() < textPos2.y() || (textPos1.y() == textPos2.y() && textPos1.x() < textPos2.x());
}

void FastFileMapper::setOverscanLines(int newOverscanLines)
{
    mOverscanLines = newOverscanLines;
}


void FastFileMapper::LinesCache::reset() const
{
    // resets all caches (mapper pointer is kept)
    mData = "";
    mLastLineLength = -1;
    mLineChar.clear();
    mCacheOffsetLine = 0;
}

QString FastFileMapper::LinesCache::getLines(int lineNr, int count) const
{
    if (!mMapper->lineCount()) return QString();
    // get the lines needed by the mapper
    if (count == 0)
        return QString();
    if (count < 0) {
        lineNr += count;
        count = -count;
    }
    if (lineNr < mCacheOffsetLine || lineNr + count >= mCacheOffsetLine + cachedLineCount()) {
        // if a part is missing, move cache
        int fromLine = qMax(0, lineNr - mMapper->mOverscanLines);
        int lineCount = qMin(qAbs(mMapper->lineCount()) - fromLine, count + mMapper->mOverscanLines * 2);
        loadCache(fromLine, lineCount);
        // if cropped now, then adjust count
        if (lineNr - mCacheOffsetLine + count >= cachedLineCount())
            count = cachedLineCount() - (lineNr - mCacheOffsetLine);
    }
    int localLine = lineNr - mCacheOffsetLine;
    if (mLineChar.size() <= localLine)
        return QString();
    qint64 from = mLineChar.at(localLine);
    qint64 to = mLineChar.at(localLine + count);
    return mData.sliced(from, to - from - mMapper->delimiter().size());
}

int FastFileMapper::LinesCache::cachedLineCount() const
{
    // The last element is only kept to determine the end of the last line
    return mLineChar.size() - 1;
}

QPoint FastFileMapper::LinesCache::posForOffset(int offset)
{
    if (!mLineChar.size()) return QPoint();
    int a = 0;
    int b = cachedLineCount();
    while (a + 1 < b) {
        int i = (a + b) / 2;
        if (offset > mLineChar.at(i)) a = i;
        else b = i;
    }
    return QPoint(offset - mLineChar.at(a), a + mCacheOffsetLine);
}

int FastFileMapper::LinesCache::lineLength(int lineNr) const
{
    if (lineNr < 0 || lineNr >= mMapper->lineCount())
        return 0;
    bool isLast =(lineNr == mMapper->lineCount() - 1);
    if (isLast && mLastLineLength >= 0)
        return mLastLineLength;

    if (lineNr < mCacheOffsetLine || lineNr >= mCacheOffsetLine + cachedLineCount())
        getLines(lineNr, 1);
    if (!mLineChar.size())
        return 0;
    int localLine = lineNr - mCacheOffsetLine;
    return mLineChar.at(localLine + 1) - mLineChar.at(localLine) - mMapper->delimiter().size();
}

qint64 FastFileMapper::LinesCache::linePos(int line) const
{
    if (line < mCacheOffsetLine || line >= mCacheOffsetLine + cachedLineCount())
        return -1;
    return mLineChar.at(line - mCacheOffsetLine);
}

const QString FastFileMapper::LinesCache::loadCache(int lineNr, int count) const
{
    if (count == 0) {
        mData.clear();
        return mData;
    }
    if (count < 0) {
        lineNr += count + 1;
        count = -count;
    }
    mCacheOffsetLine = lineNr;
    int toLine = lineNr + count;
    QByteArray bArray;
    qint64 readSize;
    {
        QMutexLocker locker(&mMapper->mMutex);
        if (mMapper->mLineByte.size() > toLine)
            readSize = mMapper->mLineByte.at(toLine) - mMapper->delimiter().size() - mMapper->mLineByte.at(lineNr);
        else
            readSize = mMapper->mBytesPerLine * qint64(toLine - lineNr) + 100; //estimated line size plus some buffer
        if (readSize > CMaxCharsPerBlock) {
            readSize = CMaxCharsPerBlock;
            DEB() << "Error reading data: requested size exceeds 2^30 characters";
        }
        if (readSize == 0) {
            mData.clear();
            return mData;
        }
        int bSize = 0;
        bool atEnd = false;
        if (mMapper->mLineByte.size() > lineNr) {
            mMapper->mFile.seek(mMapper->mLineByte.at(lineNr));
        } else {
            qint64 from = qMin(qint64(lineNr) * mMapper->mBytesPerLine, mMapper->size() - 1);
            mMapper->mFile.seek(from);
        }
        bArray.resize(readSize);
        bSize = int(mMapper->mFile.read(bArray.data(), readSize));
        atEnd = mMapper->mFile.atEnd();
        if (atEnd)
            bArray.resize(bSize);
    }
    mData = mMapper->decode(bArray);
    mLineChar.clear();
    mLineChar.reserve(count + 1);
    mLineChar.append(0);
    for (qint64 i = 0; i < mData.length(); ++i) {
        if (mData.at(i) == '\n')
            mLineChar.append(i+1);
    }
    mLineChar.append(mData.length() + mMapper->delimiter().size());
    // the last line is requested more frequently - remember size
    if (mLastLineLength < 0 && mCacheOffsetLine + count == mMapper->lineCount())
        mLastLineLength = mLineChar.last() - mLineChar.at(cachedLineCount() - 1) - mMapper->delimiter().size();
    return mData;
}

} // namespace studio
} // namespace gams
