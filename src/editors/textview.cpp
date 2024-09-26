/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
// #include "filemapper.h"
#include "fastfilemapper.h"
#include "memorymapper.h"
#include "logger.h"
#include "textviewedit.h"
#include "keys.h"
#include "settings.h"
#include "editorhelper.h"
#include "editors/navigationhistorylocator.h"
#include "editors/navigationhistory.h"
#include "search/searchhelpers.h"
#include "search/searchworker.h"

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
    int mib = Settings::settings()->toInt(skDefaultCodecMib);
    QTextCodec *codec = QTextCodec::codecForMib(mib);

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
    mMapper->setCodec(codec);

    mEdit = new TextViewEdit(*mMapper, this);
    mEdit->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *lay = new QVBoxLayout(this);
    setLayout(lay);
    lay->addWidget(mEdit);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](){
        if (int dy = verticalScrollBar()->value() - mScrollPos.y()) {
            mScrollPos.setY(verticalScrollBar()->value());
            emit scrolled(this, 0, dy);
        }
    });

    if (kind == MemoryText)
        connect(mEdit, &TextViewEdit::findClosestLstRef, this, &TextView::findClosestLstRef);
    connect(verticalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::outerScrollAction);
    connect(mEdit->horizontalScrollBar(), &QScrollBar::actionTriggered, this, &TextView::horizontalScrollAction);
    connect(mEdit, &TextViewEdit::keyPressed, this, &TextView::editKeyPressEvent);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    connect(mEdit, &TextViewEdit::searchFindNextPressed, this, &TextView::searchFindNextPressed);
    connect(mEdit, &TextViewEdit::searchFindPrevPressed, this, &TextView::searchFindPrevPressed);
    connect(mEdit, &TextViewEdit::hasHRef, this, &TextView::hasHRef);
    connect(mEdit, &TextViewEdit::jumpToHRef, this, &TextView::jumpToHRef);
    connect(mEdit, &TextViewEdit::topLineMoved, this, &TextView::updateView);
    connect(mEdit, &TextViewEdit::selectWord, this, &TextView::selectWord);

    connect(mMapper, &AbstractTextMapper::loadAmountChanged, this, &TextView::loadAmountChanged);
    connect(mMapper, &AbstractTextMapper::blockCountChanged, this, &TextView::blockCountChanged);
    connect(mMapper, &AbstractTextMapper::blockCountChanged, this, &TextView::updateVScrollZone);
    connect(mMapper, &AbstractTextMapper::selectionChanged, this, &TextView::selectionChanged);
    mEdit->verticalScrollBar()->installEventFilter(this);
    mEdit->installEventFilter(this);

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

bool TextView::loadFile(const QString &fileName, QTextCodec *codec, bool initAnchor)
{
    if (mTextKind != FileText) return false;
    mMapper->setCodec(codec);

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
    mEdit->horizontalScrollBar()->setValue(mEdit->horizontalScrollBar()->value() + dx);
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
    return mEdit->wordUnderCursor();
}

void TextView::selectAllText()
{
    mEdit->selectAllText();
}

void TextView::clearSelection()
{
    mEdit->clearSelection();
    mMapper->clearSelection();
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

bool TextView::findText(const QRegularExpression &searchRegex, QTextDocument::FindFlags flags, bool &continueFind)
{
    bool found = mMapper->findText(searchRegex, flags, continueFind);
    if (found) {
        updateView();
        emit selectionChanged();
        mEdit->ensureCursorVisible();
    }
    return found;
}

void TextView::findInSelection(const QRegularExpression &searchRegex, FileMeta* file,
                               QList<search::Result> *results, bool showResults)
{
    if (!mEdit->hasSearchSelection()) {
        mEdit->updateSearchSelection();
        mMapper->updateSearchSelection();
    }
    if (!mEdit->hasSearchSelection()) return;
    SearchWorker sw(SearchFile(file), searchRegex, mMapper->searchSelectionStart(), mMapper->searchSelectionEnd(),
                    results, showResults);
    sw.findInFiles();
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

void TextView::horizontalScrollAction(int action)
{
    Q_UNUSED(action)
    mHScrollValue = mEdit->horizontalScrollBar()->sliderPosition();
}

void TextView::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
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
        int delta = static_cast<QWheelEvent*>(event)->angleDelta().y();
        mMapper->moveVisibleTopLine(delta / -40);
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
    QPoint pos = mMapper->position(true);
    bool cursorIsValid = pos.y() > AbstractTextMapper::cursorInvalid;
    QTextCursor::MoveMode mode = event->modifiers().testFlag(Qt::ShiftModifier) ? QTextCursor::KeepAnchor
                                                                                : QTextCursor::MoveAnchor;
    if (event == Hotkey::MoveToStartOfDoc) {
        mMapper->setVisibleTopLine(0);
        mMapper->setPosToAbsStart(mode);
        updatePosAndAnchor();
    } else if (event == Hotkey::MoveToStartOfLine && cursorIsValid) {
        mMapper->setPosRelative(pos.y(), 0, mode);
        updatePosAndAnchor();
    } else if (event == Hotkey::MoveToEndOfDoc) {
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
                if (cur.positionInBlock() < cur.block().length()-1) {
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
        if (mCurrentVisibleTopLine != mMapper->visibleTopLine() || mCurrentDataLength != dat.size()) {
            mCurrentVisibleTopLine = mMapper->visibleTopLine();
            mCurrentDataLength = dat.size();
            mEdit->setPlainText(dat);
        }
        if (mCurrentFormats != formats) {
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
            mEdit->setLineMarked(lineMarked);
        }
        updatePosAndAnchor();
        mEdit->updateExtraSelections();
        mEdit->protectWordUnderCursor(false);
        if (mEdit->verticalScrollBar()->sliderPosition())
            mEdit->verticalScrollBar()->setSliderPosition(0);
        mEdit->horizontalScrollBar()->setSliderPosition(mHScrollValue);
        mEdit->horizontalScrollBar()->setValue(mEdit->horizontalScrollBar()->sliderPosition());
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

void TextView::updatePosAndAnchor()
{
    QPoint pos = mMapper->position(true);
    QPoint anchor = mMapper->anchor(true);
    if (pos.y() == AbstractTextMapper::cursorInvalid
            || ( pos.y() < AbstractTextMapper::cursorInvalid && (pos.y() == anchor.y() || !mMapper->hasSelection()) )) {
        mEdit->setCursorWidth(0);
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
        int p = block.position() + qMax(0, qMin(anchor.x(), block.length()-1));
        cursor.setPosition(p);
        block = pos.y() == AbstractTextMapper::cursorBeforeStart ?
                    mEdit->document()->firstBlock()
                  : pos.y() == AbstractTextMapper::cursorBeyondEnd ?
                        mEdit->document()->lastBlock()
                      : mEdit->document()->findBlockByNumber(qMax(0, pos.y()));
        p = block.position() + qMax(0, qMin(pos.x(), block.length()-1));
        cursor.setPosition(p, QTextCursor::KeepAnchor);
    } else {
        QTextBlock block = pos.y() == AbstractTextMapper::cursorBeforeStart ?
                    mEdit->document()->firstBlock()
                  : pos.y() == AbstractTextMapper::cursorBeyondEnd ?
                        mEdit->document()->lastBlock()
                      : mEdit->document()->findBlockByNumber(qMax(0, pos.y()));
        int p = block.position() + qMax(0, qMin(pos.x(), block.length()-1));
        cursor.setPosition(p);
    }
    disconnect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    mEdit->setTextCursor(cursor);
    connect(mEdit, &TextViewEdit::updatePosAndAnchor, this, &TextView::updatePosAndAnchor);
    horizontalScrollAction(0);
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
