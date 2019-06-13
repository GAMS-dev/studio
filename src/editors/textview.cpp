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
#include "filemapper.h"
#include "memorymapper.h"
#include "logger.h"
#include "exception.h"
#include "textviewedit.h"
#include "keys.h"

#include <QScrollBar>
#include <QTextBlock>
#include <QPlainTextDocumentLayout>
#include <QBoxLayout>

namespace gams {
namespace studio {


TextView::TextView(TextKind kind, QWidget *parent) : QAbstractScrollArea(parent), mTextKind(kind)
{
    setViewportMargins(0,0,0,0);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    setFocusPolicy(Qt::NoFocus);
    if (kind == FileText) {
        mMapper = new FileMapper();
    }
    if (kind == MemoryText) {
        MemoryMapper* mm = new MemoryMapper();
        connect(this, &TextView::addProcessData, mm, &MemoryMapper::addProcessData);
        mMapper = mm;
        mMapper->setMappingSizes();
        mLinesAddedTimer.setSingleShot(true);
        mLinesAddedTimer.setInterval(0);
        connect(&mLinesAddedTimer, &QTimer::timeout, this, &TextView::contentChanged);
        connect(mm, &MemoryMapper::contentChanged, this, &TextView::contentChanged);
    }
    mEdit = new TextViewEdit(*mMapper, this);
    mEdit->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *lay = new QVBoxLayout(this);
    setLayout(lay);
    lay->addWidget(mEdit);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(verticalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::outerScrollAction);

    connect(mEdit->horizontalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::horizontalScrollAction);
    connect(mEdit, &TextViewEdit::keyPressed, this, &TextView::editKeyPressEvent);
    connect(mEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::editScrollChanged);
    connect(mEdit, &TextViewEdit::selectionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::cursorPositionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    connect(mEdit, &TextViewEdit::searchFindNextPressed, this, &TextView::searchFindNextPressed);
    connect(mEdit, &TextViewEdit::searchFindPrevPressed, this, &TextView::searchFindPrevPressed);
    connect(mEdit, &TextViewEdit::hasHRef, this, &TextView::hasHRef);
    connect(mEdit, &TextViewEdit::jumpToHRef, this, &TextView::jumpToHRef);
    connect(mMapper, &AbstractTextMapper::loadAmountChanged, this, &TextView::loadAmountChanged);
    connect(mMapper, &AbstractTextMapper::blockCountChanged, this, &TextView::blockCountChanged);
    connect(mMapper, &AbstractTextMapper::selectionChanged, this, &TextView::selectionChanged);
    connect(mMapper, &AbstractTextMapper::linesAdded, this, &TextView::mapperLinesAdded);
//    connect(mMapper, &AbstractTextMapper::contentChanged, this, &TextView::contentChanged);

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
    return mMapper->lineCount();
}

bool TextView::loadFile(const QString &fileName, int codecMib, bool initAnchor)
{
    if (mTextKind != FileText) return false;
    if (codecMib == -1) codecMib = QTextCodec::codecForLocale()->mibEnum();
    mMapper->setCodec(codecMib == -1 ? QTextCodec::codecForMib(codecMib) : QTextCodec::codecForLocale());

    if (!static_cast<FileMapper*>(mMapper)->openFile(fileName, initAnchor)) return false;
    updateVScrollZone();
    mMapper->setMappingSizes();
    if (initAnchor)
        mMapper->setVisibleTopLine(0);
    topLineMoved();
    return true;
}

void TextView::prepareRun()
{
    mMapper->startRun();
    ChangeKeeper x(mDocChanging);
    mEdit->clear();
    topLineMoved();
}

void TextView::endRun()
{
    mMapper->endRun();
    // TODO(JM) if TextMarks left to create (errors from log-tail), do it here
}

qint64 TextView::size() const
{
    return mMapper->size();
}

void TextView::zoomIn(int range)
{
    mEdit->zoomIn(range);
    recalcVisibleLines();
}

void TextView::zoomOut(int range)
{
    mEdit->zoomOut(range);
    recalcVisibleLines();
}

bool TextView::jumpTo(int lineNr, int charNr)
{
    if (lineNr > mMapper->knownLineNrs()) return false;
    int vTop = mMapper->absTopLine()+mMapper->visibleOffset();
    int vAll = mMapper->visibleLineCount();
    if (lineNr < vTop+(vAll/3) || lineNr > vTop+(vAll*2/3)) {
        mMapper->setVisibleTopLine(qMax(0, lineNr-(vAll/3)));
        topLineMoved();
        vTop = mMapper->absTopLine()+mMapper->visibleOffset();
    }
    mMapper->setPosRelative(lineNr - mMapper->absTopLine(), charNr);
    updatePosAndAnchor();
    emit selectionChanged();
    setFocus();
    return true;
}

QPoint TextView::position() const
{
    return mMapper->position();
}

QPoint TextView::anchor() const
{
    return mMapper->anchor();
}

bool TextView::hasSelection() const
{
    return mMapper->hasSelection();
}

int TextView::knownLines() const
{
    return mMapper->knownLineNrs();
}

void TextView::copySelection()
{
    mEdit->copySelection();
}

QString TextView::selectedText() const
{
    return mMapper->selectedText();
}

void TextView::selectAllText()
{
    mEdit->selectAllText();
}

AbstractEdit *TextView::edit()
{
    return mEdit;
}

void TextView::setLineWrapMode(QPlainTextEdit::LineWrapMode mode)
{
    if (mode == QPlainTextEdit::WidgetWidth)
        DEB() << "Line wrapping is currently unsupported.";
    mEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
}

bool TextView::findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind)
{
    bool found = mMapper->findText(searchRegex, flags, continueFind);
    if (found) {
        mMapper->scrollToPosition();
        topLineMoved();
        updatePosAndAnchor();
    }
    return found;
}

void TextView::outerScrollAction(int action)
{
    DEB() << "Scrolled " << verticalScrollBar()->sliderPosition() << "  [" << verticalScrollBar()->minimum() << "," << verticalScrollBar()->maximum() << "]";
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

void TextView::horizontalScrollAction(int action)
{
    Q_UNUSED(action)
    mHScrollValue = mEdit->horizontalScrollBar()->sliderPosition();
}

void TextView::adjustOuterScrollAction()
{
    switch (mActiveScrollAction) {
    case QScrollBar::SliderSingleStepSub:
        mMapper->moveVisibleTopLine(-1);
        break;
    case QScrollBar::SliderSingleStepAdd:
        mMapper->moveVisibleTopLine(1);
        break;
    case QScrollBar::SliderPageStepSub:
        mMapper->moveVisibleTopLine(-mVisibleLines+1);
        break;
    case QScrollBar::SliderPageStepAdd:
        mMapper->moveVisibleTopLine(mVisibleLines-1);
        break;
    case QScrollBar::SliderMove: {
        int lineNr = verticalScrollBar()->sliderPosition() - verticalScrollBar()->minimum();
        if (mMapper->knownLineNrs() >= lineNr) {
            mMapper->setVisibleTopLine(lineNr);
        } else {
            double region = double(lineNr) / (verticalScrollBar()->maximum()-verticalScrollBar()->minimum());
            mMapper->setVisibleTopLine(region);
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
    int lineDelta = mEdit->verticalScrollBar()->sliderPosition() - mMapper->visibleOffset();
    mMapper->moveVisibleTopLine(lineDelta);
    topLineMoved();
}

void TextView::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    recalcVisibleLines();
}

void TextView::recalcVisibleLines()
{
    mVisibleLines = (mEdit->height() - mEdit->contentsMargins().top() - mEdit->contentsMargins().bottom())
            / mEdit->fontMetrics().height();
    mMapper->setVisibleLineCount(mVisibleLines);
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
        mMapper->moveVisibleTopLine(-1);
        break;
    case Qt::Key_Down:
        mMapper->moveVisibleTopLine(1);
        break;
    case Qt::Key_PageUp:
        mMapper->moveVisibleTopLine(-mVisibleLines+1);
        break;
    case Qt::Key_PageDown:
        mMapper->moveVisibleTopLine(mVisibleLines-1);
        break;
    case Qt::Key_Home:
        mMapper->setVisibleTopLine(0);
        break;
    case Qt::Key_End:
        mMapper->setVisibleTopLine(1.0);
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
        if (!mMapper->hasSelection()) {
            QTextCursor anc = mEdit->textCursor();
            anc.setPosition(cur.anchor());
            mMapper->setPosRelative(anc.blockNumber(), anc.positionInBlock());
        }
        mMapper->setPosRelative(cur.blockNumber(), cur.positionInBlock(), QTextCursor::KeepAnchor);
    } else {
        mMapper->setPosRelative(cur.blockNumber(), cur.positionInBlock());
    }
    emit selectionChanged();
}

void TextView::init()
{
    layout()->setContentsMargins(0, 0, verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0, 0);
    mEdit->setFocus();
    mInit = false;
    recalcVisibleLines();
}

void TextView::updateVScrollZone()
{
    int count = mMapper->lineCount();
    DEB() << "LineCount: " << count;
    verticalScrollBar()->setPageStep(mVisibleLines);
    if (count < 0) { // estimated lines count
        verticalScrollBar()->setMinimum(qMin(count+mVisibleLines-1, 0));
        verticalScrollBar()->setMaximum(0);
    } else { // known lines count
        verticalScrollBar()->setMinimum(0);
        verticalScrollBar()->setMaximum(qMax(count-mVisibleLines+1, 0));
    }
    syncVScroll();
}

void TextView::syncVScroll()
{
    mEdit->blockSignals(true);
    if (mMapper->absTopLine() >= 0) { // current line is known
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum() + mMapper->absTopLine() + mMapper->visibleOffset());
    } else { // current line is estimated
        qreal factor= qreal(qAbs(mMapper->absTopLine()) + mMapper->visibleOffset()) / qAbs(lineCount());
        verticalScrollBar()->setSliderPosition(qRound(verticalScrollBar()->minimum() - verticalScrollBar()->minimum() * factor));
    }
    verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition());
    mEdit->blockSignals(false);
}

void TextView::topLineMoved()
{
    if (!mDocChanging) {
        DEB() << "visibleLineCount: " << mMapper->visibleLineCount() << " visibleOffset: " << mMapper->visibleOffset();
        ChangeKeeper x(mDocChanging);
        mEdit->setTextCursor(QTextCursor(mEdit->document()));
        mEdit->protectWordUnderCursor(true);
        QVector<LineFormat> formats;
        mEdit->setPlainText(mMapper->lines(0, 3*mMapper->visibleLineCount(), formats));
        QTextCursor cur(mEdit->document());
        cur.select(QTextCursor::Document);
        cur.setCharFormat(QTextCharFormat());
        for (int row = 0; row < mEdit->blockCount() && row < formats.size(); ++row) {
            if (formats.at(row).start < 0) continue;
            const LineFormat &format = formats.at(row);
            QTextBlock block = mEdit->document()->findBlockByNumber(row);
            QTextCursor cursor(block);
            cursor.setPosition(block.position()+format.start);
            cursor.setPosition(block.position()+format.end, QTextCursor::KeepAnchor);
            cursor.setCharFormat(format.format);
        }
        updatePosAndAnchor();
        mEdit->blockSignals(true);
        mEdit->verticalScrollBar()->setSliderPosition(mMapper->visibleOffset());
        mEdit->verticalScrollBar()->setValue(mEdit->verticalScrollBar()->sliderPosition());
//        mEdit->verticalScrollBar()->setValue(mMapper->visibleOffset()); // workaround: isn't set correctly on the first time
        mEdit->blockSignals(false);
        updateVScrollZone();
        mEdit->updateExtraSelections();
        mEdit->protectWordUnderCursor(false);
        mEdit->horizontalScrollBar()->setSliderPosition(mHScrollValue);
        mEdit->horizontalScrollBar()->setValue(mEdit->horizontalScrollBar()->sliderPosition());
//        mEdit->horizontalScrollBar()->setValue(mHScrollValue);  // workaround: isn't set correctly on the first time
//        DEB() << "mapper lines = " << mMapper->lineCount() << "   scroll lines = " << verticalScrollBar()->maximum();
    }
}

void TextView::topLineMoved(int offset)
{
    if (qAbs(offset) > 10) {
        topLineMoved();
        return;
    }
    // Test if this method is faster than the default one which always replaces all text in mEdit
    if (offset < 0) {
        QTextCursor cur(mEdit->document());
        cur.insertText(mMapper->lines(0, offset));
        // TODO(JM) adjust when to remove lines (top-line dependant)
        if (mEdit->document()->blockCount() > mMapper->bufferedLines()) {
            cur = QTextCursor(mEdit->document()->findBlockByNumber(mMapper->bufferedLines()));
            cur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cur.removeSelectedText();
        }
    } else {
        DEB() << "some new lines " << offset;
        QTextCursor cur(mEdit->document());
        cur.movePosition(QTextCursor::End);
        // TODO(JM) adjust when to remove lines (top-line dependant)
        cur.insertText(mMapper->lines(cur.blockNumber(), offset));
        if (mEdit->document()->blockCount() > mMapper->bufferedLines()) {
            cur = QTextCursor(mEdit->document()->findBlockByNumber(
                                  mMapper->bufferedLines() - mEdit->document()->blockCount()));
            cur.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            DEB() << "REMOVED:\n" << cur.selectedText();
            cur.removeSelectedText();
            DEB() << "TOPLINE:\n" << mEdit->document()->firstBlock().text();
        }
    }
    updatePosAndAnchor();
    mEdit->blockSignals(true);
    mEdit->verticalScrollBar()->setSliderPosition(mMapper->visibleOffset());
    mEdit->verticalScrollBar()->setValue(mEdit->verticalScrollBar()->sliderPosition());
//        mEdit->verticalScrollBar()->setValue(mMapper->visibleOffset()); // workaround: isn't set correctly on the first time
    mEdit->blockSignals(false);
    updateVScrollZone();
    mEdit->updateExtraSelections();
    mEdit->protectWordUnderCursor(false);
    mEdit->horizontalScrollBar()->setSliderPosition(mHScrollValue);
    mEdit->horizontalScrollBar()->setValue(mEdit->horizontalScrollBar()->sliderPosition());
}

TextView::TextKind TextView::textKind() const
{
    return mTextKind;
}

void TextView::setLogParser(LogParser *logParser)
{
    if (qobject_cast<MemoryMapper*>(mMapper))
        qobject_cast<MemoryMapper*>(mMapper)->setLogParser(logParser);
}

void TextView::setDebugMode(bool debug)
{
    if (mMapper->debugMode() != debug) {
        mMapper->setDebugMode(debug);
    }
}

void TextView::updatePosAndAnchor()
{
    QPoint pos = mMapper->position(true);
    QPoint anchor = mMapper->anchor(true);
    if (pos.y() < 0) return;

    int scrollPos = mEdit->verticalScrollBar()->sliderPosition();
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
    disconnect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    mEdit->setTextCursor(cursor);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    mEdit->verticalScrollBar()->setSliderPosition(scrollPos);
    mEdit->verticalScrollBar()->setValue(mEdit->verticalScrollBar()->sliderPosition());
}

void TextView::mapperLinesAdded(int lineAddCount)
{
    mLinesAddedCount += lineAddCount;
    mStayAtTail = (mEdit->verticalScrollBar()->sliderPosition() >= mEdit->verticalScrollBar()->maximum()-2);
    if (!mLinesAddedTimer.isActive())
        mLinesAddedTimer.start();
}

void TextView::updateExtraSelections()
{
    mEdit->updateExtraSelections();
}

void TextView::contentChanged()
{
    int offset = 0;
    if (mLinesAddedCount) {
        // TODO(JM) implement contentChanged()
        if (mStayAtTail)
            mMapper->moveVisibleTopLine(mLinesAddedCount);
        DEB() << "mLinesAddedCount " << mLinesAddedCount;
        offset = mLinesAddedCount;
        mLinesAddedCount = 0;
        // 1. if position at end -> keep at end
        // 2. if position near end -> keep position BUT modify doc in the background
        // later: 3. if a unit has been folded -> adapt top-line
        topLineMoved();
    }

}

void TextView::sendAddedLines()
{
    // TODO(JM) use this to adapt editors document
//    if (mMapper->)
}

void TextView::marksChanged(const QSet<int> dirtyLines)
{
    mEdit->marksChanged(dirtyLines);
}


} // namespace studio
} // namespace gams
