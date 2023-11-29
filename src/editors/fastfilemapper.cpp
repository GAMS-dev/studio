#include "fastfilemapper.h"
#include "logger.h"

#include <QRegularExpression>
#include <QElapsedTimer>
#include <QtConcurrent>

namespace gams {
namespace studio {

FastFileMapper::FastFileMapper(QObject *parent)
    : gams::studio::AbstractTextMapper{parent}
{}

FastFileMapper::~FastFileMapper()
{}

bool FastFileMapper::openFile(const QString &fileName, bool initAnchor)
{
    if (!fileName.isEmpty()) {
        closeAndReset();

        if (initAnchor) {
            mPosition = QPoint();
            mAnchor = QPoint();
        }

        QElapsedTimer et;
        et.start();
        mFile.setFileName(fileName);
        mSize = mFile.size();
        if (!mFile.open(QFile::ReadOnly)) {
            DEB() << "Could not open file " << fileName;
            return false;
        }
        mLines = scanLF();
        DEB() << "elapsed: " << et.elapsed();
        initDelimiter();
        updateMaxTop();
        setVisibleTopLine(0);
        emit loadAmountChanged(mLines.count());
        emit blockCountChanged();
    }
    return false;
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
    // TODO(JM) implement
}

void FastFileMapper::endRun()
{
    // TODO(JM) implement
}

bool FastFileMapper::setVisibleTopLine(double region)
{
    return setVisibleTopLine(int(region * lineCount()));
}

bool FastFileMapper::setVisibleTopLine(int lineNr)
{
    int validLineNr = qMax(0, qMin(lineNr, mLines.size() - mVisibleLineCount));
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
    // TODO(JM) implement
    // int lineNr = position();
    // if (pos.y() < visibleLineCount() / 5 || pos.y() == cursorBeyondEnd || pos.y() > (visibleLineCount() * 4) / 5)
    //     setVisibleTopLine(cm->startLineNr + mPosition.localLine - visibleLineCount() / 2);
}

int FastFileMapper::lineCount() const
{
    return mLines.count();
}

int FastFileMapper::knownLineNrs() const
{
    return mLines.count();
}

QString FastFileMapper::lines(int localLineNrFrom, int lineCount) const
{
    int lineNr = visibleTopLine() + localLineNrFrom;
    adjustLines(lineNr, lineCount);
    return readLines(lineNr, lineCount);
}

QString FastFileMapper::lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const
{
    int lineNr = visibleTopLine() + localLineNrFrom;
    adjustLines(lineNr, lineCount);
    QString res = readLines(lineNr, lineCount);
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
    // TODO(JM) implement
    return false;
}

QString FastFileMapper::selectedText() const
{
    // TODO(JM) implement
    return QString();
}

QString FastFileMapper::positionLine() const
{
    if (!mLines.count())
        return QString();
    return readLines(mPosition.y(), 1);
}

void FastFileMapper::setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode)
{
    mPosition = QPoint(visibleTopLine() + localLineNr, charNr);
    if (mode == QTextCursor::MoveAnchor)
        mAnchor = mPosition;
}

void FastFileMapper::setPosToAbsStart(QTextCursor::MoveMode mode)
{
    mPosition = QPoint();
    if (mode == QTextCursor::MoveAnchor)
        mAnchor = mPosition;
}

void FastFileMapper::setPosToAbsEnd(QTextCursor::MoveMode mode)
{
    mPosition = endPosition();
    if (mode == QTextCursor::MoveAnchor)
        mAnchor = mPosition;
}

void FastFileMapper::selectAll()
{
    mPosition = QPoint();
    mAnchor = endPosition();
}

void FastFileMapper::clearSelection()
{
    mAnchor = mPosition;
}

QPoint FastFileMapper::position(bool local) const
{
    if (!local)
        return mPosition;
    return QPoint(mPosition.x(), mPosition.y() - visibleTopLine());
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
    if (mAnchor == mPosition || mLines.count() == 0) return 0;
    // We set a limit of maxint / 20
    QPoint pFrom = mAnchor;
    QPoint pTo = mPosition;
    if (pFrom.y() < 0)
        pFrom = QPoint(0, 0);
    if (pFrom.y() >= mLines.count())
        pFrom = QPoint(int(size() - mLines.last()), mLines.count() - 1);

    if (pTo.y() < 0)
        pTo = QPoint(0, 0);
    if (pTo.y() >= mLines.count())
        pTo = QPoint(int(size() - mLines.last()), mLines.count() - 1);

    qint64 from = mLines.at(pFrom.y()) + pFrom.x();
    qint64 to = mLines.at(pTo.y()) + pTo.x();

    // To avoid time-consuming double codec-conversion: estimate with factor *2 (1 Byte in storage is 2 Bytes in Memory)
    return qAbs(to - from) * 2;
}

bool FastFileMapper::atTail()
{
    return lineCount() >= 0 && mVisibleTopLine + mVisibleLineCount >= lineCount();
}

void FastFileMapper::updateSearchSelection()
{
    // TODO(JM) implement
}

void FastFileMapper::clearSearchSelection()
{
    // TODO(JM) implement
}

QPoint FastFileMapper::searchSelectionStart()
{
    // TODO(JM) implement
    return QPoint();
}

QPoint FastFileMapper::searchSelectionEnd()
{
    // TODO(JM) implement
    return QPoint();
}

void FastFileMapper::dumpPos() const
{
    // TODO(JM) implement
}

void FastFileMapper::reset()
{
    AbstractTextMapper::reset();
    mPosition = QPoint();
    mAnchor = QPoint();
    mVisibleTopLine = 0;
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
    mLines.clear();
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

QList<qint64> FastFileMapper::scanLF()
{
    const int threadMax = 32;
    mFile.reset();
    QList<qint64> lf;
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
    //    DEB() << "THREAD    :  File: " << mFile.fileName() << "  size(count):" << mSize << " (" << start << ")  " << "lines: " << lf.size();
    return lf;

}

QPoint FastFileMapper::endPosition()
{
    if (mLines.count()) {
        int lineLen = size() - mLines.last();
        return QPoint(lineLen, mLines.count()-1);
    }
    return QPoint();
}

QString FastFileMapper::readLines(int lineNr, int count) const
{
    QString res;
    QTextStream ds(&mFile);
    ds.seek(mLines.at(lineNr));
    int toLine = lineNr + count;
    bool atEnd = toLine == mLines.count() || mLines.at(toLine) == size();
    qint64 readSize = (atEnd ? size() : mLines.at(toLine) - delimiter().length()) - mLines.at(lineNr);
    res = ds.read(readSize);
    return res;
}

bool FastFileMapper::adjustLines(int &lineNr, int &count) const
{
    int fromLine = qBound(0, lineNr, mLines.count()-1);
    int toLine = qBound (0, count + lineNr, mLines.count());
    if (fromLine == lineNr && count == toLine - fromLine)
        return true;
    lineNr = fromLine;
    count = toLine - fromLine;
    return false;
}

void FastFileMapper::initDelimiter() const
{
    setDelimiter("");
    if (mLines.count() < 2) return;
    mFile.reset();
    QDataStream ds(&mFile);
    int start = mLines.at(1)-2;
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

} // namespace studio
} // namespace gams
