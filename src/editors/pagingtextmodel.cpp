#include "pagingtextmodel.h"
#include "exception.h"
#include "logger.h"
#include <QFile>
#include <QTextStream>
#include <QHeaderView>

namespace gams {
namespace studio {

RawText::RawText()
{
    mCodec = QTextCodec::codecForLocale();
}

void RawText::loadFile(const QString &fileName)
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

int RawText::lineCount() const
{
    return mDataIndex.count()-1;
}

QString RawText::line(int i) const
{
    QTextStream in(mData);
    in.setCodec(mCodec);
    in.seek(mDataIndex.at(i));
//    if (i < 5) {
//        QString deb;
//        QString debM;
//        int a = mDataIndex.at(i)-3;
//        if (a < 0) {
//            deb.fill(' ',-a);
//            debM.fill(' ',-a);
//            a = 0;
//        }
//        int s = mDataIndex.at(i+1)-mDataIndex.at(i)-mDelimiter.size();
//        debM += '^' + QString(s-2, '~') + '^';
//        if (s+a >= mData.size()) s = mData.size()-a-1;
//        for (int x = a; x < s+a ; ++x) {
//            if (mData.at(x) == '\n') deb += '_';
//            else if (mData.at(x) == '\r') deb += '^';
//            else deb += mData.at(x);
//        }
//        DEB() << deb << "   from " << a << ", size " << s;
//        DEB() << debM;
//    }
    return in.read(mDataIndex.at(i+1)-mDataIndex.at(i)-mDelimiter.size());
}

void RawText::appendLine(const QString &line)
{
    // TODO(JM)
//    mData.append(QByteArray());
//    QTextStream out(&mData.last());
//    out.setCodec(mCodec);
//    out << line;
}


PagingTextModel::PagingTextModel(QFontMetrics metrics, QObject *parent)
    : QAbstractTableModel(parent), mRawText(), mMetrics(metrics)
{
}

PagingTextModel::~PagingTextModel()
{
}

int PagingTextModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mRawText.lineCount();
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
    case Qt::DisplayRole:
        return index.column() ? mRawText.line(index.row()) : QVariant(index.row()+1);
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
    mRawText.clear();
    mRawText.loadFile(fileName);
    updateSizes();
    endResetModel();
}

void PagingTextModel::appendLine(const QString &line)
{
    beginInsertRows(QModelIndex(), mRawText.lineCount(), mRawText.lineCount());
    mRawText.appendLine(line);
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
    int max = qMax(1, mRawText.lineCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    DEB() << "digits for " << mRawText.lineCount() << ": " << digits;
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

PagingTextView::PagingTextView(QWidget *parent): QTableView(parent), mModel(parent->fontMetrics())
{
    setModel(&mModel);
    connect(&mModel, &PagingTextModel::reorganized, this, &PagingTextView::reorganize);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setVisible(false);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultSectionSize(15);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(15);
    horizontalHeader()->setMinimumSectionSize(15);
    setShowGrid(false);
}

void PagingTextView::loadFile(QString fileName)
{
    mModel.loadFile(fileName);
}

FileId PagingTextView::fileId() const
{
    return mFileId;
}

void PagingTextView::setFileId(const FileId &fileId)
{
    mFileId = fileId;
}

NodeId PagingTextView::groupId() const
{
    return mGroupId;
}

void PagingTextView::setGroupId(const NodeId &groupId)
{
    mGroupId = groupId;
}

void PagingTextView::zoomIn(int range)
{
    zoomInF(range);
}

void PagingTextView::zoomOut(int range)
{
    zoomInF(-range);
}

void PagingTextView::zoomInF(float range)
{
    if (range == 0.f)
        return;
    QFont f = font();
    const double newSize = f.pointSizeF() + double(range);
    if (newSize <= 0)
        return;
    f.setPointSizeF(newSize);
    setFont(f);
}

void PagingTextView::showEvent(QShowEvent *event)
{
    QTableView::showEvent(event);
}

bool PagingTextView::event(QEvent *event)
{
    bool res = QTableView::event(event);
    if (event->type() == QEvent::FontChange) {
        mModel.setFontMetrics(QFontMetrics(font()));
    }
    return res;
}

void PagingTextView::reorganize()
{
    horizontalHeader()->setCascadingSectionResizes(true);
    DEB() << "MinSec: " << horizontalHeader()->minimumSectionSize();
    int to = mModel.columnCount(QModelIndex())-1;
    for (int col = 0; col < to; ++col) {
        setColumnWidth(col, mModel.columnWidth(col));
        DEB() << "col[" << col << "]  wid from " << columnWidth(col) << " to " << mModel.columnWidth(col);
    }
}

void PagingTextView::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent(event);
    mModel.setFontMetrics(fontMetrics());
}

} // namespace studio
} // namespace gams
