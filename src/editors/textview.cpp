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
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);

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
    int count = (mMapper.lineCount() < 0) ? mTopBufferLines*3 : mMapper.lineCount();
    mMapper.setMappingSizes(count);
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

QPoint TextView::position() const
{
    return mMapper.position();
}

QPoint TextView::anchor() const
{
    return mMapper.anchor();
}

//int TextView::findLine(int lineNr)
//{
//    if (mMapper.lineCount() >= 0 || lineNr < mMapper.knownLineNrs()) {
//        if (mMapper.setVisibleTopLine(qMax(0, lineNr - mVisibleLines/2))) return lineNr;
//    }
//    mTransferedLineAmount = 0;
//    mLineToFind = lineNr;
//    return mMapper.knownLineNrs();
//}

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
    case QScrollBar::SliderSingleStepSub:
        mMapper.moveVisibleTopLine(-1);
        break;
    case QScrollBar::SliderSingleStepAdd:
        mMapper.moveVisibleTopLine(1);
        break;
    case QScrollBar::SliderPageStepSub:
        mMapper.moveVisibleTopLine(-mVisibleLines+1);
        break;
    case QScrollBar::SliderPageStepAdd:
        mMapper.moveVisibleTopLine(mVisibleLines-1);
        break;
    case QScrollBar::SliderMove: {
        int lineNr = verticalScrollBar()->value() - verticalScrollBar()->minimum();
        DEB() << " lineNr: " << lineNr;
        if (mMapper.knownLineNrs() >= lineNr) {
            mMapper.setVisibleTopLine(lineNr);
        } else {
            double region = double(lineNr) / (verticalScrollBar()->maximum()-verticalScrollBar()->minimum());
            mMapper.setVisibleTopLine(region);
        }
    }
        break;
    default:
        break;
    }
    topLineMoved();
    mActiveScrollAction = QScrollBar::SliderNoAction;
}

void TextView::cursorPositionChanged()
{
    if (mDocChanging) return;
    TRACE();
    QTextCursor localPos = mEdit->textCursor();
    QTextCursor::MoveMode mode = localPos.hasSelection() ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
    mMapper.setRelPos(localPos.blockNumber(), localPos.positionInBlock(), mode);
}

void TextView::editScrollChanged()
{
    if (mDocChanging) return;
    TRACE();
    // TODO(JM) need to store the current local to determine the offset
    int lineDelta = 0;
    mMapper.moveTopLine(lineDelta);
//    mMapper.setVisibleTopLine(mTopLine + mEdit->verticalScrollBar()->value());
}

void TextView::scrollContentsBy(int dx, int dy)
{
//    if (!mDocChanging) {
//        TRACE();
//        mMapper.moveTopLine(mEdit->verticalScrollBar()->value()-mTopVisibleLine);
//        topLineMoved();
////        setVisibleTop(mTopPos.cachedLine + mEdit->verticalScrollBar()->value());
//    }
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
    switch (event->key()) {
    case Qt::Key_Up:
        mMapper.moveVisibleTopLine(-1);
        break;
    case Qt::Key_Down:
        mMapper.moveVisibleTopLine(1);
        break;
    case Qt::Key_PageUp:
        mMapper.moveVisibleTopLine(-mVisibleLines+1);
        break;
    case Qt::Key_PageDown:
        mMapper.moveVisibleTopLine(mVisibleLines-1);
        break;
    default:
        event->ignore();
        break;
    }
    topLineMoved();
}

void TextView::selectionChanged()
{
    if (mDocChanging) return;
    TRACE();
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
    int lineCount = mMapper.lineCount();
    if (lineCount < 0) { // estimated lines count
        verticalScrollBar()->setMinimum(qMin(lineCount+mMapper.visibleOffset(), 0));
        verticalScrollBar()->setMaximum(0);
    } else { // known lines count
        verticalScrollBar()->setMinimum(0);
        verticalScrollBar()->setMaximum(qMax(lineCount-mMapper.visibleOffset(), 0));
    }
    syncVScroll();
}

void TextView::syncVScroll()
{
    TRACE();
    mEdit->blockSignals(true);
    if (mMapper.absTopLine() >= 0) { // current line is known
        verticalScrollBar()->setValue(verticalScrollBar()->minimum() + mMapper.absTopLine() + mMapper.visibleOffset());
    } else { // current line is estimated
        qreal factor= qreal(qAbs(mMapper.absTopLine()) + mMapper.visibleOffset()) / qAbs(mMapper.lineCount());
        verticalScrollBar()->setValue(qRound(verticalScrollBar()->minimum() - verticalScrollBar()->minimum() * factor));
    }
    mEdit->blockSignals(false);
}

void TextView::topLineMoved()
{
    TRACE();
    if (!mDocChanging) {
        ChangeKeeper x(mDocChanging);
        mEdit->setPlainText(mMapper.lines(0, 3*mTopBufferLines));
        updatePosAndAnchor();
        mEdit->blockSignals(true);
        mEdit->verticalScrollBar()->setValue(mMapper.visibleOffset());
        mEdit->blockSignals(false);
        updateVScrollZone();
    }
}

void TextView::updatePosAndAnchor()
{
    TRACE();
    QPoint pos = mMapper.position();
    QPoint anchor = mMapper.anchor();
    if (pos.x() < 0) {
        return;
//        mEdit->setTextCursor(QTextCursor());
    } else {
        int scrollPos = mEdit->verticalScrollBar()->value();
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
        mEdit->verticalScrollBar()->setValue(scrollPos);
    }
}


} // namespace studio
} // namespace gams
