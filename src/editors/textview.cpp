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
        connect(mm, &MemoryMapper::createMarks, this, &TextView::createMarks);
        connect(mm, &MemoryMapper::appendLines, this, &TextView::appendLines);
//        connect(mm, &MemoryMapper::appendDisplayLines, this, &TextView::appendedLines);
        connect(mm, &MemoryMapper::updateView, this, &TextView::updateView);
        mMapper = mm;
    }
    mEdit = new TextViewEdit(*mMapper, this);
    mEdit->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *lay = new QVBoxLayout(this);
    setLayout(lay);
    lay->addWidget(mEdit);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    if (kind == MemoryText)
        connect(mEdit, &TextViewEdit::textDoubleClicked, this, &TextView::textDoubleClicked);
    connect(verticalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::outerScrollAction);
    connect(mEdit->horizontalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::horizontalScrollAction);
    connect(mEdit, &TextViewEdit::keyPressed, this, &TextView::editKeyPressEvent);
//    connect(mEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::editScrollChanged);
    connect(mEdit, &TextViewEdit::selectionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::cursorPositionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    connect(mEdit, &TextViewEdit::searchFindNextPressed, this, &TextView::searchFindNextPressed);
    connect(mEdit, &TextViewEdit::searchFindPrevPressed, this, &TextView::searchFindPrevPressed);
    connect(mEdit, &TextViewEdit::hasHRef, this, &TextView::hasHRef);
    connect(mEdit, &TextViewEdit::jumpToHRef, this, &TextView::jumpToHRef);
    connect(mEdit, &TextViewEdit::topLineMoved, this, &TextView::updateView);
//    connect(mEdit, &TextViewEdit::recalcVisibleLines, this, &TextView::recalcVisibleLines);
    connect(mMapper, &AbstractTextMapper::loadAmountChanged, this, &TextView::loadAmountChanged);
    connect(mMapper, &AbstractTextMapper::blockCountChanged, this, &TextView::blockCountChanged);
    connect(mMapper, &AbstractTextMapper::blockCountChanged, this, &TextView::updateVScrollZone);
    connect(mMapper, &AbstractTextMapper::selectionChanged, this, &TextView::selectionChanged);
//    connect(mMapper, &AbstractTextMapper::contentChanged, this, &TextView::contentChanged);
    mEdit->verticalScrollBar()->installEventFilter(this);

/* --- scrollbar controlling qt-methods
    QObject::connect(control, SIGNAL(documentSizeChanged(QSizeF)), q, SLOT(_q_adjustScrollbars()));
    QPlainTextEdit::setDocument(QTextDocument *document);
    QPlainTextEditPrivate::append(const QString &text, Qt::TextFormat format);
    QPlainTextEdit::resizeEvent(QResizeEvent *e);
    QPlainTextEdit::setLineWrapMode(LineWrapMode wrap);
*/
}

TextView::~TextView()
{
    if (mStayAtTail) delete mStayAtTail;
    mStayAtTail = nullptr;
    mMapper->deleteLater();
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
    mMapper->setMappingSizes();
    recalcVisibleLines();
    if (initAnchor)
        mMapper->setVisibleTopLine(0);
    updateView();
    return true;
}

void TextView::prepareRun()
{
    mMapper->startRun();
    if (mMapper->kind() == AbstractTextMapper::memoryMapper)
        mStayAtTail = new bool(true);
    updateView();
}

void TextView::endRun()
{
    mMapper->endRun();
    if (mStayAtTail) delete mStayAtTail;
    mStayAtTail = nullptr;
    updateView();
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

bool TextView::jumpTo(int lineNr, int charNr, int length)
{
    if (lineNr > mMapper->knownLineNrs()) return false;
    int vTop = mMapper->visibleTopLine();
    int vAll = mMapper->visibleLineCount();
    if (lineNr < vTop+(vAll/3) || lineNr > vTop+(vAll*2/3)) {
        mMapper->setVisibleTopLine(qMax(0, lineNr-(vAll/3)));
        updateView();
    }
    mMapper->setPosRelative(lineNr - mMapper->visibleTopLine(), charNr + length, QTextCursor::MoveAnchor);
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
        updateView();
        emit selectionChanged();
    }
    return found;
}

void TextView::outerScrollAction(int action)
{
    if (mDocChanging) return;
    switch (action) {
    case QScrollBar::SliderSingleStepAdd: mMapper->moveVisibleTopLine(1); break;
    case QScrollBar::SliderSingleStepSub: mMapper->moveVisibleTopLine(-1); break;
    case QScrollBar::SliderPageStepAdd: mMapper->moveVisibleTopLine(mMapper->visibleLineCount()-1); break;
    case QScrollBar::SliderPageStepSub: mMapper->moveVisibleTopLine(-mMapper->visibleLineCount()+1); break;
    case QScrollBar::SliderMove: {
        int lineNr = verticalScrollBar()->sliderPosition() - verticalScrollBar()->minimum();
        int max = verticalScrollBar()->maximum() - verticalScrollBar()->minimum();
        if (mMapper->knownLineNrs() >= lineNr)
            mMapper->setVisibleTopLine(lineNr);
         else
            mMapper->setVisibleTopLine(double(lineNr) / (max));
    }
        break;
    default: break;
    }
    topLineMoved();
}

void TextView::horizontalScrollAction(int action)
{
    Q_UNUSED(action)
    mHScrollValue = mEdit->horizontalScrollBar()->sliderPosition();
}

void TextView::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    recalcVisibleLines();
}

void TextView::recalcVisibleLines()
{
    int visibleLines = mEdit->viewport()->height() / mEdit->fontMetrics().height();
    mMapper->setVisibleLineCount(visibleLines);
    updateVScrollZone();
    updateView();
}

void TextView::showEvent(QShowEvent *event)
{
    QAbstractScrollArea::showEvent(event);
    if (mInit) init();
}

void TextView::setMarks(const LineMarks *marks)
{
    mEdit->setMarks(marks);
}

const LineMarks *TextView::marks() const
{
    return mEdit->marks();
}

bool TextView::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::Wheel) {
        mMapper->moveVisibleTopLine(static_cast<QWheelEvent*>(event)->delta() / -40);
        verticalScrollBar()->setSliderPosition(qAbs(mMapper->visibleTopLine()) - verticalScrollBar()->minimum());
        verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition());
        topLineMoved();
        return true;
    }
    return false;
}

void TextView::jumpToEnd()
{
    if (mMapper->lineCount() > 0) {
        mMapper->setVisibleTopLine(mMapper->lineCount() - mMapper->visibleLineCount());
    } else
        mMapper->setVisibleTopLine(1.0);
    verticalScrollBar()->setSliderPosition(qAbs(mMapper->visibleTopLine()) - verticalScrollBar()->minimum());
    verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition());
    topLineMoved();
}

void TextView::updateView()
{
    if (mStayAtTail && *mStayAtTail) {
        jumpToEnd();
    }
    verticalScrollBar()->setSliderPosition(qAbs(mMapper->visibleTopLine()) - verticalScrollBar()->minimum());
    verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition());
    topLineMoved();
}

int TextView::firstErrorLine()
{
    MemoryMapper *memMapper = qobject_cast<MemoryMapper*>(mMapper);
    if (!memMapper) return -1;
    return memMapper->firstErrorLine();
}

void TextView::editKeyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up: mMapper->moveVisibleTopLine(-1); break;
    case Qt::Key_Down: mMapper->moveVisibleTopLine(1); break;
    case Qt::Key_PageUp: mMapper->moveVisibleTopLine(-mMapper->visibleLineCount()+1); break;
    case Qt::Key_PageDown: mMapper->moveVisibleTopLine(mMapper->visibleLineCount()-1); break;
    case Qt::Key_Home: mMapper->setVisibleTopLine(0); break;
    case Qt::Key_End:
        if (mMapper->lineCount() > 0) {
            mMapper->setVisibleTopLine(mMapper->lineCount() - mMapper->visibleLineCount() + 1);
        } else
            mMapper->setVisibleTopLine(1.0);
        break;
    default:
        event->ignore();
        break;
    }
    updateView();
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
    mInit = false;
    recalcVisibleLines();
}

void TextView::updateVScrollZone()
{
    verticalScrollBar()->setPageStep(mMapper->visibleLineCount());
    verticalScrollBar()->setMaximum(qAbs(mMapper->lineCount()) - mMapper->visibleLineCount());
    topLineMoved();
}

void TextView::topLineMoved()
{
    if (!mDocChanging) {
        if (mStayAtTail && mMapper->atTail()) *mStayAtTail = true;
        ChangeKeeper x(mDocChanging);
        mEdit->protectWordUnderCursor(true);
        mEdit->setTextCursor(QTextCursor(mEdit->document()));
        QVector<LineFormat> formats;
        mEdit->setPlainText(mMapper->lines(0, mMapper->visibleLineCount()+1, formats)); // (mMapper->atTail() ? 0 : 1)
        QTextCursor cur(mEdit->document());
        cur.select(QTextCursor::Document);
        cur.setCharFormat(QTextCharFormat());
        for (int row = 0; row < mEdit->blockCount() && row < formats.size(); ++row) {
            if (formats.at(row).start < 0) continue;
            const LineFormat &format = formats.at(row);
            QTextBlock block = mEdit->document()->findBlockByNumber(row);
            QTextCursor cursor(block);
            if (format.extraLstFormat) {
                cursor.setPosition(block.position()+3, QTextCursor::KeepAnchor);
                QTextCharFormat extraFormat = *format.extraLstFormat;
                extraFormat.setAnchor(true);
                extraFormat.setAnchorHref(format.extraLstHRef);
                cursor.setCharFormat(extraFormat);
            }
            cursor.setPosition(block.position()+format.start);
            cursor.setPosition(block.position()+format.end, QTextCursor::KeepAnchor);
            cursor.setCharFormat(format.format);
        }
        updatePosAndAnchor();
        updateVScrollZone();
        mEdit->updateExtraSelections();
        mEdit->protectWordUnderCursor(false);
        mEdit->horizontalScrollBar()->setSliderPosition(mHScrollValue);
        mEdit->horizontalScrollBar()->setValue(mEdit->horizontalScrollBar()->sliderPosition());
    }
}

TextView::TextKind TextView::textKind() const
{
    return mTextKind;
}

void TextView::setLogParser(LogParser *logParser)
{
    if (qobject_cast<MemoryMapper*>(mMapper))
        static_cast<MemoryMapper*>(mMapper)->setLogParser(logParser);
}

void TextView::setDebugMode(bool debug)
{
    if (mMapper->debugMode() != debug) {
        mMapper->setDebugMode(debug);
        if (mMapper->lineCount() > 0)
            updateView();
    }
}

void TextView::invalidate()
{
    recalcVisibleLines();
}

void TextView::reset()
{
    mMapper->reset();
}

void TextView::updatePosAndAnchor()
{
    QPoint pos = mMapper->position(true);
    QPoint anchor = mMapper->anchor(true);
    if (pos.y() < 0) return;
    if (mMapper->debugMode()) {
        pos.setY(pos.y()*2 + 1);
        anchor.setY(anchor.y()*2 + 1);
    }

    ChangeKeeper x(mDocChanging);
    QTextCursor cursor = mEdit->textCursor();
    if (anchor.y() < 0 && pos == anchor) {
        QTextBlock block = mEdit->document()->findBlockByNumber(pos.y());
        int p = block.position() + qMin(block.length(), pos.x());
        cursor.setPosition(p);
    } else {
        QTextBlock block = mEdit->document()->findBlockByNumber(anchor.y());
        int p = block.position() + qMin(block.length(), anchor.x());
        cursor.setPosition(p);
        block = mEdit->document()->findBlockByNumber(pos.y());
        p = block.position() + qMin(block.length(), pos.x());
        cursor.setPosition(p, QTextCursor::KeepAnchor);
    }
    disconnect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    mEdit->setTextCursor(cursor);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
}

QString findLstHRef(const QTextBlock &block)
{
    if (!block.isValid() || block.length() < 3) return QString();
    QTextCursor cursor(block);
    cursor.setPosition(block.position()+1);
    if (!cursor.charFormat().anchorHref().isEmpty())
        return cursor.charFormat().anchorHref();
    cursor.setPosition(block.position()+ block.length() -2);
    if (!cursor.charFormat().anchorHref().isEmpty())
        return cursor.charFormat().anchorHref();
    return QString();
}

void TextView::textDoubleClicked(const QTextCursor &cursor, bool &done)
{
    QTextBlock blockUp = cursor.block();
    QTextBlock blockDn = cursor.block();
    // while in error description, upwards
    while (blockUp.isValid()) {
        if (!blockUp.text().startsWith(" "))
            break;
        blockUp = blockUp.previous();
    }
    // while in error description, upwards
    while (blockDn.isValid()) {
        if (!blockDn.text().startsWith(" "))
            break;
        blockDn = blockDn.next();
    }
    QString href;
    while (href.isEmpty() && (blockUp.isValid() || blockDn.isValid())) {
        href = findLstHRef(blockUp);
        if (href.isEmpty()) {
            href = findLstHRef(blockDn);
            if (href.isEmpty()) {
                if (blockUp.isValid()) blockUp = blockUp.previous();
                if (blockDn.isValid()) blockDn = blockDn.next();
            }
        }
    }
    if (!href.isEmpty()) {
        done = true;
        jumpToHRef(href);
    }
}

void TextView::updateExtraSelections()
{
    mEdit->updateExtraSelections();
}

void TextView::marksChanged(const QSet<int> dirtyLines)
{
    mEdit->marksChanged(dirtyLines);
}


} // namespace studio
} // namespace gams
