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
#include "textviewedit.h"

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
    connect(mEdit, &TextViewEdit::keyPressed, this, &TextView::editKeyPressEvent);
    connect(mEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::editScrollChanged);
    connect(mEdit, &TextViewEdit::selectionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    mPeekTimer.setSingleShot(true);
    connect(&mPeekTimer, &QTimer::timeout, this, &TextView::peekMoreLines);
    mEdit->verticalScrollBar()->setVisible(false);


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
    int count = (mMapper.lineCount() < 0) ? mTopBufferLines*3 : mMapper.lineCount();
    mMapper.setMappingSizes(count);
    ChangeKeeper x(mDocChanging);
    mEdit->setPlainText(mMapper.lines(0, count));
    mEdit->setTextCursor(QTextCursor(mEdit->document()));
    mPeekTimer.start(100);
}

qint64 TextView::fileSize() const
{
    return mMapper.size();
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

int TextView::knownLines() const
{
    return mMapper.knownLineNrs();
}

void TextView::copySelection()
{
    mEdit->copySelection();
}

void TextView::selectAllText()
{
    mEdit->selectAllText();
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
    // keep timer alive
    if (amount.part < amount.all) mPeekTimer.start(50);
    emit loadAmountChanged();
    emit blockCountChanged(lineCount());
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

void TextView::editScrollChanged()
{
    if (mDocChanging) return;
    int lineDelta = mEdit->verticalScrollBar()->value() - mMapper.visibleOffset();
    mMapper.moveVisibleTopLine(lineDelta);
    topLineMoved();
}

void TextView::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    mVisibleLines = (mEdit->height() - mEdit->contentsMargins().top() - mEdit->contentsMargins().bottom())
            / mEdit->fontMetrics().height();
    mMapper.setVisibleLineCount(mVisibleLines);
    updateVScrollZone();
}

void TextView::showEvent(QShowEvent *event)
{
    QAbstractScrollArea::showEvent(event);
    if (mInit) init();
}

void TextView::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    mEdit->setFocus();
}

void TextView::setMarks(const LineMarks *marks)
{
    mEdit->setMarks(marks);
}

const LineMarks *TextView::marks() const
{
    return mEdit->marks();
}

void TextView::editKeyPressEvent(QKeyEvent *event)
{
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
    case Qt::Key_Home:
        mMapper.setVisibleTopLine(0.0);
        break;
    case Qt::Key_End:
        mMapper.setVisibleTopLine(1.0);
        break;
    default:
        event->ignore();
        break;
    }
    topLineMoved();
}

void TextView::handleSelectionChange()
{
    if (mDocChanging) return;
    QTextCursor cur = mEdit->textCursor();
    if (cur.hasSelection()) {
        if (!mMapper.hasSelection()) {
            QTextCursor anc = mEdit->textCursor();
            anc.setPosition(cur.anchor());
            mMapper.setPosRelative(anc.blockNumber(), anc.positionInBlock());
        }
        mMapper.setPosRelative(cur.blockNumber(), cur.positionInBlock(), QTextCursor::KeepAnchor);
    } else {
        mMapper.setPosRelative(cur.blockNumber(), cur.positionInBlock());
    }
    emit selectionChanged();
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
        verticalScrollBar()->setMinimum(qMin(lineCount+mVisibleLines-1, 0));
        verticalScrollBar()->setMaximum(0);
    } else { // known lines count
        verticalScrollBar()->setMinimum(0);
        verticalScrollBar()->setMaximum(qMax(lineCount-mVisibleLines+1, 0));
    }
    syncVScroll();
}

void TextView::syncVScroll()
{
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
    if (!mDocChanging) {
        ChangeKeeper x(mDocChanging);
        mEdit->protectWordUnderCursor(true);
        mEdit->setPlainText(mMapper.lines(0, 3*mTopBufferLines));
        updatePosAndAnchor();
        mEdit->blockSignals(true);
        mEdit->verticalScrollBar()->setValue(mMapper.visibleOffset());
        mEdit->blockSignals(false);
        updateVScrollZone();
        mEdit->updateExtraSelections();
        mEdit->protectWordUnderCursor(false);
    }
}

void TextView::updatePosAndAnchor()
{
    QPoint pos = mMapper.position(true);
    QPoint anchor = mMapper.anchor(true);
    if (pos.y() < 0) {
        return;
    } else {
        int scrollPos = mEdit->verticalScrollBar()->value();
        ChangeKeeper x(mDocChanging);
        QTextCursor cursor = mEdit->textCursor();
        if (anchor.y() < 0 && pos == anchor) {
            QTextBlock block = mEdit->document()->findBlockByNumber(pos.y());
            int p = block.position() + qMin(block.length()-1, pos.x());
            cursor.setPosition(p);
        } else {
            QTextBlock block = mEdit->document()->findBlockByNumber(anchor.y());
            int p = block.position() + qMin(block.length()-1, anchor.x());
            cursor.setPosition(p);
            block = mEdit->document()->findBlockByNumber(pos.y());
            p = block.position() + qMin(block.length()-1, pos.x());
            cursor.setPosition(p, QTextCursor::KeepAnchor);
        }
        mEdit->setTextCursor(cursor);
        mEdit->verticalScrollBar()->setValue(scrollPos);
    }
}


} // namespace studio
} // namespace gams
