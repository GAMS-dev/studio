/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include <QMimeData>
#include <QTextBlock>
#include <QScrollBar>
#include <QToolTip>
#include <QTextDocumentFragment>
#include "editors/abstractedit.h"
#include "logger.h"
#include "keys.h"
#include "scheme.h"

namespace gams {
namespace studio {

AbstractEdit::AbstractEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    mSelUpdater.setSingleShot(true);
    mSelUpdater.setInterval(10);
    connect(&mSelUpdater, &QTimer::timeout, this, &AbstractEdit::internalExtraSelUpdate);
}

AbstractEdit::~AbstractEdit()
{
    mSelUpdater.stop();
}

void AbstractEdit::setOverwriteMode(bool overwrite)
{
    QPlainTextEdit::setOverwriteMode(overwrite);
}

bool AbstractEdit::overwriteMode() const
{
    return QPlainTextEdit::overwriteMode();
}

void AbstractEdit::sendToggleBookmark()
{
    FileId fi = fileId();
    if (fi.isValid()) {
        emit toggleBookmark(fi, absoluteBlockNr(textCursor().blockNumber()), textCursor().positionInBlock());
        emit updateRequest(rect(), 0);
    }
}

void AbstractEdit::sendJumpToNextBookmark()
{
    FileId fi = fileId();
    if (fi.isValid()) {
        emit jumpToNextBookmark(false, fi, absoluteBlockNr(textCursor().blockNumber()));
    }
}

void AbstractEdit::sendJumpToPrevBookmark()
{
    FileId fi = fileId();
    if (fi.isValid()) {
        emit jumpToNextBookmark(true, fi, absoluteBlockNr(textCursor().blockNumber()));
    }
}

QMimeData* AbstractEdit::createMimeDataFromSelection() const
{
    QMimeData* mimeData = new QMimeData();
    QTextCursor c = textCursor();
    QString plainTextStr = c.selection().toPlainText();
    mimeData->setText(plainTextStr);

    return mimeData;
}

void AbstractEdit::updateGroupId()
{
    marksChanged();
}

void AbstractEdit::disconnectTimers()
{
    disconnect(&mSelUpdater, &QTimer::timeout, this, &AbstractEdit::internalExtraSelUpdate);
}

void AbstractEdit::updateExtraSelections()
{
    mSelUpdater.start();
//    QList<QTextEdit::ExtraSelection> selections;
//    extraSelMarks(selections);
    //    setExtraSelections(selections);
}

void AbstractEdit::unfold(QTextBlock block)
{
    Q_UNUSED(block)
}

void AbstractEdit::setMarks(const LineMarks *marks)
{
    mMarks = marks;
    marksChanged();
}

const LineMarks* AbstractEdit::marks() const
{
    return mMarks;
}

int AbstractEdit::absoluteBlockNr(const int &localBlockNr) const
{
    return localBlockNr;
}

int AbstractEdit::localBlockNr(const int &absoluteBlockNr) const
{
    return absoluteBlockNr;
}

int AbstractEdit::topVisibleLine()
{
    QTextBlock block = firstVisibleBlock();
    return block.isValid() ? block.blockNumber() : 0;
}

void AbstractEdit::extraSelCurrentLine(QList<QTextEdit::ExtraSelection> &selections)
{
    if (!Settings::settings()->toBool(skEdHighlightCurrentLine)) return;

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(toColor(Scheme::Edit_currentLineBg));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.movePosition(QTextCursor::StartOfBlock);
    selection.cursor.clearSelection();
    selections.append(selection);
}

void AbstractEdit::extraSelMarks(QList<QTextEdit::ExtraSelection> &selections)
{
    if (!marks()) return;
    QTextBlock block = firstVisibleBlock();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int line = topVisibleLine();
    while (block.isValid() && top < viewport()->height()) {
        const QList<TextMark*> lm = marks()->values(line);
        for (const TextMark* m: lm) {
            QTextEdit::ExtraSelection selection;
            selection.cursor = textCursor();
            if (m->blockStart() < 0) continue;
            int start = m->blockStart();
            int end = m->size() ? (m->size() < 0 ? block.length() : m->blockEnd()+1) : 0;
            selection.cursor.setPosition(block.position() + start);
            selection.cursor.setPosition(block.position() + end, QTextCursor::KeepAnchor);
            if (m->type() == TextMark::error || m->refType() == TextMark::error) {
                if (m->refType() == TextMark::error)
                    selection.format.setForeground(m->color());
                selection.format.setUnderlineColor(toColor(Scheme::Normal_Red));
                if (m->size() == 1) {
                    selection.format.setBackground(toColor(Scheme::Edit_errorBg));
                    selection.format.setForeground(toColor(Scheme::Edit_text));
                }
                selection.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
                selection.format.setAnchorNames(QStringList()<<QString::number(m->line()));
            } else if (m->type() == TextMark::link) {
                selection.format.setForeground(m->color());
                selection.format.setUnderlineColor(m->color());
                selection.format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                selection.format.setAnchor(true);
                selection.format.setAnchorNames(QStringList()<<QString::number(m->line()));
            }
            selections << selection;
        }
        top += qRound(blockBoundingRect(block).height());
        block = block.next();
        ++line;
    }
}

void AbstractEdit::updateCursorShape(bool greedy)
{
    QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
    Qt::CursorShape shape = (mousePos.x() < 0) ? Qt::ArrowCursor : Qt::IBeamCursor;
    TextLinkType linkType = checkLinks(mousePos, greedy);
    if (linkType == linkMiss) shape = Qt::ForbiddenCursor;
    else if (linkType != linkNone && linkType != linkHide) shape = Qt::PointingHandCursor;
    viewport()->setCursor(shape);
}

QPoint AbstractEdit::toolTipPos(const QPoint &mousePos)
{
    mTipPos = mousePos;
    QPoint pos = mousePos;
    if (!mMarksAtMouse.isEmpty()) {
        QTextCursor cursor(document()->findBlockByNumber(localBlockNr(mMarksAtMouse.first()->line())));
        cursor.setPosition(cursor.position() + mMarksAtMouse.first()->column(), QTextCursor::MoveAnchor);
        pos.setY(cursorRect(cursor).bottom());
    } else {
        QTextCursor cursor = cursorForPosition(mousePos);
        cursor.setPosition(cursor.block().position());
        pos.setY(cursorRect(cursor).bottom());
    }
    if (pos.x() < 10) pos.setX(10);
    if (pos.x() > width()-100) pos.setX(width()-100);
    return pos;
}

QVector<int> AbstractEdit::toolTipLstNumbers(const QPoint &pos)
{
    Q_UNUSED(pos)
    QVector<int> lstLines;
    for (TextMark *mark: mMarksAtMouse) {
        int lstLine = mark->value();
        if (lstLine < 0 && mark->refMark()) lstLine = mark->refMark()->value();
        if (lstLine >= 0) lstLines << lstLine;
    }
    return lstLines;
}

LinePair AbstractEdit::findFoldBlock(int line, bool onlyThisLine) const
{
    Q_UNUSED(line)
    Q_UNUSED(onlyThisLine)
    return LinePair();
}

bool AbstractEdit::ensureUnfolded(int line)
{
    Q_UNUSED(line)
    return false;
}

TextLinkType AbstractEdit::checkLinks(const QPoint &mousePos, bool greedy, QString *fName)
{
    Q_UNUSED(fName)
    // greedy extends the scope (e.g. when control modifier is pressed)
    if (mMarks && !mMarks->isEmpty() && !mMarksAtMouse.isEmpty()) {
        return (!greedy && mousePos.x() > 0) ? linkHide
                                             : mMarksAtMouse.first()->linkExist() ? linkMark : linkMiss;
    }
    return linkNone;
}

void AbstractEdit::jumpToCurrentLink(const QPoint &mousePos)
{
    TextLinkType linkType = checkLinks(mousePos, true);
    if (linkType == linkMark)
        mMarksAtMouse.first()->jumpToRefMark();
}

void AbstractEdit::updateMarksAtMouse(QTextCursor cursor)
{
    QList<TextMark*> marks = mMarks->values(absoluteBlockNr(cursor.blockNumber()));
    mMarksAtMouse.clear();
    for (TextMark* mark: marks) {
        if ((!mark->groupId().isValid() || mark->groupId() == groupId()))
            mMarksAtMouse << mark;
    }
}

void AbstractEdit::internalExtraSelUpdate()
{
    QList<QTextEdit::ExtraSelection> selections;
    extraSelCurrentLine(selections);
    extraSelMarks(selections);
    setExtraSelections(selections);
}

//void AbstractEdit::showToolTip(const QList<TextMark*> &marks, const QPoint &pos)
//{
//    if (marks.size() > 0) {
//        QStringList tips;
//        emit requestMarkTexts(groupId(), marks, tips);
//        QToolTip::showText(mapToGlobal(pos), tips.join("\n"), this);
//    }
//}

QString AbstractEdit::getToolTipText(const QPoint &pos)
{
    QVector<int> lstLines = toolTipLstNumbers(pos);
    if (lstLines.isEmpty()) return QString();
    QStringList tips;
    emit requestLstTexts(groupId(), lstLines, tips);
    return tips.join("\n");
}

void AbstractEdit::updateToolTip(const QPoint &pos)
{
    QString text = getToolTipText(pos);
    if (!isToolTipValid(text, pos)) {
        if (text.isEmpty() || pos.isNull()) {
            mTipPos = QPoint();
            if (QToolTip::isVisible()) QToolTip::hideText();
        } else {
            mTipPos = pos;
            QToolTip::showText(mapToGlobal(toolTipPos(pos)), text, this);
        }
    }
}

bool AbstractEdit::isToolTipValid(QString text, const QPoint &pos)
{
    if (!text.isEmpty() && !pos.isNull()) {
        return (mTipPos-pos).manhattanLength() < 5 && text == QToolTip::text();
    }
    return !QToolTip::isVisible() && mTipPos.isNull();
}

bool AbstractEdit::event(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        e->ignore();
        return true;
    }
    if (e->type() == QEvent::FontChange) {
        QFontMetrics metric(font());
        setTabStopDistance(Settings::settings()->toInt(skEdTabSize) * metric.horizontalAdvance(' '));
    }
    return QPlainTextEdit::event(e);
}

void AbstractEdit::keyPressEvent(QKeyEvent *e)
{
    if (e == Hotkey::MoveViewLineUp) {
        verticalScrollBar()->setValue(verticalScrollBar()->value()-1);
        e->accept();
    } else if (e == Hotkey::MoveViewLineDown) {
        verticalScrollBar()->setValue(verticalScrollBar()->value()+1);
        e->accept();
    } else if (e == Hotkey::MoveViewPageUp) {
        verticalScrollBar()->setValue(verticalScrollBar()->value()-verticalScrollBar()->pageStep());
        e->accept();
    } else if (e == Hotkey::MoveViewPageDown) {
        verticalScrollBar()->setValue(verticalScrollBar()->value()+verticalScrollBar()->pageStep());
        e->accept();
    } else {
        QPlainTextEdit::keyPressEvent(e);
        if ((e->key() & 0x11111110) == Qt::Key_Home)
            emit verticalScrollBar()->valueChanged(verticalScrollBar()->value());
    }
    updateCursorShape(e->modifiers() & Qt::ControlModifier);
}

void AbstractEdit::keyReleaseEvent(QKeyEvent *e)
{
    QPlainTextEdit::keyReleaseEvent(e);
    if (e->key() == Qt::Key_Backspace)
        ensureUnfolded(textCursor().blockNumber());
    updateCursorShape(e->modifiers() & Qt::ControlModifier);
}

void AbstractEdit::mousePressEvent(QMouseEvent *e)
{
    QPlainTextEdit::mousePressEvent(e);
    if (!mMarksAtMouse.isEmpty()) {
        mClickPos = e->pos();
    } else if (e->button() == Qt::RightButton) {
        QTextCursor currentTC = textCursor();
        QTextCursor mouseTC = cursorForPosition(e->pos());
        if (currentTC.hasSelection()
                && (mouseTC.position() > qMin(currentTC.position(), currentTC.anchor())
                    && mouseTC.position() < qMax(currentTC.position(), currentTC.anchor())))
                return;
        setTextCursor(mouseTC);
    }
}

const QList<TextMark *> &AbstractEdit::marksAtMouse() const
{
    return mMarksAtMouse;
}

void AbstractEdit::mouseMoveEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseMoveEvent(e);
    bool offClickRegion = !mClickPos.isNull() && (mClickPos-e->pos()).manhattanLength() > 4;
    bool validLink = (isReadOnly() || e->pos().x() < 0 || e->modifiers() & Qt::ControlModifier) && !offClickRegion;

    if (mMarks /*&& validLink*/) {
        updateMarksAtMouse(cursorForPosition(e->pos()));
    } else {
        mMarksAtMouse.clear();
    }
    updateToolTip(e->pos());
    updateCursorShape(validLink);
}

void AbstractEdit::mouseReleaseEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseReleaseEvent(e);
    bool offClickRegion = !mClickPos.isNull() && (mClickPos-e->pos()).manhattanLength() > 4;
    bool validLink = (isReadOnly() || e->pos().x() < 0 || e->modifiers() & Qt::ControlModifier) && !offClickRegion;
    mClickPos = QPoint();
    if (validLink) jumpToCurrentLink(e->pos());
}

void AbstractEdit::marksChanged(const QSet<int> dirtyLines)
{
    mMarksAtMouse.clear();
    Q_UNUSED(dirtyLines)
}

void AbstractEdit::jumpTo(int line, int column)
{
    QTextCursor cursor;
    if (document()->blockCount()-1 < line) return;
    cursor = QTextCursor(document()->findBlockByNumber(line));
    cursor.clearSelection();
    cursor.setPosition(cursor.position() + column);
    setTextCursor(cursor);
    // center line vertically
    qreal visLines = qreal(rect().height()) / cursorRect().height();
    qreal visLine = qreal(cursorRect().bottom()) / cursorRect().height();
    int mv = qRound(visLine - visLines/2);
    if (qAbs(mv) > visLines/3)
        verticalScrollBar()->setValue(verticalScrollBar()->value()+mv);
}

}
}
