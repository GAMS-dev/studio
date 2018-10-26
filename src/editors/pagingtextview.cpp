#include "pagingtextview.h"
#include "exception.h"
#include "logger.h"

#include <QHeaderView>
#include <QScrollBar>

namespace gams {
namespace studio {

PagingTextView::PagingTextView(QWidget *parent): QTableView(parent), mModel(parent->fontMetrics())
{
    setModel(&mModel);
    connect(&mModel, &PagingTextModel::reorganized, this, &PagingTextView::reorganize);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setVisible(false);
//    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultSectionSize(15);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(15);
    horizontalHeader()->setMinimumSectionSize(15);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setShowGrid(false);
}

void PagingTextView::loadFile(QString fileName)
{
    mModel.loadFile(fileName);
    calculateWidth();
    adjustWidth();
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

void PagingTextView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    QModelIndex leftInd = mModel.index(index.row(), 0, index.parent());
    QTableView::scrollTo(leftInd, hint);
}

void PagingTextView::showEvent(QShowEvent *event)
{
    QTableView::showEvent(event);
    calculateWidth();
    adjustWidth();
}

bool PagingTextView::event(QEvent *event)
{
    bool res = QTableView::event(event);
    if (event->type() == QEvent::FontChange) {
        QFontMetrics metric = QFontMetrics(font());
        mModel.setFontMetrics(metric);
        if (metric.height() != verticalHeader()->defaultSectionSize())
            verticalHeader()->setDefaultSectionSize(metric.height());
    }
    return res;
}

void PagingTextView::reorganize()
{
    horizontalHeader()->setCascadingSectionResizes(true);
    int to = mModel.columnCount(QModelIndex())-1;
    for (int col = 0; col < to; ++col) {
        setColumnWidth(col, mModel.columnWidth(col));
    }
}

void PagingTextView::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent(event);
    mModel.setFontMetrics(fontMetrics());
}

void PagingTextView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    adjustWidth();
}

void PagingTextView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        event->accept();
        (event->delta() > 0) ? zoomIn() : zoomOut();
    } else {
        QTableView::wheelEvent(event);
    }
}

void PagingTextView::calculateWidth()
{
    mLastWidthData.width = 0;
    int col = mModel.columnCount(QModelIndex())-1;
    mLastWidthData.top = rowAt(0);
    mLastWidthData.bottom = qMax(rowAt(height()), mModel.rowCount(QModelIndex())-1);
    if (mLastWidthData.bottom > mLastWidthData.top+1000) mLastWidthData.bottom = mLastWidthData.top+500;
    for (int row = mLastWidthData.top; row < mLastWidthData.bottom; ++row) {
        int width = mModel.data(mModel.index(row, col), Qt::SizeHintRole).toInt();
        if (mLastWidthData.width < width) mLastWidthData.width = width;
    }
}

void PagingTextView::scrollContentsBy(int dx, int dy)
{
    QTableView::scrollContentsBy(dx, dy);
    if (dy) {
        calculateWidth();
        adjustWidth();
    }
}

void PagingTextView::adjustWidth()
{
    int maxWidth = width() - verticalScrollBar()->width() - 6;
    for (int col = 0; col < mModel.columnCount(QModelIndex())-1; ++col) {
        maxWidth -= mModel.columnWidth(col);
    }
    if (maxWidth < mLastWidthData.width) maxWidth = mLastWidthData.width;
    int col = mModel.columnCount(QModelIndex())-1;
    if (columnWidth(col) != maxWidth) setColumnWidth(col, maxWidth);

}

} // namespace studio
} // namespace gams
