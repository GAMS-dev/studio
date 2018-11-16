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
#include "textview.h"
#include "logger.h"
#include "exception.h"

#include <QScrollBar>
#include <QTextBlock>
#include <QPlainTextDocumentLayout>
#include <QBoxLayout>

namespace gams {
namespace studio {

void TextViewEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
        DEB() << "before key: " << verticalScrollBar()->value();
    }
    CodeEdit::keyPressEvent(event);
    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
        DEB() << "after key: " << verticalScrollBar()->value();
    }
//    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown)
//        emit verticalScrollBar()->valueChanged(verticalScrollBar()->value());
//    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
//        DEB() << "after update: " << verticalScrollBar()->value();
//    }
}


TextView::TextView(QWidget *parent) : QAbstractScrollArea(parent)
{
    setViewportMargins(0,0,0,0);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mEdit = new TextViewEdit(mMapper, this);
    mEdit->setFrameShape(QFrame::NoFrame);
    mEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//    mEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QVBoxLayout *lay = new QVBoxLayout(this);
    setLayout(lay);
    lay->addWidget(mEdit);

//    mEdit->setCursorWidth(0);
    connect(verticalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::outerScrollAction);
//    connect(mEdit, &CodeEdit::cursorPositionChanged, this, &TextView::editScrollChanged);
    connect(mEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::editScrollChanged);
//    connect(mEdit->verticalScrollBar(), &QScrollBar::rangeChanged, this, &TextView::editScrollResized);

    mPeekTimer.setSingleShot(true);
    connect(&mPeekTimer, &QTimer::timeout, this, &TextView::peekMoreLines);
//    mEdit->verticalScrollBar()->setVisible(false);

/* --- scrollbar controlling qt-methods
    QObject::connect(control, SIGNAL(documentSizeChanged(QSizeF)), q, SLOT(_q_adjustScrollbars()));
    QPlainTextEdit::setDocument(QTextDocument *document);
    QPlainTextEditPrivate::append(const QString &text, Qt::TextFormat format);
    QPlainTextEdit::resizeEvent(QResizeEvent *e);
    QPlainTextEdit::setLineWrapMode(LineWrapMode wrap);
*/

}

int TextView::lineCount() const
{
    return mMapper.lineCount();
}

void TextView::loadFile(const QString &fileName, QList<int> codecMibs)
{
    mMapper.setCodec(codecMibs.size() ? QTextCodec::codecForMib(codecMibs.at(0)) : QTextCodec::codecForLocale());
    mMapper.openFile(fileName);
    updateVScrollZone();
    mTransferedAmount = 0;
    int count = (mMapper.lineCount() < 0) ? 300 : mMapper.lineCount();
    mDocChanging = true;
    mEdit->setPlainText(mMapper.lines(0, count));
    mEdit->setTextCursor(QTextCursor(mEdit->document()));
    mDocChanging = false;
    mPeekTimer.start(100);
}

void TextView::zoomIn(int range)
{
    mEdit->zoomIn(range);
}

void TextView::zoomOut(int range)
{
    mEdit->zoomOut(range);
}

void TextView::getPosAndAnchor(QPoint &pos, QPoint &anchor) const
{
    mMapper.getPosAndAnchor(pos, anchor);
}

int TextView::findLine(int lineNr)
{
    if (mMapper.lineCount() >= 0 || lineNr < mMapper.knownLineNrs()) {
        if (mMapper.setTopLine(qMax(0, lineNr - 150))) return lineNr;
    }
    mTransferedLineAmount = 0;
    mLineToFind = lineNr;
    return mMapper.knownLineNrs();
}

void TextView::peekMoreLines()
{
    TextMapper::ProgressAmount amount = mMapper.peekChunksForLineNrs(4);
    if (amount.part < amount.all) mPeekTimer.start(100);
    // in 5% steps
    int percentAmount = (amount.part * 20 / amount.all) * 5;
    if (mTransferedAmount != percentAmount) emit loadAmount(percentAmount);
    mTransferedAmount = percentAmount;
    if (mLineToFind >= 0) {
        percentAmount = qMin(100, (amount.part * 20 / mLineToFind) * 5);
        if (mTransferedLineAmount != percentAmount) emit findLineAmount(percentAmount);
        mTransferedLineAmount = percentAmount;
    }
}

void TextView::outerScrollAction(int action)
{
    switch (action) {
    case QScrollBar::SliderSingleStepAdd:
    case QScrollBar::SliderSingleStepSub:
    case QScrollBar::SliderPageStepAdd:
    case QScrollBar::SliderPageStepSub:
    case QScrollBar::SliderMove:
        mActiveScrollAction = QScrollBar::SliderAction(action);
        QTimer::singleShot(0, this, &TextView::adjustOuterScrollAction);
        break;
    default:
        mActiveScrollAction = QScrollBar::SliderNoAction;
        break;
    }
}

void TextView::adjustOuterScrollAction()
{
    TRACE();
    switch (mActiveScrollAction) {
    case QScrollBar::SliderSingleStepAdd:
    case QScrollBar::SliderSingleStepSub:
    case QScrollBar::SliderPageStepAdd:
    case QScrollBar::SliderPageStepSub:
        mEdit->verticalScrollBar()->triggerAction(mActiveScrollAction);
        break;
    case QScrollBar::SliderMove:
//        if (verticalScrollBar()->minimum()) {
//            DEB() << "Scrollbar[" << verticalScrollBar()->minimum()
//                  << "] at " << verticalScrollBar()->value()
//                  << " (" << (verticalScrollBar()->value()-verticalScrollBar()->minimum()) << ")";
//        } else {
//            DEB() << "Scrollbar[+" << verticalScrollBar()->maximum()
//                  << "] at " << verticalScrollBar()->value();
//        }
        setVisibleTop(verticalScrollBar()->value() - verticalScrollBar()->minimum());
        break;
    default:
        break;
    }
    mActiveScrollAction = QScrollBar::SliderNoAction;
}

void TextView::editScrollChanged()
{
//    TRACE();
    if (!mDocChanging) setVisibleTop(mTopLine + mEdit->verticalScrollBar()->value());
}

void TextView::scrollContentsBy(int dx, int dy)
{
    // FIXME(JM) source is outer scrollbar: adapt document and inner scrollbar (of the edit)
    Q_UNUSED(dx)
    Q_UNUSED(dy)
    //    syncVScroll(-1, verticalScrollBar()->value());
}

void TextView::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    mVisibleLines = (mEdit->height() - mEdit->contentsMargins().top() - mEdit->contentsMargins().bottom())
            / mEdit->fontMetrics().height();
    updateVScrollZone();
}

void TextView::showEvent(QShowEvent *event)
{
    QAbstractScrollArea::showEvent(event);
    if (mInit) init();
}

void TextView::init()
{
    layout()->setContentsMargins(0,0,verticalScrollBar()->width(),0);
    mEdit->setFocus();
    mInit = false;
}

void TextView::updateVScrollZone()
{
    TRACE();
    int lineCount = mMapper.lineCount();
    if (lineCount > 0) { // known lines count
        verticalScrollBar()->setMinimum(0);
        verticalScrollBar()->setMaximum(qMax(lineCount-mVisibleLines, 0));
    } else { // estimated lines count
        verticalScrollBar()->setMinimum(qMin(lineCount+mVisibleLines, 0));
        verticalScrollBar()->setMaximum(0);
    }
    syncVScroll();
}

void TextView::syncVScroll()
{
    TRACE();
    if (mMapper.absTopLine() >= 0) { // current line is known
        DEB() << "syncVScroll: " << verticalScrollBar()->minimum() << " + " << mMapper.absTopLine()
              << " + " << mTopVisibleLine << " = " << (verticalScrollBar()->minimum() + mMapper.absTopLine() + mTopVisibleLine);
        verticalScrollBar()->setValue(verticalScrollBar()->minimum() + mMapper.absTopLine() + mTopVisibleLine);
    } else { // current line is estimated
        qreal factor= qreal(mMapper.absPos(mTopVisibleLine + mTopLine, 0)) / mMapper.fileSizeInByteBlocks();
        mEdit->blockSignals(true);
        verticalScrollBar()->setValue(qRound(verticalScrollBar()->minimum() - verticalScrollBar()->minimum() * factor));
        mEdit->blockSignals(false);
    }
}

void TextView::setVisibleTop(int lineNr)
{
    TRACE();
    if (lineNr == mTopLine+mTopVisibleLine) return;
    DEB() << "new top lineNr: " << lineNr;
    mTopLine = qBound(0, lineNr - mTopBufferLines, qAbs(mMapper.lineCount())-(mTopBufferLines*2)); // the absolute line in the file
    mTopVisibleLine = lineNr - mTopLine;
    if (lineNr <= mMapper.knownLineNrs()) { // known line number: jump directly to line
        mMapper.setTopLine(mTopLine);
    } else { // estimated line number: jump to estimated byte position
        int remain;
        int byteBlock = mMapper.absPos(mTopLine, 0, &remain);
        mMapper.setTopOffset(byteBlock, remain);
    }
    DEB() << "Set edit scroll to: " << mTopVisibleLine;
    mDocChanging = true;
    mEdit->document()->setPlainText(mMapper.lines(0, 3*mTopBufferLines));
    mEdit->verticalScrollBar()->setValue(mTopVisibleLine);
    mDocChanging = false;
    updateVScrollZone();
}

//void TextView::editScrollResized(int min, int max)
//{
//    TRACE();
//    verticalScrollBar()->setMinimum(min);
//    verticalScrollBar()->setMaximum(max);
//}

} // namespace studio
} // namespace gams
