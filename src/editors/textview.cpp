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
        connect(mm, &MemoryMapper::contentChanged, this, &TextView::contentChanged);
        connect(mm, &MemoryMapper::createMarks, this, &TextView::createMarks);
        connect(mm, &MemoryMapper::appendLines, this, &TextView::appendLines);
        connect(mm, &MemoryMapper::appendDisplayLines, this, &TextView::appendedLines);
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
    connect(mEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::editScrollChanged);
    connect(mEdit, &TextViewEdit::selectionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::cursorPositionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    connect(mEdit, &TextViewEdit::searchFindNextPressed, this, &TextView::searchFindNextPressed);
    connect(mEdit, &TextViewEdit::searchFindPrevPressed, this, &TextView::searchFindPrevPressed);
    connect(mEdit, &TextViewEdit::hasHRef, this, &TextView::hasHRef);
    connect(mEdit, &TextViewEdit::jumpToHRef, this, &TextView::jumpToHRef);
    connect(mEdit, &TextViewEdit::recalcVisibleLines, this, &TextView::recalcVisibleLines);
    connect(mMapper, &AbstractTextMapper::loadAmountChanged, this, &TextView::loadAmountChanged);
    connect(mMapper, &AbstractTextMapper::blockCountChanged, this, &TextView::blockCountChanged);
    connect(mMapper, &AbstractTextMapper::selectionChanged, this, &TextView::selectionChanged);
//    connect(mMapper, &AbstractTextMapper::contentChanged, this, &TextView::contentChanged);

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
    updateVScrollZone();
    mMapper->setMappingSizes();
    recalcVisibleLines();
    if (initAnchor)
        mMapper->setVisibleTopLine(0);
    topLineMoved();
    return true;
}

void TextView::prepareRun()
{
    mMapper->startRun();
    topLineMoved();
}

void TextView::endRun()
{
    mMapper->endRun();
    topLineMoved();
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
    int vTop = mMapper->absTopLine()+mMapper->visibleOffset();
    int vAll = mMapper->visibleLineCount();
    if (lineNr < vTop+(vAll/3) || lineNr > vTop+(vAll*2/3)) {
        mMapper->setVisibleTopLine(qMax(0, lineNr-(vAll/3)));
        topLineMoved();
        vTop = mMapper->absTopLine()+mMapper->visibleOffset();
    }
    mMapper->setPosRelative(lineNr - mMapper->absTopLine(), charNr + length, QTextCursor::MoveAnchor);
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
        emit selectionChanged();
    }
    return found;
}

void TextView::outerScrollAction(int action)
{
    if (mDocChanging) return;
//    DEB() << "Scrolled " << verticalScrollBar()->sliderPosition() << "  [" << verticalScrollBar()->minimum() << "," << verticalScrollBar()->maximum() << "]";
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
        mMapper->moveVisibleTopLine(-mMapper->visibleLineCount()+1);
        break;
    case QScrollBar::SliderPageStepAdd:
        mMapper->moveVisibleTopLine(mMapper->visibleLineCount()-1);
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
//    recalcVisibleLines();
}

void TextView::recalcVisibleLines()
{
    int visibleLines = mEdit->viewport()->height() / mEdit->fontMetrics().height();
    mMapper->setVisibleLineCount(visibleLines);
    updateVScrollZone();
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

void TextView::scrollToEnd()
{
    if (mMapper->lineCount() > 0) {
        mMapper->setVisibleTopLine(mMapper->lineCount() - mMapper->visibleLineCount() - 1);
    } else
        mMapper->setVisibleTopLine(1.0);
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
    case Qt::Key_Up:
        mMapper->moveVisibleTopLine(-1);
        break;
    case Qt::Key_Down:
        mMapper->moveVisibleTopLine(1);
        break;
    case Qt::Key_PageUp:
        mMapper->moveVisibleTopLine(-mMapper->visibleLineCount()+1);
        break;
    case Qt::Key_PageDown:
        mMapper->moveVisibleTopLine(mMapper->visibleLineCount()-1);
        break;
    case Qt::Key_Home:
        mMapper->setVisibleTopLine(0);
        break;
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
    mInit = false;
    recalcVisibleLines();
}

void TextView::updateVScrollZone()
{
    int count = mMapper->lineCount();
    verticalScrollBar()->setPageStep(mMapper->visibleLineCount());
    if (count < 0) { // estimated lines count
        verticalScrollBar()->setMinimum(qMin(count+mMapper->visibleLineCount()-1, 0));
        verticalScrollBar()->setMaximum(0);
    } else { // known lines count
        verticalScrollBar()->setMinimum(0);
        verticalScrollBar()->setMaximum(qMax(count-mMapper->visibleLineCount(), 0));
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
        ChangeKeeper x(mDocChanging);
        mEdit->protectWordUnderCursor(true);
        mEdit->setTextCursor(QTextCursor(mEdit->document()));
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
        mEdit->blockSignals(true);
        mEdit->verticalScrollBar()->setSliderPosition(mMapper->visibleOffset());
        mEdit->verticalScrollBar()->setValue(mEdit->verticalScrollBar()->sliderPosition());
        mEdit->blockSignals(false);
        updateVScrollZone();
        mEdit->updateExtraSelections();
        mEdit->protectWordUnderCursor(false);
        mEdit->horizontalScrollBar()->setSliderPosition(mHScrollValue);
        mEdit->horizontalScrollBar()->setValue(mEdit->horizontalScrollBar()->sliderPosition());
    }
}

void TextView::appendedLines(const QStringList &lines, bool append, bool overwriteLast, const QVector<LineFormat> &formats)
{
    mLinesAddedCount += lines.length();
    if (append || overwriteLast) --mLinesAddedCount;
    mStayAtTail = (mEdit->verticalScrollBar()->sliderPosition() >= mEdit->verticalScrollBar()->maximum()-3);
    if (!mStayAtTail) return;

    int remainLineSpace = qMax(0, mMapper->visibleLineCount() - mEdit->blockCount() - mMapper->visibleOffset());

    ChangeKeeper x(mDocChanging);
    QTextCursor cur(mEdit->document());
    cur.movePosition(QTextCursor::End);
    if (append || overwriteLast) {
        cur.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        if (overwriteLast)
            cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        cur.removeSelectedText();
        ++remainLineSpace;
    }
    int firstLine = cur.blockNumber();
    cur.insertText(lines.join("\n")+"\n");

    for (int row = 0; row < formats.size(); ++row) {
        const LineFormat &fmt = formats.at(row);
        if (fmt.start < 0) continue;
        QTextBlock block = mEdit->document()->findBlockByNumber(firstLine+row);
        if (fmt.extraLstFormat) {
            cur.setPosition(block.position());
            cur.setPosition(block.position()+3, QTextCursor::KeepAnchor);
            cur.setCharFormat(*fmt.extraLstFormat);
        }
        cur.setPosition(block.position() + fmt.start);
        cur.setPosition(block.position() + fmt.end, QTextCursor::KeepAnchor);
        cur.setCharFormat(fmt.format);
        cur.setPosition(block.position() + fmt.end);
        cur.setCharFormat(QTextCharFormat());
    }

    int topLinesToRemove = mEdit->blockCount() - mMapper->bufferedLines();
    if (topLinesToRemove > 0) {
        cur.setPosition(0);
        cur.setPosition(mEdit->document()->findBlockByNumber(topLinesToRemove).position()
                        , QTextCursor::KeepAnchor);
        cur.removeSelectedText();
    }
    mMapper->setVisibleTopLine(mMapper->lineCount() - mMapper->visibleLineCount());
    updatePosAndAnchor();
    mEdit->blockSignals(true);
    mEdit->verticalScrollBar()->setSliderPosition(mMapper->visibleOffset());
    mEdit->verticalScrollBar()->setValue(mEdit->verticalScrollBar()->sliderPosition());
    mEdit->blockSignals(false);
    updateVScrollZone();
    mEdit->updateExtraSelections();
    mEdit->protectWordUnderCursor(false);
    mEdit->horizontalScrollBar()->setSliderPosition(mHScrollValue);
    mEdit->horizontalScrollBar()->setValue(mEdit->horizontalScrollBar()->sliderPosition());
    mEdit->repaint();
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
        topLineMoved();
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

    int scrollPos = mEdit->verticalScrollBar()->sliderPosition();
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
    mEdit->verticalScrollBar()->setSliderPosition(scrollPos);
    mEdit->verticalScrollBar()->setValue(mEdit->verticalScrollBar()->sliderPosition());
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

void TextView::contentChanged()
{
    int offset = 0;
    if (mLinesAddedCount) {
        // TODO(JM) implement contentChanged()
        if (mStayAtTail)
            mMapper->moveVisibleTopLine(mLinesAddedCount);
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
