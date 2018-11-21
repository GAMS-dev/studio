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


TextView::TextView(QWidget *parent) : QAbstractScrollArea(parent)
{
    setViewportMargins(0,0,0,0);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mEdit = new TextViewEdit(mMapper, this);
    mEdit->setReadOnly(true);
    mEdit->setFrameShape(QFrame::NoFrame);
    mEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//    mEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QVBoxLayout *lay = new QVBoxLayout(this);
    setLayout(lay);
    lay->addWidget(mEdit);

//    mEdit->setCursorWidth(0);
    connect(verticalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::outerScrollAction);
//    connect(mEdit, &TextViewEdit::cursorPositionChanged, this, &TextView::cursorPositionChanged);
    connect(mEdit, &TextViewEdit::keyPressed, this, &TextView::editKeyPressEvent);
    connect(mEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::editScrollChanged);
    connect(mEdit, &TextViewEdit::selectionChanged, this, &TextView::selectionChanged);
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
    ChangeKeeper x(mDocChanging);
    mEdit->setPlainText(mMapper.lines(0, count));
    mEdit->setTextCursor(QTextCursor(mEdit->document()));
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
    TRACE();
    DEB() << "action: " << action;
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
//        mEdit->verticalScrollBar()->triggerAction(mActiveScrollAction);
        setVisibleTop(verticalScrollBar()->value() - verticalScrollBar()->minimum());
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

void TextView::cursorPositionChanged()
{
    if (!mDocChanging) {
        QTextCursor localPos = mEdit->textCursor();
        QTextCursor::MoveMode mode = localPos.hasSelection() ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
        mMapper.setRelPos(localPos.blockNumber(), localPos.positionInBlock(), mode);
    }
}

void TextView::editScrollChanged()
{
    TRACE();
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

void TextView::focusInEvent(QFocusEvent *event)
{
    mEdit->setFocus();
}

void TextView::editKeyPressEvent(QKeyEvent *event)
{
    TRACE();
    QPoint pos;
    QPoint anchor;
    event->accept();
    mMapper.getPosAndAnchor(pos, anchor);
    switch (event->key()) {
    case Qt::Key_Up:
        pos.ry() -= 1;
        break;
    case Qt::Key_Down:
        pos.ry() += 1;
        break;
    case Qt::Key_PageUp:
        pos.ry() -= mVisibleLines;
        break;
    case Qt::Key_PageDown:
        pos.ry() += mVisibleLines;
        break;
    default:
        event->ignore();
        break;
    }
    if (mTopVisibleLine + mTopLine > pos.y()) {
        setVisibleTop(qMin(0, pos.y() - (mVisibleLines/3)));
    } else if (mTopVisibleLine + mTopLine + mVisibleLines < pos.y()) {
        setVisibleTop(qMin(0, pos.y() - (mVisibleLines * 2/3)));
    }
    QTextCursor::MoveMode mode = (event->modifiers() & Qt::ShiftModifier) ? QTextCursor::KeepAnchor
                                                                          : QTextCursor::MoveAnchor;
    mMapper.setRelPos(pos.y()-mTopLine, pos.x(), mode);
    mMapper.getPosAndAnchor(pos, anchor);
    updatePosAndAnchor(pos, anchor);
}

void TextView::selectionChanged()
{
    if (mDocChanging) return;
    QTextCursor cur = mEdit->textCursor();
    if (cur.hasSelection()) {
        if (!mMapper.hasSelection()) {
            QTextCursor anc = mEdit->textCursor();
            anc.setPosition(cur.anchor());
            mMapper.setRelPos(anc.blockNumber(), anc.positionInBlock());
        }
        mMapper.setRelPos(cur.blockNumber(), cur.positionInBlock(), QTextCursor::KeepAnchor);
    } else {
        mMapper.setRelPos(cur.blockNumber(), cur.positionInBlock());
    }
}

void TextView::init()
{
    layout()->setContentsMargins(0,0,verticalScrollBar()->width(),0);
    mEdit->setFocus();
    mInit = false;
}

void TextView::updateVScrollZone()
{
//    TRACE();
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
//    TRACE();
    if (mMapper.absTopLine() >= 0) { // current line is known
//        DEB() << "syncVScroll: " << verticalScrollBar()->minimum() << " + " << mMapper.absTopLine()
//              << " + " << mTopVisibleLine << " = " << (verticalScrollBar()->minimum() + mMapper.absTopLine() + mTopVisibleLine);
        verticalScrollBar()->setValue(verticalScrollBar()->minimum() + mMapper.absTopLine() + mTopVisibleLine);
    } else { // current line is estimated
        qreal factor= qreal(mMapper.absPos(mTopVisibleLine + mTopLine, 0)) / mMapper.fileSizeInByteBlocks();
        mEdit->blockSignals(true);
        verticalScrollBar()->setValue(qRound(verticalScrollBar()->minimum() - verticalScrollBar()->minimum() * factor));
        mEdit->blockSignals(false);
    }
}

void TextView::updatePosAndAnchor(QPoint &pos, QPoint &anchor)
{
    cropPosition(pos);
    cropPosition(anchor);
    if (pos.x() < 0) {
        return;
//        mEdit->setTextCursor(QTextCursor());
    } else {
        ChangeKeeper x(mDocChanging);
        QTextCursor cursor = mEdit->textCursor();
        if (anchor.x() < 0 && pos == anchor) {
            QTextBlock block = mEdit->document()->findBlockByNumber(pos.y()-mTopLine);
            int p = block.position() + qMin(block.length()-1, pos.x());
            cursor.setPosition(p);
        } else {
            QTextBlock block = mEdit->document()->findBlockByNumber(anchor.y()-mTopLine);
            int p = block.position() + qMin(block.length()-1, anchor.x());
            cursor.setPosition(p);
            block = mEdit->document()->findBlockByNumber(pos.y()-mTopLine);
            p = block.position() + qMin(block.length()-1, pos.x());
            cursor.setPosition(p, QTextCursor::KeepAnchor);
        }
        mEdit->setTextCursor(cursor);
    }
}

void TextView::setVisibleTop(int lineNr)
{
    TRACE();
    if (lineNr == mTopLine+mTopVisibleLine) return;
    DEB() << "new top lineNr: " << lineNr;
    mTopLine = qBound(0, lineNr - mTopBufferLines, qAbs(mMapper.lineCount())-(mTopBufferLines*3)); // the absolute line in the file
    mTopVisibleLine = lineNr - mTopLine;
    if (lineNr <= mMapper.knownLineNrs()) { // known line number: jump directly to line
        mMapper.setTopLine(mTopLine);
    } else { // estimated line number: jump to estimated byte position
        int remain;
        int byteBlock = mMapper.absPos(mTopLine, 0, &remain);
        mMapper.setTopOffset(byteBlock, remain);
    }
    DEB() << "Set edit scroll to: " << mTopVisibleLine;
    {
        ChangeKeeper x(mDocChanging);
        QPoint pos;
        QPoint anchor;
        mMapper.getPosAndAnchor(pos, anchor);
//        DEB() << " pos: " << pos << " anchor: " << anchor << " mTopLine: " << mTopLine;
        mEdit->document()->setPlainText(mMapper.lines(0, 3*mTopBufferLines));
        updatePosAndAnchor(pos, anchor);
        mEdit->verticalScrollBar()->setValue(mTopVisibleLine);
    }
    updateVScrollZone();
}

void TextView::cropPosition(QPoint &pos)
{
    if (pos.x() < 0) return;
    int y = qBound(mTopLine, pos.y(), qMin(qAbs(mMapper.lineCount())-1, 3*mTopBufferLines + mTopLine-1));
    if (y < pos.y()) {
        DEB() << "upper-cut";
        pos.setY(y);
        pos.setX(mEdit->document()->lastBlock().length()-1);
    } else if (y > pos.y()) {
        DEB() << "lower-cut";
        pos.setY(y);
        pos.setX(0);
    }
}

//void TextView::editScrollResized(int min, int max)
//{
//    TRACE();
//    verticalScrollBar()->setMinimum(min);
//    verticalScrollBar()->setMaximum(max);
//}

} // namespace studio
} // namespace gams
