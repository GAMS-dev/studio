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
#include "keys.h"

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
    mEdit->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *lay = new QVBoxLayout(this);
    setLayout(lay);
    lay->addWidget(mEdit);

    connect(verticalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::outerScrollAction);
    connect(mEdit, &TextViewEdit::keyPressed, this, &TextView::editKeyPressEvent);
    connect(mEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::editScrollChanged);
    connect(mEdit, &TextViewEdit::selectionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::cursorPositionChanged, this, &TextView::handleSelectionChange);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
//    connect(mEdit, &AbstractEdit::toggleBookmark, this, &TextView::toggleBookmark);
//    connect(mEdit, &AbstractEdit::jumpToNextBookmark, this, &TextView::jumpToNextBookmark);
    connect(mEdit, &TextViewEdit::searchFindNextPressed, this, &TextView::searchFindNextPressed);
    connect(mEdit, &TextViewEdit::searchFindPrevPressed, this, &TextView::searchFindPrevPressed);

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

void TextView::loadFile(const QString &fileName, int codecMib)
{
    if (codecMib == -1) codecMib = QTextCodec::codecForLocale()->mibEnum();
    mMapper.setCodec(codecMib == -1 ? QTextCodec::codecForMib(codecMib) : QTextCodec::codecForLocale());
    mMapper.openFile(fileName);
    updateVScrollZone();
    int count = (mMapper.lineCount() < 0) ? mTopBufferLines*3 : mMapper.lineCount();
    mMapper.setMappingSizes(count);
    ChangeKeeper x(mDocChanging);
    mEdit->setPlainText(mMapper.lines(0, count));
    mEdit->setTextCursor(QTextCursor(mEdit->document()));
    mPeekTimer.start(100);
}

void TextView::closeFile()
{
    mMapper.closeAndReset();
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

bool TextView::jumpTo(int lineNr, int charNr)
{
    if (lineNr > mMapper.knownLineNrs()) return false;
    int vTop = mMapper.absTopLine()+mMapper.visibleOffset();
    int vAll = mMapper.visibleLineCount();
    if (lineNr < vTop+(vAll/3) || lineNr > vTop+(vAll*2/3)) {
        mMapper.setVisibleTopLine(qMax(0, lineNr-(vAll/3)));
        topLineMoved();
        vTop = mMapper.absTopLine()+mMapper.visibleOffset();
    }
    mMapper.setPosRelative(lineNr - mMapper.absTopLine(), charNr);
    updatePosAndAnchor();
    setFocus();
    return true;
}

QPoint TextView::position() const
{
    return mMapper.position();
}

QPoint TextView::anchor() const
{
    return mMapper.anchor();
}

bool TextView::hasSelection() const
{
    return mMapper.hasSelection();
}

int TextView::knownLines() const
{
    return mMapper.knownLineNrs();
}

void TextView::copySelection()
{
    mEdit->copySelection();
}

QString TextView::selectedText() const
{
    return mMapper.selectedText();
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

bool TextView::findText(QRegularExpression seachRegex, QTextDocument::FindFlags flags)
{
    bool found = false;
    int lineCount = 3 * mTopBufferLines;
    QPoint pos = mMapper.position();
    if (flags.testFlag(QTextDocument::FindBackward))  pos = mMapper.anchor();
    if (pos.y() < 0 || pos.x() < 0) pos = QPoint();

    int relStart = pos.y() - mMapper.absTopLine();

    if (flags.testFlag(QTextDocument::FindBackward)) {
        // search backwards
        int charInLine = mMapper.line(relStart).length() - qMax(0, pos.x());
        relStart -= lineCount-1;
        if (mMapper.absTopLine() + relStart < 0) {
            lineCount += mMapper.absTopLine() + relStart;
            relStart = -mMapper.absTopLine();
        }
        while (!found) {
            QString textBlock = mMapper.lines(relStart, lineCount);
            if (charInLine) textBlock = textBlock.left(textBlock.length() - charInLine);
            QRegularExpressionMatch match;
            textBlock.lastIndexOf(seachRegex, textBlock.length(), &match);
            if (match.hasMatch() || match.hasPartialMatch()) {
                QStringRef ref = textBlock.leftRef(match.capturedStart());
                int line = ref.count("\n");
                int charNr = line ? match.capturedStart() - ref.lastIndexOf("\n") - 1
                                  : match.capturedStart();
                mMapper.setPosRelative(line+relStart, charNr);
                mMapper.setPosRelative(line+relStart, charNr + match.capturedLength(), QTextCursor::KeepAnchor);
                found = true;
            }
            charInLine = 0;
            if (mMapper.absTopLine()+relStart <= 0) break;
        }
    } else {
        int charInLine = pos.x();
        // search forwards
        while (!found) {
            QString textBlock = mMapper.lines(relStart, lineCount);
            if (textBlock.isEmpty()) break;
            QRegularExpressionMatch match;
            textBlock.indexOf(seachRegex, charInLine, &match);
            if (match.hasMatch()) {
                QStringRef ref = textBlock.midRef(charInLine, match.capturedStart()-charInLine);
                int line = ref.count("\n");
                int charNr = line ? match.capturedStart() - charInLine - ref.lastIndexOf("\n") - 1
                                  : match.capturedStart();
                mMapper.setPosRelative(line+relStart, charNr);
                mMapper.setPosRelative(line+relStart, charNr + match.capturedLength(), QTextCursor::KeepAnchor);
                found = true;
            }
            charInLine = 0;
            relStart += lineCount;
        }
    }
    if (found) {
        mMapper.setVisibleTopLine(qMax(0, mMapper.position().y() - mVisibleLines/3));
        topLineMoved();
        updatePosAndAnchor();
    }
    return found;
}

void TextView::peekMoreLines()
{
    // peek and keep timer alive if not done
    if (mMapper.peekChunksForLineNrs(4)) mPeekTimer.start(50);
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
        mEdit->verticalScrollBar()->setValue(mMapper.visibleOffset()); // workaround: isn't set correctly on the first time
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
    if (pos.y() < 0) return;

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
    disconnect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    mEdit->setTextCursor(cursor);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    mEdit->verticalScrollBar()->setValue(scrollPos);
}

void TextView::updateExtraSelections()
{
    mEdit->updateExtraSelections();
}

void TextView::marksChanged()
{
    mEdit->marksChanged();
}


} // namespace studio
} // namespace gams
