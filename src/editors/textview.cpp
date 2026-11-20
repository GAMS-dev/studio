/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "fastfilemapper.h"
#include "memorymapper.h"
#include "logger.h"
#include "textviewedit.h"
#include "keys.h"
#include "settings.h"
#include "editorhelper.h"
#include "editors/navigationhistorylocator.h"
#include "editors/navigationhistory.h"
#include "search/searchfile.h"
#include "search/searchworker.h"
#include "encoding.h"

#include <QScrollBar>
#include <QTextBlock>
#include <QPlainTextDocumentLayout>
#include <QBoxLayout>
#include <QtMath>

namespace gams {
namespace studio {

using namespace search;

TextView::TextView(TextKind kind, QWidget *parent) : QAbstractScrollArea(parent), mTextKind(kind)
{
    setViewportMargins(0,0,0,0);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    if (kind == FileText) {
        mMapper = new FastFileMapper();
    }

    if (kind == MemoryText) {
        MemoryMapper* mm = new MemoryMapper();
        connect(this, &TextView::addProcessLog, mm, &MemoryMapper::addProcessLog);
        connect(mm, &MemoryMapper::createMarks, this, &TextView::createMarks);
        connect(mm, &MemoryMapper::appendLines, this, &TextView::appendLines);
        connect(mm, &MemoryMapper::switchLst, this, &TextView::switchLst);
        connect(mm, &MemoryMapper::registerGeneratedFile, this, &TextView::registerGeneratedFile);

//        connect(mm, &MemoryMapper::appendDisplayLines, this, &TextView::appendedLines);
        connect(mm, &MemoryMapper::updateView, this, &TextView::updateView);
        mMapper = mm;
    }
    mMapper->setEncoding(Encoding::defaultEncoding(Settings::settings()->toInt(skDefaultCodecMib)));

    mEdit = new TextViewEdit(*mMapper, this);
    mEdit->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *lay = new QVBoxLayout(this);
    setLayout(lay);
    lay->addWidget(mEdit);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    horizontalScrollBar()->setRange(0, 0);
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &TextView::horizontalScrollChanged);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](){
        if (int dy = verticalScrollBar()->value() - mScrollPos.y()) {
            mScrollPos.setY(verticalScrollBar()->value());
            emit scrolled(this, 0, dy);
        }
    });

    if (kind == MemoryText)
        connect(mEdit, &TextViewEdit::findClosestLstRef, this, &TextView::findClosestLstRef);
    connect(verticalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::outerScrollAction);
    connect(mEdit, &TextViewEdit::keyPressed, this, &TextView::editKeyPressEvent);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    connect(mEdit, &TextViewEdit::scrollColumns, this, [this](int columns) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + columns);
    });
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::selectionChanged);
    connect(mEdit, &TextViewEdit::positionedByMouse, this, [this]() {
        // mouse released on a line that ends left of the view places the cursor at the line end
        if (mDocChanging) return;
        QPoint pos = mMapper->position(true);
        if (pos.y() < 0 || pos.x() >= mColumnOffset) return;
        horizontalScrollBar()->setValue(qMax(0, pos.x() - mEdit->columnCount() * 3 / 4));
    });
    connect(mEdit, &TextViewEdit::continueFindPressed, this, &TextView::continueFindPressed);
    connect(mEdit, &TextViewEdit::continueSearchPressed, this, &TextView::continueSearchPressed);
    connect(mEdit, &TextViewEdit::hasHRef, this, &TextView::hasHRef);
    connect(mEdit, &TextViewEdit::jumpToHRef, this, &TextView::jumpToHRef);
    connect(mEdit, &TextViewEdit::topLineMoved, this, &TextView::updateView);
    connect(mEdit, &TextViewEdit::selectWord, this, &TextView::selectWord);
    connect(mEdit->horizontalScrollBar(), &QScrollBar::rangeChanged, this, &TextView::applyInnerHScroll);
    connect(mEdit->horizontalScrollBar(), &QScrollBar::valueChanged, this, &TextView::applyInnerHScroll);

    connect(mMapper, &AbstractTextMapper::loadAmountChanged, this, &TextView::loadAmountChanged);
    connect(mMapper, &AbstractTextMapper::blockCountChanged, this, &TextView::blockCountChanged);
    connect(mMapper, &AbstractTextMapper::blockCountChanged, this, &TextView::updateVScrollZone);
    connect(mMapper, &AbstractTextMapper::selectionChanged, this, &TextView::selectionChanged);
    mEdit->verticalScrollBar()->installEventFilter(this);
    mEdit->installEventFilter(this);
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

bool TextView::loadFile(const QString &fileName, const QString &encoding, bool initAnchor)
{
    if (mTextKind != FileText) return false;
    mMapper->setEncoding(encoding);

    if (!static_cast<FastFileMapper*>(mMapper)->openFile(fileName, initAnchor)) return false;
    recalcVisibleLines();
    if (initAnchor)
        mMapper->setVisibleTopLine(0);
    updateView();
    return true;
}

void TextView::print(QPagedPaintDevice *printer)
{
    QString fileName = static_cast<FastFileMapper*>(mMapper)->fileName();
    if(fileName.isEmpty()) return;
    QString text;
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)){
        text = file.readAll();
        file.close();
    }
    QTextDocument document(text);
    document.print(printer);
}

void TextView::scrollSynchronize(int dx, int dy)
{
    if (dx) scrollHPixels(dx);
    mMapper->setVisibleTopLine(mMapper->visibleTopLine() + dy);
    updateView();
}

TextView::TextKind TextView::kind() const
{
    if (mMapper->kind() == AbstractTextMapper::memoryMapper)
        return MemoryText;
    return FileText;
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
    if (mStayAtTail) {
        jumpToEnd(); // in case this has been left out (on a fast run)
        delete mStayAtTail;
        mStayAtTail = nullptr;
    }
    updateView();
    topLineMoved(); // force repaint directly (missed at updateView if slider hasn't changed)
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

bool TextView::jumpTo(int lineNr, int charNr, int length, bool focus)
{
    if (lineNr > mMapper->knownLineNrs())
        lineNr = mMapper->knownLineNrs();

    int vTop = mMapper->visibleTopLine();
    int vAll = mMapper->visibleLineCount();
    if (lineNr < vTop+(vAll/6) || lineNr > vTop+(vAll*5/6)) {
        mMapper->setVisibleTopLine(qMax(0, lineNr-(vAll/2)));
        updateView();
    }

    int relLine = lineNr - mMapper->visibleTopLine();
    if (length > 0) mMapper->setPosRelative(relLine, charNr, QTextCursor::MoveAnchor);
    mMapper->setPosRelative(relLine, charNr + length, (length > 0 ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor));
    updatePosAndAnchor();
    ensureCursorColumnVisible();
    emit selectionChanged();
    if (focus) mEdit->setFocus();
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
    mMapper->copyToClipboard();
}

QString TextView::selectedText() const
{
    return mMapper->selectedText();
}

QString TextView::wordUnderCursor() const
{
    return mCurrentWord;
}

QString TextView::currentFindSelection(bool keep)
{
    if (mMapper->position().x() != mMapper->anchor().x() &&
        mMapper->position().y() == mMapper->anchor().y())
        return selectedText();
    if (keep)
        return wordUnderCursor();

    mEdit->recalcWordUnderCursor(true);
    QPoint pos = mMapper->position(true);
    mMapper->setPosRelative(pos.y(), mDocColumn + mEdit->textCursor().positionInBlock());
    updatePosAndAnchor();
    return mEdit->wordUnderCursor();
}

void TextView::selectAllText()
{
    mEdit->selectAllText();
}

void TextView::clearSelection()
{
    mEdit->clearSelection(); // this also updates the mapper
    emit selectionChanged();
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

void TextView::setFindTerm(const QRegularExpression &rex, QTextDocument::FindFlags options)
{
    mEdit->setFindTerm(rex, options);
}

bool TextView::findText(const QRegularExpression &rex, QTextDocument::FindFlags options, bool next)
{
    if (rex.pattern().isEmpty()) {
        mEdit->setFindTerm(rex, options);
        return false;
    }
    if (hasSelectedFind()) {
        // recent find is selected -> this is a find next/previous
        mMapper->setSelectionDirection(options.testFlag(QTextDocument::FindBackward) ? Qt::RightToLeft
                                                                                     : Qt::LeftToRight);
    }
    if (next)
        mMapper->clearSelection();

    mEdit->setFindTerm(rex, options);
    bool found = false;
    bool goOn = false;
    while (!found) {
        found = mMapper->searchText(rex, options, goOn);
        if (found) {
            // TODO(JM) find a way to skip hidden matches
            mMapper->scrollToPosition();
            updateView();
            updatePosAndAnchor();
            emit selectionChanged();
            if (!mEdit->textCursor().hasSelection())
                found = false;
        } else if (!goOn) {
            break;
        }
    }

    if (found) {
        mFindSpan = QPair<QPoint, int>(mMapper->position(), mMapper->anchor().x());
        updateView();
        updatePosAndAnchor();
        emit selectionChanged();
        ensureCursorColumnVisible();
    }
    return found;
}

bool TextView::searchText(const QRegularExpression &searchRegex, QTextDocument::FindFlags flags, bool &continueFind)
{
    bool found = mMapper->searchText(searchRegex, flags, continueFind);
    if (found) {
        updateView();
        updatePosAndAnchor();
        emit selectionChanged();
        ensureCursorColumnVisible();
    }
    return found;
}

bool TextView::hasSelectedFind()
{
    return (mFindSpan.second != mFindSpan.first.x()
            && mFindSpan.second == mMapper->anchor().x()
            && mFindSpan.first == mMapper->position());
}

void TextView::clearSelectedFind()
{
    bool doClear = hasSelectedFind();
    mFindSpan = {{0,0},0};
    if (doClear) {
        clearSelection();
    }
}

void TextView::searchInSelection(const QRegularExpression &searchRegex, FileMeta* file,
                                 QList<search::Result> *results, QHash<FileId, SortedIntMap> *lines, bool showResults)
{
    if (!mMapper->hasSearchSelection()) {
        mEdit->updateSearchSelection();
        mMapper->updateSearchSelection();
    }
    if (!mMapper->hasSearchSelection()) return;
    SearchWorker sw(SearchFile(file), searchRegex, mMapper->searchSelectionStart(), mMapper->searchSelectionEnd(),
                    results, lines, showResults);
    sw.searchInFiles();
}

void TextView::clearSearchSelection()
{
    mEdit->clearSearchSelection();
    mMapper->clearSearchSelection();
}

void TextView::setSearchSelectionActive(bool active)
{
    mEdit->setSearchSelectionActive(active);
    mMapper->setSearchSelectionActive(active);
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
        int lineNr = verticalScrollBar()->sliderPosition();
        if (mMapper->knownLineNrs() >= lineNr)
            mMapper->setVisibleTopLine(lineNr);
         else
            mMapper->setVisibleTopLine(double(lineNr) / verticalScrollBar()->maximum());
    }
        break;
    default: break;
    }
    topLineMoved();
}

void TextView::horizontalScrollChanged(int value)
{
    // only the wheel scrolls by pixels, any other change aligns the view to the column
    if (!mKeepSubPixel) mSubPixel = 0.0;
    if (value == mColumnOffset) {
        applyInnerHScroll();
        return;
    }
    mColumnOffset = value;
    topLineMoved();
}

void TextView::scrollHPixels(qreal dx)
{
    qreal charWidth = mEdit->charWidth();
    qreal maxPixels = horizontalScrollBar()->maximum() * charWidth;
    qreal pixels = qBound(0.0, mColumnOffset * charWidth + mSubPixel + dx, maxPixels);
    int column = qFloor(pixels / charWidth);
    mSubPixel = qMax(0.0, pixels - column * charWidth);
    if (column == mColumnOffset) {
        applyInnerHScroll();
    } else {
        mKeepSubPixel = true;
        horizontalScrollBar()->setValue(column); // triggers horizontalScrollChanged -> topLineMoved
        mKeepSubPixel = false;
    }
}

void TextView::applyInnerHScroll()
{
    if (mApplyingInnerHScroll) return;
    mApplyingInnerHScroll = true;
    QScrollBar *bar = mEdit->horizontalScrollBar();
    int pixels = qMax(0, qRound((mColumnOffset - mDocColumn) * mEdit->charWidth() + mSubPixel));
    if (bar->maximum() < pixels) bar->setMaximum(pixels);
    if (bar->value() != pixels) bar->setValue(pixels);
    mApplyingInnerHScroll = false;
}

void TextView::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    if (!mInit) updateMargins();
    QTimer::singleShot(0, this, &TextView::recalcVisibleLines);
    QTimer::singleShot(0, this, &TextView::topLineMoved);
}

void TextView::recalcVisibleLines()
{
    int visibleLines = mEdit->lineCount();
    if (visibleLines > 0) {
        mMapper->setVisibleLineCount(visibleLines);
        updateVScrollZone();
        updateView();
    }
}

void TextView::showEvent(QShowEvent *event)
{
    QAbstractScrollArea::showEvent(event);
    if (mInit) init();
}

void TextView::focusInEvent(QFocusEvent *event)
{
    QAbstractScrollArea::focusInEvent(event);
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

bool TextView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == verticalScrollBar() && mStayAtTail) {
        if (event->type() == QEvent::MouseButtonPress) {
            mSliderMouseStart = static_cast<QMouseEvent*>(event)->position().y();
            mSliderStartedAtTail = *mStayAtTail;
            if (mSliderStartedAtTail) *mStayAtTail = false;
        }
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->buttons().testFlag(Qt::LeftButton) && me->position().y() > verticalScrollBar()->height() - verticalScrollBar()->width())
                mSliderStartedAtTail = true;
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (mSliderStartedAtTail && (mSliderMouseStart-3 < me->position().y())) {
                *mStayAtTail = true;
            } else if (mSliderMouseStart-3 > me->position().y()) {
                *mStayAtTail = false;
            }
        }
    }
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        QPoint pixels = wheelEvent->hasPixelDelta() ? wheelEvent->pixelDelta() : wheelEvent->angleDelta() / 4;
        // Keeps the start axis for touchpad to avoid the view jumping back and forth.
        Qt::ScrollPhase phase = wheelEvent->phase();
        if (phase == Qt::ScrollBegin)
            mPartialScroll = 0.0;
        if (phase == Qt::ScrollBegin || phase == Qt::NoScrollPhase)
            mWheelAxis = 0;
        if (!mWheelAxis && !pixels.isNull())
            mWheelAxis = qAbs(pixels.x()) > qAbs(pixels.y()) ? Qt::Horizontal : Qt::Vertical;
        int axis = mWheelAxis;
        if (phase == Qt::ScrollEnd)
            mWheelAxis = 0;
        if (axis == Qt::Horizontal) {
            if (pixels.x()) scrollHPixels(-pixels.x());
            return true;
        }
        if (axis != Qt::Vertical) return true;
        mPartialScroll -= pixels.y() * .1;
        int delta = int(mPartialScroll);
        mPartialScroll -= delta;
        if (!delta) return true;
        mMapper->moveVisibleTopLine(delta);
        if (verticalScrollBar()->maximum()) {
            verticalScrollBar()->setSliderPosition(qAbs(mMapper->visibleTopLine()));
            verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition());
        } else topLineMoved();
        if (mStayAtTail) {
            if (delta > 0) *mStayAtTail = false;
            else {
                if (!*mStayAtTail) *mStayAtTail = mMapper->atTail();
            }
        }
        return true;
    }
    if (event->type() == QEvent::FontChange) {
        QTimer::singleShot(0, this, &TextView::recalcVisibleLines);
        QTimer::singleShot(0, this, &TextView::topLineMoved);
    }
    if (event->type() == QEvent::FocusOut) {
        mEdit->setCursorWidth(0);
    }
    if (event->type() == QEvent::FocusIn) {
        updatePosAndAnchor();
    }
    return false;
}

void TextView::jumpToEnd()
{
    if (mMapper->lineCount() > 0) {
        mMapper->setVisibleTopLine(mMapper->lineCount() - mMapper->reducedVisibleLineCount());
        mMapper->setPosToAbsEnd();
        if (mStayAtTail) *mStayAtTail = mMapper->atTail();
    } else {
        mMapper->setVisibleTopLine(1.0);
    }
    if (verticalScrollBar()->maximum() && verticalScrollBar()->sliderPosition() != qAbs(mMapper->visibleTopLine())) {
        verticalScrollBar()->setSliderPosition(qAbs(mMapper->visibleTopLine()));
        verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition());
    } else {
        topLineMoved();
    }
}

void TextView::setLineMarker(int line)
{
    const QList<int> markers = mMapper->lineMarkers();
    if (markers.size() != 1 || markers.first() != line) {
        mMapper->setLineMarkers(QList<int>() << line);
        topLineMoved();
    }
}

void TextView::updateView()
{
    if (mStayAtTail && *mStayAtTail) {
        jumpToEnd();
    } else {
        if (verticalScrollBar()->maximum()) {
            verticalScrollBar()->blockSignals(true);
            verticalScrollBar()->setSliderPosition(qAbs(mMapper->visibleTopLine()));
            verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition());
            verticalScrollBar()->blockSignals(false);
        }
        topLineMoved();
    }
}

void TextView::selectWord(int localLine, int charFrom, int charTo)
{
    mMapper->setPosRelative(localLine, charFrom);
    mMapper->setPosRelative(localLine, charTo, QTextCursor::KeepAnchor);
    updatePosAndAnchor();
}

int TextView::firstErrorLine()
{
    MemoryMapper *memMapper = qobject_cast<MemoryMapper*>(mMapper);
    if (!memMapper) return -1;
    return memMapper->firstErrorLine();
}

void TextView::editKeyPressEvent(QKeyEvent *event)
{
    const QPoint absPosBefore = mMapper->position();
    QPoint pos = mMapper->position(true);
    bool cursorIsValid = pos.y() > AbstractTextMapper::cursorInvalid;
    QTextCursor::MoveMode mode = event->modifiers().testFlag(Qt::ShiftModifier) ? QTextCursor::KeepAnchor
                                                                                : QTextCursor::MoveAnchor;
    if (event == Hotkey::MoveToStartOfDoc) {
        mMapper->jumpToPrevParagraph(mode);
        updatePosAndAnchor();
    } else if (event == Hotkey::MoveToStartOfBlock) {
            mMapper->setVisibleTopLine(0);
            mMapper->setPosToAbsStart(mode);
            updatePosAndAnchor();
    } else if (event == Hotkey::MoveToStartOfLine && cursorIsValid) {
        mMapper->setPosRelative(pos.y(), 0, mode);
        updatePosAndAnchor();
    } else if (event == Hotkey::MoveToEndOfDoc) {
        mMapper->jumpToNextParagraph(mode);
        updatePosAndAnchor();
    } else if (event == Hotkey::MoveToEndOfBlock) {
        jumpToEnd();
        mMapper->setPosToAbsEnd(mode);
        updatePosAndAnchor();
    } else if (event == Hotkey::MoveToEndOfLine && cursorIsValid) {
        mMapper->setPosRelative(pos.y()+1, -1, mode);
        updatePosAndAnchor();
    } else if (event == Hotkey::MoveViewLineUp) {
        mMapper->moveVisibleTopLine(-1);
    } else if (event == Hotkey::MoveViewLineDown) {
        mMapper->moveVisibleTopLine(1);
    } else if (event == Hotkey::MoveCharGroupRight) {
        int p = pos.x();
        QString line = mMapper->positionLine();
        EditorHelper::nextWord(0, p, line);
        if (p > line.length()) {
            if (pos.y() > mMapper->visibleLineCount()-2) {
                mMapper->moveVisibleTopLine(1);
                mMapper->setPosRelative(pos.y(), 0, mode);
            } else
                mMapper->setPosRelative(pos.y()+1, 0, mode);
        } else {
            mMapper->setPosRelative(pos.y(), p, mode);
        }
    } else if (event == Hotkey::MoveCharGroupLeft) {
        int p = pos.x();
        if (p == 0) {
            if (pos.y() == 0) {
                if (mMapper->moveVisibleTopLine(-1) >= 0)
                    mMapper->setPosRelative(1, -1, mode);
            } else {
                mMapper->setPosRelative(pos.y(), -1, mode);
            }
        } else {
            QString line = mMapper->positionLine();
            EditorHelper::prevWord(0, p, line);
            mMapper->setPosRelative(pos.y(), p, mode);
        }
    } else if (!event->modifiers().testFlag(Qt::AltModifier)) {
        switch (event->key()) {
        case Qt::Key_Up:
            if (!cursorIsValid)
                mMapper->moveVisibleTopLine(-1);
            else {
                QPoint p = mMapper->position(true);
                mMapper->setPosRelative(p.y()-1, -2, mode);
                if (mMapper->visibleTopLine() > 0 && p.y() < 2)
                    mMapper->moveVisibleTopLine(-1);
                else updatePosAndAnchor();
            }
            break;
        case Qt::Key_Down:
            if (!cursorIsValid)
                mMapper->moveVisibleTopLine(1);
            else {
                mMapper->setPosRelative(pos.y()+1, -2, mode);
                if (pos.y() > mMapper->visibleLineCount()-2)
                    mMapper->moveVisibleTopLine(1);
                else updatePosAndAnchor();
            }
            break;
        case Qt::Key_Left:
            if (pos.y() >= 0) {
                mMapper->setPosRelative(pos.y(), pos.x()-1, mode);
                if (mMapper->visibleTopLine() > 0 && pos.y() < 2 && pos.y() > mMapper->position(true).y())
                    mMapper->moveVisibleTopLine(-1);
                else updatePosAndAnchor();
            }
            break;
        case Qt::Key_Right:
            if (pos.y() >= 0) {
                QTextCursor cur = mEdit->textCursor();
                int lineLength = mLineLengths.value(cur.blockNumber(), cur.block().length()-1);
                if (mDocColumn + cur.positionInBlock() < lineLength) {
                    mMapper->setPosRelative(pos.y(), pos.x()+1, mode);
                    if (pos.y() > mMapper->visibleLineCount()-2 && pos.y() < mMapper->position(true).y())
                        mMapper->moveVisibleTopLine(1);
                    else updatePosAndAnchor();
                } else {
                    mMapper->setPosRelative(pos.y()+1, 0, mode);
                    if (pos.y() > mMapper->visibleLineCount()-2)
                        mMapper->moveVisibleTopLine(1);
                    else updatePosAndAnchor();
                }
            }
            break;
        case Qt::Key_PageUp: mMapper->moveVisibleTopLine(-mMapper->visibleLineCount()+1); break;
        case Qt::Key_PageDown: mMapper->moveVisibleTopLine(mMapper->visibleLineCount()-1); break;
        default:
            event->ignore();
            break;
        }
    }
    if (mStayAtTail) *mStayAtTail = mMapper->atTail();
    emit selectionChanged();
    updateView();
    if (mMapper->position() != absPosBefore)
        ensureCursorColumnVisible();
}

void TextView::init()
{
    updateMargins();
    mInit = false;
    recalcVisibleLines();
}

void TextView::updateMargins()
{
    int right = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
    int bottom = horizontalScrollBar()->isVisible() ? horizontalScrollBar()->height() : 0;
    QMargins margins(0, 0, right, bottom);
    if (layout()->contentsMargins() != margins)
        layout()->setContentsMargins(margins);
}

int TextView::toDocColumn(int column, const QTextBlock &block) const
{
    return qBound(0, column - mDocColumn, qMax(0, block.length()-1));
}

void TextView::adjustSearchSelection()
{
    if (!mMapper->hasSearchSelection()) return;

    QPoint from = QPoint(mMapper->searchSelectionStart());
    QPoint to = QPoint(mMapper->searchSelectionEnd());

    from.ry() -= mMapper->visibleTopLine();
    if (from.y() < 0)
        from = QPoint(0,0);
    else if (from.y() > mMapper->visibleLineCount())
        return;

    to.ry() -= mMapper->visibleTopLine();
    bool toDocEnd = false;
    if (to.y() < 0)
        return;
    else if (to.y() > mMapper->visibleLineCount()) {
        to.setY(mMapper->visibleLineCount());
        toDocEnd = true;
    }

    QTextBlock fromBlock = mEdit->document()->findBlockByLineNumber(from.y());
    QTextBlock toBlock = mEdit->document()->findBlockByLineNumber(to.y());
    QTextCursor cur(mEdit->document());
    cur.setPosition(fromBlock.position() + toDocColumn(from.x(), fromBlock));
    cur.setPosition(toBlock.position() + (toDocEnd ? qMax(0, toBlock.length()-1) : toDocColumn(to.x(), toBlock)),
                    QTextCursor::KeepAnchor);
    mEdit->adjustSearchSelection(cur);
}

void TextView::updateHScrollZone(int maxLineLength)
{
    int columns = mEdit->columnCount();
    QScrollBar *bar = horizontalScrollBar();
    bar->blockSignals(true);
    bar->setPageStep(columns);
    bar->setSingleStep(1);
    // +1 to have room for the cursor behind the last character
    bar->setMaximum(qMax(0, maxLineLength + 1 - columns));
    bar->blockSignals(false);
    mColumnOffset = bar->value();
}

bool TextView::ensureCursorColumnVisible()
{
    if (mDocChanging) return false;
    QPoint pos = mMapper->position(true);
    if (pos.y() < 0) return false;
    int columns = mEdit->columnCount();
    int col = pos.x();
    int offset = mColumnOffset;
    // scroll by a quarter of the width beyond the cursor
    if (col < offset)
        offset = qMax(0, col - columns / 4);
    else if (col > offset + columns - 2)
        offset = qMax(0, col - columns * 3 / 4);
    if (offset == mColumnOffset) return false;
    if (offset > horizontalScrollBar()->maximum())
        horizontalScrollBar()->setMaximum(offset);
    horizontalScrollBar()->setValue(offset); // triggers horizontalScrollChanged -> topLineMoved
    return true;
}

void TextView::updateVScrollZone()
{
    verticalScrollBar()->setPageStep(mMapper->visibleLineCount());
    verticalScrollBar()->setMaximum(qMax(0, qAbs(mMapper->lineCount()) - mMapper->reducedVisibleLineCount()));
    if (mMapper->kind() == AbstractTextMapper::fileMapper) {
        if (verticalScrollBar()->maximum()) {
            verticalScrollBar()->setSliderPosition(qAbs(mMapper->visibleTopLine()));
            verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition());
        } else topLineMoved();
    }
}

void TextView::topLineMoved()
{
    bool wasRecording = NavigationHistoryLocator::navigationHistory()->isRecording();
    NavigationHistoryLocator::navigationHistory()->stopRecord();

    if (!mDocChanging) {
        ChangeKeeper x(mDocChanging);
        mEdit->protectWordUnderCursor(true);
        disconnect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
        mEdit->setTextCursor(QTextCursor(mEdit->document()));
        connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
        QList<LineFormat> formats;
        QString dat = mMapper->lines(0, mMapper->visibleLineCount()+1, formats);

        // Only visible columns. As the font is monospace, the horizontal scrolling can be handled by cutting the lines.
        QList<QStringView> lines = QStringView(dat).split(u'\n');
        QList<int> lineLengths;
        lineLengths.reserve(lines.size());
        int maxLineLength = 0;
        for (QStringView &line : lines) {
            if (line.endsWith(u'\r')) line.chop(1);
            lineLengths << line.length();
            maxLineLength = qMax(maxLineLength, int(line.length()));
        }
        updateHScrollZone(maxLineLength);
        // The document holds a buffer of columns on both sides of the visible part. Scrolling within the buffer only
        // shifts the inner edit by pixels, which allows smooth scrolling and avoids rebuilding the document.
        const int CColumnBuffer = 20;
        const int visibleColumns = mEdit->columnCount() + 1; // +1 for a partially visible column when scrolled by pixels
        const int columnCount = visibleColumns + 1 + 2 * CColumnBuffer;
        bool inBuffer = mColumnOffset >= mCurrentDocColumn
                        && mColumnOffset + visibleColumns <= mCurrentDocColumn + mCurrentColumnCount;
        bool rebuild = mCurrentVisibleTopLine != mMapper->visibleTopLine() || mCurrentDataLength != dat.size()
                       || !inBuffer || mCurrentColumnCount != columnCount;
        if (rebuild) {
            mDocColumn = qMax(0, mColumnOffset - CColumnBuffer);
            mCurrentVisibleTopLine = mMapper->visibleTopLine();
            mCurrentDataLength = dat.size();
            mCurrentDocColumn = mDocColumn;
            mCurrentColumnCount = columnCount;
            mLineLengths = lineLengths;
            mEdit->setColumnOffset(mDocColumn);
            if (maxLineLength > columnCount || mDocColumn > 0) {
                QStringList visibleParts;
                visibleParts.reserve(lines.size());
                for (const QStringView &line : std::as_const(lines))
                    visibleParts << line.mid(mDocColumn, columnCount).toString();
                mEdit->setPlainText(visibleParts.join('\n'));
            } else {
                mEdit->setPlainText(dat);
            }
            adjustSearchSelection();
        }
        if (rebuild || mCurrentFormats != formats) {
            mCurrentFormats = formats;
            QTextCursor cur(mEdit->document());
            cur.select(QTextCursor::Document);
            cur.setCharFormat(QTextCharFormat());
            QList<bool> lineMarked;
            for (int row = 0; row < mEdit->blockCount() && row < formats.size(); ++row) {
                lineMarked << formats.at(row).lineMarked;
                if (formats.at(row).start < 0) continue;
                const LineFormat &format = formats.at(row);
                QTextBlock block = mEdit->document()->findBlockByNumber(row);
                QTextCursor cursor(block);
                if (format.extraLstFormat && mDocColumn < 3) {
                    cursor.setPosition(block.position() + toDocColumn(3, block), QTextCursor::KeepAnchor);
                    QTextCharFormat extraFormat = *format.extraLstFormat;
                    extraFormat.setAnchor(true);
                    extraFormat.setAnchorHref(format.extraLstHRef);
                    cursor.setCharFormat(extraFormat);
                }
                cursor.setPosition(block.position() + toDocColumn(format.start, block));
                cursor.setPosition(block.position() + toDocColumn(format.end, block), QTextCursor::KeepAnchor);
                if (cursor.hasSelection())
                    cursor.setCharFormat(format.format);
            }
            mEdit->setLineMarked(lineMarked);
        }
        updatePosAndAnchor();
        mEdit->updateExtraSelections();
        mEdit->protectWordUnderCursor(false);
        if (mEdit->verticalScrollBar()->sliderPosition())
            mEdit->verticalScrollBar()->setSliderPosition(0);
        applyInnerHScroll();
        if (wasRecording) NavigationHistoryLocator::navigationHistory()->startRecord();
    }
}

TextView::TextKind TextView::textKind() const
{
    return mTextKind;
}

void TextView::setLogParser(LogParser *logParser)
{
    if (kind() == TextView::MemoryText)
        static_cast<MemoryMapper*>(mMapper)->setLogParser(logParser);
}

LogParser *TextView::logParser() const
{
    if (kind() == TextView::MemoryText)
        return static_cast<MemoryMapper*>(mMapper)->logParser();
    return nullptr;
}

void TextView::setDebugMode(bool debug)
{
    if (mMapper->debugMode() != debug) {
        mMapper->setDebugMode(debug);
        if (mMapper->lineCount() > 0) {
            mCurrentVisibleTopLine = -1;
            updateView();
            topLineMoved();
        }
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

void TextView::updatePosAndAnchor(bool toWordStart)
{
    QPoint pos = mMapper->position(true);
    QPoint anchor = mMapper->anchor(true);
    // if anchor or selection has changed after marking the find -> invalidate the find-span
    if (mFindSpan.first != mMapper->position() ||
        mFindSpan.second != mMapper->anchor().x() ||
        mMapper->position().y() != mMapper->anchor().y() )
        mFindSpan = {{0,0},0};
    if (pos.y() == AbstractTextMapper::cursorInvalid
            || ( pos.y() < AbstractTextMapper::cursorInvalid && (pos.y() == anchor.y() || !mMapper->hasSelection()) )) {
        mEdit->setCursorWidth(0);
        mCurrentWord = "";
        return;
    }
    mEdit->setCursorWidth( (pos.y() > AbstractTextMapper::cursorInvalid) ? 2 : 0 );
    if (mMapper->debugMode()) {
        if (pos.y() > AbstractTextMapper::cursorInvalid)
            pos.setY(qMin(pos.y()*2 + 1, mMapper->visibleLineCount()));
        if (anchor.y() > AbstractTextMapper::cursorInvalid)
            anchor.setY(qMin(anchor.y()*2 + 1, mMapper->visibleLineCount()));
    }

    ChangeKeeper x(mDocChanging);
    QTextCursor cursor = QTextCursor(mEdit->document());
    if (mMapper->hasSelection()) {
        QTextBlock block = anchor.y() == AbstractTextMapper::cursorBeforeStart ?
                    mEdit->document()->firstBlock()
                  : anchor.y() == AbstractTextMapper::cursorBeyondEnd ?
                        mEdit->document()->lastBlock()
                      : mEdit->document()->findBlockByNumber(qMax(0, anchor.y()));
        int p = block.position() + toDocColumn(anchor.x(), block);
        cursor.setPosition(p);
        block = pos.y() == AbstractTextMapper::cursorBeforeStart ?
                    mEdit->document()->firstBlock()
                  : pos.y() == AbstractTextMapper::cursorBeyondEnd ?
                        mEdit->document()->lastBlock()
                      : mEdit->document()->findBlockByNumber(qMax(0, pos.y()));
        p = block.position() + toDocColumn(pos.x(), block);
        cursor.setPosition(p, QTextCursor::KeepAnchor);
    } else {
        QTextBlock block = pos.y() == AbstractTextMapper::cursorBeforeStart ?
                    mEdit->document()->firstBlock()
                  : pos.y() == AbstractTextMapper::cursorBeyondEnd ?
                        mEdit->document()->lastBlock()
                      : mEdit->document()->findBlockByNumber(qMax(0, pos.y()));
        int p = block.position() + toDocColumn(pos.x(), block);
        cursor.setPosition(p);
    }
    disconnect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    mEdit->setTextCursor(cursor);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    // setTextCursor may have scrolled the inner edit to show the cursor
    applyInnerHScroll();
    if (!mMapper->hasSelection()) {
        mCurrentWord = mEdit->wordUnderCursor();
        if (toWordStart) {
            int offset = mEdit->textCursor().positionInBlock();
            mEdit->recalcWordUnderCursor(toWordStart);
            offset -= mEdit->textCursor().positionInBlock();
            QPoint relPos = mMapper->position(true);
            mMapper->setPosRelative(relPos.y(), relPos.x() - offset);
        }
    } else {
        mCurrentWord = "";
    }
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

void TextView::findClosestLstRef(const QTextCursor &cursor)
{
    if (mMapper->kind() != AbstractTextMapper::memoryMapper) return;
    int line = cursor.blockNumber();
    QString href = static_cast<MemoryMapper*>(mMapper)->findClosestLst(line);
    if (!href.isEmpty()) emit jumpToHRef(href);
    else emit jumpToHRef("LST:0");
}

void TextView::updateExtraSelections()
{
    mEdit->updateExtraSelections();
}

void TextView::updateTheme()
{
    if (mMapper->kind() != AbstractTextMapper::memoryMapper) return;
    static_cast<MemoryMapper*>(mMapper)->updateTheme();
    topLineMoved();
}

void TextView::marksChanged(const QSet<int> &dirtyLines)
{
    mEdit->marksChanged(dirtyLines);
}


} // namespace studio
} // namespace gams
