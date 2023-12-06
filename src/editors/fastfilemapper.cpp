#include "fastfilemapper.h"
#include "logger.h"

#include <QRegularExpression>
#include <QElapsedTimer>
#include <QtConcurrent>

namespace gams {
namespace studio {

const int CMaxCharsPerBlock = 1024*1024*1024;
const int CExtraLines = 50;

FastFileMapper::FastFileMapper(QObject *parent)
    : gams::studio::AbstractTextMapper(parent)
    , mCache(this)
{}

FastFileMapper::~FastFileMapper()
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
        scanLF(mLineByte);
        initDelimiter();
        updateMaxTop();
        setVisibleTopLine(0);
        emit loadAmountChanged(mLineByte.count());
        emit blockCountChanged();
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
    int validLineNr = qMax(0, qMin(lineNr, mLineByte.size() - visibleLineCount()));
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
    return mLineByte.count();
}

int FastFileMapper::knownLineNrs() const
{
    return mLineByte.count();
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
            LineFormat markedLine(0, 0, fmt);
            markedLine.lineMarked = true;
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
    bool backwards = flags.testFlag(QTextDocument::FindBackward);
    int part = backwards ? 2 : 1;

    // TODO(JM) implement
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
    return res.mid(from, len);
}

QString FastFileMapper::positionLine() const
{
    if (!mLineByte.count())
        return QString();
    return mCache.getLines(mPosition.y(), 1);
}

void FastFileMapper::setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode)
{
    bool toEnd = (localLineNr > 0) && (charNr == -1);
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
    return QPoint(mAnchor.x(), mAnchor.y() - visibleTopLine());
}

bool FastFileMapper::hasSelection() const
{
    return mPosition != mAnchor;
}

int FastFileMapper::selectionSize() const
{
    if (mAnchor == mPosition || mLineByte.count() == 0) return 0;
    // We set a limit of maxint / 20
    QPoint pFrom = mAnchor;
    QPoint pTo = mPosition;
    if (pFrom.y() < 0)
        pFrom = QPoint(0, 0);
    if (pFrom.y() >= mLineByte.count())
        pFrom = QPoint(int(size() - mLineByte.last()), mLineByte.count() - 1);

    if (pTo.y() < 0)
        pTo = QPoint(0, 0);
    if (pTo.y() >= mLineByte.count())
        pTo = QPoint(int(size() - mLineByte.last()), mLineByte.count() - 1);

    qint64 from = mLineByte.at(pFrom.y()) + pFrom.x();
    qint64 to = mLineByte.at(pTo.y()) + pTo.x();

    // To avoid time-consuming double codec-conversion: estimate with factor *2 (1 Byte in storage is 2 Bytes in Memory)
    return qAbs(to - from) * 2;
}

bool FastFileMapper::atTail()
{
    return lineCount() >= 0 && mVisibleTopLine + visibleLineCount() >= lineCount();
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

void FastFileMapper::reset()
{
    AbstractTextMapper::reset();
    mPosition = QPoint(0,-1);
    mAnchor = QPoint(0,-1);
    mVisibleTopLine = 0;
    mCache.reset();
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
    mLineByte.clear();
    reset();
}

QList<qint64> subScanLF(char*data, int len, qint64 offset)
{
    QList<qint64> res;
    res.reserve(5000);
    for (int i = 0; i < len; ++i) {
        if (data[i] == '\n')
            res.append(offset + i + 1);
    }
    return res;
}

QList<qint64> FastFileMapper::scanLF(QList<qint64> &lf)
{
    const int threadMax = 32;
    mFile.reset();
    lf.clear();
    lf.reserve(mFile.size() / 45);
    lf << 0.;
    QDataStream ds(&mFile);
    int len = 1024*512;
    char *data[threadMax];
    for (int i = 0; i < threadMax; ++i)
        data[i] = new char[len];
    qint64 start = 0;

    QList< QFuture< QList<qint64> > > fut;
    int threadCount = 0;
    while (!ds.atEnd()) {
        int len1 = ds.readRawData(data[threadCount], len);
        qint64 offset = start;
        fut << QtConcurrent::run(subScanLF, data[threadCount], len1, offset);
        start += len1;
        ++threadCount;
        if (threadCount >= threadMax || ds.atEnd())  {
            for (int i = 0; i < fut.count(); ++i) {
                lf << fut.at(i).result();
            }
            fut.clear();
            threadCount = 0;
        }
    }
    lf.squeeze();
    for (int i = 0; i < threadMax; ++i)
        delete data[i];
    return lf;

}

QPoint FastFileMapper::endPosition()
{
    if (!mLineByte.count()) return QPoint();
    return QPoint(mCache.lineLength(mLineByte.count()-1), mLineByte.count()-1);
}

bool FastFileMapper::adjustLines(int &lineNr, int &count) const
{
    int fromLine = qMax(0, qMin(lineNr, mLineByte.count()-1));
    int toLine = qBound(0, count + lineNr, mLineByte.count());
    if (fromLine == lineNr && count == toLine - fromLine)
        return true;
    lineNr = fromLine;
    count = toLine - fromLine;
    return false;
}

void FastFileMapper::initDelimiter() const
{
    setDelimiter("");
    if (mLineByte.count() < 2) return;
    mFile.reset();
    QDataStream ds(&mFile);
    int start = mLineByte.at(1)-2;
    if (start < 0) {
        setDelimiter("\n");
        return;
    }
    if (start > 0)
        ds.skipRawData(start);
    char delim[2];
    ds.readRawData(delim, 2);
    setDelimiter((delim[0] == '\r') ? "\r\n" : "\n");
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


void FastFileMapper::LinesCache::reset() const
{
    // resets all caches (mapper pointer is kept)
    mCache = "";
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
    if (lineNr < mCacheOffsetLine || lineNr + count > mCacheOffsetLine + mLineChar.size() - 1) {
        // if a part is missing, move cache
        int fromLine = qMax(0, lineNr - CExtraLines);
        int lineCount = qMin(mMapper->lineCount(), count + CExtraLines * 2);
        if (fromLine + lineCount > mMapper->lineCount())
            fromLine -= fromLine + lineCount - mMapper->lineCount();
        moveCache(fromLine, lineCount);
    }
    int localLine = lineNr - mCacheOffsetLine;
    qint64 from = mLineChar.at(localLine);
    qint64 to = mLineChar.at(localLine + count);
    return mCache.mid(from, to - from - mMapper->delimiter().size());
}

int FastFileMapper::LinesCache::lineLength(int lineNr) const
{
    if (lineNr < 0 || lineNr >= mMapper->lineCount())
        return 0;
    bool isLast =(lineNr == mMapper->lineCount() - 1);
    if (isLast && mLastLineLength >= 0)
        return mLastLineLength;

    if (lineNr < mCacheOffsetLine || lineNr >= mCacheOffsetLine + mLineChar.size())
        getLines(lineNr, 1);
    int localLine = lineNr - mCacheOffsetLine;
    return mLineChar.at(localLine + 1) - mLineChar.at(localLine) - mMapper->delimiter().size();
}

void FastFileMapper::LinesCache::moveCache(int lineNr, int count) const
{
    // TODO apply codec (default from main-file)
    if (count == 0) return;
    if (count < 0) {
        lineNr += count;
        count = -count;
    }
    mCacheOffsetLine = lineNr;
    int toLine = lineNr + count; // the linebreak right after the last line to read
    bool atEnd = (toLine == mMapper->mLineByte.count());
    qint64 readSize = (atEnd ? mMapper->size() : mMapper->mLineByte.at(toLine) - mMapper->delimiter().size())
                      - mMapper->mLineByte.at(lineNr);
    if (readSize > CMaxCharsPerBlock) {
        readSize = CMaxCharsPerBlock;
        DEB() << "Error reading data: requested size exceeds 2^30 characters";
    }
    if (readSize == 0)
        return;
    QTextStream ts(&mMapper->mFile);
    ts.seek(mMapper->mLineByte.at(lineNr));
    mCache = ts.read(readSize);
    mLineChar.clear();
    mLineChar.reserve(count+1);
    mLineChar.append(0);
    for (qint64 i = 0; i < mCache.length(); ++i) {
        if (mCache.at(i) == '\n')
            mLineChar.append(i+1);
    }
    mLineChar.append(mCache.length() + mMapper->delimiter().size());
    // the last line is requested more frequently - remember size
    if (mLastLineLength < 0 && mCacheOffsetLine + count == mMapper->lineCount() - 1)
        mLastLineLength = mLineChar.last() - mLineChar.at(mLineChar.size() - 2) - mMapper->delimiter().size();
}

} // namespace studio
} // namespace gams
