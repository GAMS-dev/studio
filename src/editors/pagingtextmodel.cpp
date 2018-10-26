#include "pagingtextmodel.h"
#include "exception.h"
#include "logger.h"
#include <QFile>
#include <QTextStream>

namespace gams {
namespace studio {

RawText::RawText()
{
    mCodec = QTextCodec::codecForLocale();
}

RawText::~RawText()
{

}

void RawText::openFile(const QString &fileName)
{
    TRACETIME();
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            DEB() << "Could not open file " << fileName;
            return;
        }
        mData = file.readAll();
        // determine the line delimiter of this file
        for (int i = 0; i < mData.size(); ++i) {
            if (mData.at(i) == '\n' || mData.at(i) == '\r') {
                if (mData.size() > i+1 && mData.at(i) != mData.at(i+1) && (mData.at(i+1) == '\n' || mData.at(i+1) == '\r')) {
                    mDelimiter = mData.mid(i, 2);
                } else {
                    mDelimiter = mData.mid(i, 1);
                }
                break;
            }
        }
        DEB() << " delimiter size: " << mDelimiter.size();
        int lines = mData.count(mDelimiter.at(0));
        mDataIndex.reserve(lines+2);
        mDataIndex << 0;
        for (int i = 0; i < mData.size(); ++i) {
            if (mData[i] == mDelimiter.at(0)) {
                mDataIndex << (i + mDelimiter.size());
            }
        }
        mDataIndex << mData.length()+mDelimiter.size();
        file.close();
    }
}

void RawText::clear()
{
    mDataIndex.clear();
    mData.clear();
}

int RawText::blockCount() const
{
    return mDataIndex.count()-1;
}

QString RawText::block(int i) const
{
    QTextStream in(mData);
    in.setCodec(mCodec);
    in.seek(mDataIndex.at(int(i)));
    return in.read(mDataIndex.at(int(i)+1)-mDataIndex.at(int(i))-mDelimiter.size());
}

void RawText::appendLine(const QString &line)
{
    // TODO(JM)
//    mData.append(QByteArray());
//    QTextStream out(&mData.last());
//    out.setCodec(mCodec);
//    out << line;
}


PagingText::PagingText()
{
    clear();
}

PagingText::~PagingText()
{
    if (mFile.isOpen()) {
        mFile.close();
    }
}

void PagingText::openFile(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        clear();
        mFile.setFileName(fileName);
        if (!mFile.open(QFile::ReadOnly)) {
            DEB() << "Could not open file " << fileName;
            return;
        }
        mFileSize = mFile.size();
        ChunkMap &map = getChunk(0);
        // determine the line delimiter of this file
        for (int i = 0; i < mData.size(); ++i) {
            if (map.bArray.at(i) == '\n' || map.bArray.at(i) == '\r') {
                if (map.bArray.size() > i+1 && map.bArray.at(i) != map.bArray.at(i+1)
                        && (map.bArray.at(i+1) == '\n' || map.bArray.at(i+1) == '\r')) {
                    mDelimiter = map.bArray.mid(i, 2);
                } else {
                    mDelimiter = map.bArray.mid(i, 1);
                }
                break;
            }
        }
        DEB() << " delimiter size: " << mDelimiter.size();
    }
}

void PagingText::clear()
{
    if (mFile.isOpen()) {
        for (ChunkMap &block: mData) {
            mFile.unmap(block.pointer);
        }
        mData.clear();
        mFile.close();
    }
    mPos = 0;
    mAnchor = 0;
    mTopPos = 0;
}

int PagingText::blockCount() const
{
    return mFileSize;
}

QString PagingText::block(int i) const
{

}

PagingText::ChunkMap &PagingText::getChunk(qint64 byteNr)
{
    qint64 start = ((byteNr / mChunkSize) * mChunkSize) - mOverlap;
    // block already present?
    for (ChunkMap &bm: mData) {
        if (bm.start == start) return bm;
    }

    // remove max-exceeding block
    if (mData.size() > mMaxChunks) {
        ChunkMap delBlock = mData.takeFirst();
        mFile.unmap(delBlock.pointer);
    }
    // map file into BlockMap
    ChunkMap res;
    res.start = start;
    qint64 end = start + (2 * mOverlap);
    if (start < 0) start = 0;
    if (end > mFileSize) end = mFileSize;
    res.size = end-start;
    res.pointer = mFile.map(res.start, res.size);
    res.bArray.setRawData(reinterpret_cast<char*>(res.pointer), uint(res.size));

    // create index for linebreaks
    int lines = res.bArray.count(mDelimiter.at(0));
    res.index.reserve(lines+2);
    res.index << 0;
    for (int i = 0; i < mData.size(); ++i) {
        if (res.bArray[i] == mDelimiter.at(0)) {
            res.index << (i + mDelimiter.size());
        }
    }
    res.index << mData.length()+mDelimiter.size();

    mData << res;
    return mData.last();
}



PagingTextModel::PagingTextModel(QFontMetrics metrics, QObject *parent)
    : QAbstractTableModel(parent), mMetrics(metrics)
{
    mTextData = new RawText();
}

PagingTextModel::~PagingTextModel()
{
    delete mTextData;
}

int PagingTextModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mTextData->blockCount();
}

int PagingTextModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mColumns;
//    int res = mShowLineNr ? 2 : 1;
//    return mMarks.isEmpty() ? res : res+1;
}

QVariant PagingTextModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    switch (role) {
    case Qt::DisplayRole: {
        return index.column() ? mTextData->block(index.row()) : QVariant(index.row()+1);
    }
    case Qt::SizeHintRole: {
        QString text = index.column() ? mTextData->block(index.row()) : QString::number(index.row()+1);
        DEB() << "row " << index.row() << "  width " << (6 + mMetrics.width(text));
        return QVariant(6 + mMetrics.width(text));
    }
    case Qt::DecorationRole:
        // TODO(JM) Icon for TextMark
        break;
    case Qt::TextAlignmentRole:
        if (index.column()+1 < mColumns) {
            return QVariant(Qt::AlignRight);
        }
        break;
    case Qt::TextColorRole:
        if (index.column()+1 < mColumns) {
            return QColor(Qt::gray);
        }
        break;
    case Qt::BackgroundColorRole:
        if (index.column()+1 < mColumns) {
            return QColor(245,245,245);
        }
        break;
    default:
        break;
    }
    return QVariant();
}

void PagingTextModel::setLineNumbersVisible(bool visible)
{
    if (mLineNumbersVisible != visible) {
        if (visible) beginInsertColumns(QModelIndex(), mColumns, mColumns);
        else beginRemoveColumns(QModelIndex(), mColumns-2, mColumns-2);
        mLineNumbersVisible = visible;
        updateSizes();
        if (visible) endInsertColumns();
        else endRemoveColumns();
    }
}

int PagingTextModel::columnWidth(int column)
{
    if (!mMarks.isEmpty() && column == 0) return 16;
    if (mLineNumbersVisible && column == mColumns-2) return mLineNrWidth;
    return 10;
}

void PagingTextModel::loadFile(const QString &fileName)
{
    beginResetModel();
    mTextData->clear();
    mTextData->openFile(fileName);
    updateSizes();
    endResetModel();
}

void PagingTextModel::appendLine(const QString &line)
{
    beginInsertRows(QModelIndex(), mTextData->blockCount(), mTextData->blockCount());
    mTextData->appendLine(line);
    updateSizes();
    endInsertRows();
}

void PagingTextModel::updateSizes()
{
    bool change = false;
    int res = mLineNumbersVisible ? 2 : 1;
    int value = (mMarks.isEmpty() ? res : res+1);
    if (value != mColumns) {
        mColumns = value;
        change = true;
    }

    int digits = 1;
    int max = qMax(1, mTextData->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
//    DEB() << "digits for " << mRawText.lineCount() << ": " << digits;
    value = (6 + mMetrics.width(QLatin1Char('9')) * digits);
    if (value != mLineNrWidth) {
        mLineNrWidth = value;
        change = true;
    }
    if (change) emit reorganized();
}

void PagingTextModel::setFontMetrics(QFontMetrics metrics)
{
    mMetrics = metrics;
    updateSizes();
}

QTextCodec *TextProvider::codec() const
{
    return mCodec;
}

void TextProvider::setCodec(QTextCodec *codec)
{
    mCodec = codec;
}

} // namespace studio
} // namespace gams
