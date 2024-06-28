/*
 * This file is part of the GAMS Studio project.
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
#include "textviewedit.h"
#include "search/searchdialog.h"
#include "search/searchlocator.h"
#include "keys.h"
#include "theme.h"
#include "logger.h"
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>

namespace gams {
namespace studio {

static const int CAnyModifier = Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier|Qt::MetaModifier;

TextViewEdit::TextViewEdit(AbstractTextMapper &mapper, QWidget *parent)
    : CodeEdit(parent), mMapper(mapper), mSettings(Settings::settings())
{
    setMarks(nullptr);
    setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    setAllowBlockEdit(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    disconnect(&wordDelayTimer(), &QTimer::timeout, this, &CodeEdit::updateExtraSelections);
    setMouseTracking(true);
    connect(&mScrollTimer, &QTimer::timeout, this, &TextViewEdit::scrollStep);
}

void TextViewEdit::protectWordUnderCursor(bool protect)
{
    mKeepWordUnderCursor = protect;
}

bool TextViewEdit::hasSelection() const
{
    return mMapper.hasSelection();
}

void TextViewEdit::clearSelection()
{
    mMapper.clearSelection();
    emit updatePosAndAnchor();
}

void TextViewEdit::disconnectTimers()
{
    CodeEdit::disconnectTimers();
}

AbstractEdit::EditorType TextViewEdit::type() const
{
    return LstView;
}

int TextViewEdit::lineCount()
{
    QFontMetricsF metric(font());
    qreal lineHeight = qCeil(qMax(metric.lineSpacing(), metric.boundingRect("Äg").height()));
    return qFloor(qreal(viewport()->height()) / lineHeight);
}

void TextViewEdit::copySelection()
{
    int selSize = mMapper.selectionSize();
    if (selSize < 0) {
        QMessageBox::information(this, "Large selection", "Your selection is too large for the clipboard");
        return;
    }
    if (selSize > 5*1024*1024) {
        QStringList list;
        list << "KB" << "MB" << "GB";
        QStringListIterator i(list);
        QString unit("bytes");
        while (selSize >= 1024.0 && i.hasNext()) {
            unit = i.next();
            selSize = int(selSize / 1024.0);
        }
        QString text = QString("Your selection is very large (%1 %2). Do you want to proceed?").arg(selSize,'f',2).arg(unit);
        QMessageBox::StandardButton choice = QMessageBox::question(this, "Large selection", text);
        if (choice != QMessageBox::Yes) return;
    }
    mMapper.copyToClipboard();
}

void TextViewEdit::selectAllText()
{
    mMapper.selectAll();
    emit updatePosAndAnchor();
}

void TextViewEdit::scrollStep()
{
    if (!mScrollDelta) {
        mScrollTimer.stop();
        return;
    }
    int step = mScrollDelta > 0 ? 1 : -1;
    step = step + mScrollDelta / 10;
    mMapper.moveVisibleTopLine(step);
    QTextCursor cursor = QTextCursor(document());
    if (mScrollDelta > 0) {
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::Left);
    }
    mMapper.setPosRelative(cursor.blockNumber(), cursor.positionInBlock(), QTextCursor::KeepAnchor);
    emit topLineMoved();
    int msec = scrollMs(mScrollDelta);
    if (msec != mScrollTimer.interval())
        mScrollTimer.setInterval(msec);
}

int TextViewEdit::scrollMs(int delta)
{
    int msec = 500 - qMin(delta*delta, 495);
    return msec;
}

void TextViewEdit::setLineMarked(const QVector<bool> &newLineMarked)
{
    mLineMarked = newLineMarked;
}

void TextViewEdit::keyPressEvent(QKeyEvent *event)
{
//    if (event->key() == Qt::Key_Control) {
//        QPoint pos = mapFromGlobal(QCursor::pos());
//        QTextCursor cursor = cursorForPosition(pos);
//        emit findNearLst(cursor, false);
//        // modify the mouse-cursor if link found (done==true)
//    }
    if (event->key() >= Qt::Key_Home && event->key() <= Qt::Key_PageDown) {
        emit keyPressed(event);
        if (!event->isAccepted())
            CodeEdit::keyPressEvent(event);
    } else if (event == Hotkey::SelectAll) {
        selectAllText();
    } else if (event == Hotkey::Copy) {
        copySelection();
    } else {
        CodeEdit::keyPressEvent(event);
    }
}

void TextViewEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu();
    QAction *lastAct = nullptr;
    for (int i = menu->actions().count()-1; i >= 0; --i) {
        QAction *act = menu->actions().at(i);
        if (act->objectName() == "select-all") {
            if (blockEdit()) act->setEnabled(false);
            menu->removeAction(act);
            act->disconnect();
            connect(act, &QAction::triggered, this, &CodeEdit::selectAllText);
            menu->insertAction(lastAct, act);
        } else if (act->objectName() == "edit-paste" && act->isEnabled()) {
            menu->removeAction(act);
            act->disconnect();
        } else if (act->objectName() == "edit-cut") {
            menu->removeAction(act);
            act->disconnect();
        } else if (act->objectName() == "edit-copy") {
            menu->removeAction(act);
            act->disconnect();
            act->setEnabled(mMapper.hasSelection());
            act->setShortcut(QKeySequence("Ctrl+C"));
            connect(act, &QAction::triggered, this, &TextViewEdit::copySelection);
            menu->insertAction(lastAct, act);
        } else if (act->objectName() == "edit-delete") {
            menu->removeAction(act);
            act->disconnect();
        }
        lastAct = act;
    }
    QAction act("Clear Log", this);
    if (mMapper.kind() == AbstractTextMapper::memoryMapper) {
        connect(&act, &QAction::triggered, &mMapper, &AbstractTextMapper::reset);
        menu->addAction(&act);
    }

    menu->exec(e->globalPos());
    delete menu;
}

void TextViewEdit::recalcWordUnderCursor()
{
    if (!mKeepWordUnderCursor) {
        CodeEdit::recalcWordUnderCursor();
    }
}

int TextViewEdit::absoluteBlockNr(const int &localBlockNr) const
{
    int res = mMapper.visibleTopLine();
    if (res < 0) {
        res -= localBlockNr;
    } else {
        res += localBlockNr;
    }
    return res;
}

int TextViewEdit::localBlockNr(const int &absoluteBlockNr) const
{
    int res = mMapper.visibleTopLine();
    if (res < 0) {
        res = absoluteBlockNr + res + 1;
    } else {
        res = absoluteBlockNr - res + 1;
    }
    return res;
}

void TextViewEdit::extraSelCurrentLine(QList<QTextEdit::ExtraSelection> &selections)
{
    Q_UNUSED(selections)
    return;
}

void TextViewEdit::extraSelLineMarks(QList<QTextEdit::ExtraSelection> &selections)
{
    QTextBlock block = firstVisibleBlock();
    int i = 0;
    while (block.isValid()) {
        if (mLineMarked.size() > i && mLineMarked.at(i)) {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(toColor(Theme::Edit_logRemoteBk));
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = QTextCursor(block);
            selections.append(selection);
        }
        block = block.next();
        ++i;
    }
}

void TextViewEdit::mousePressEvent(QMouseEvent *e)
{
    setCursorWidth(2);
    if (!marks() || marks()->isEmpty()) {
        QTextCursor cursor = cursorForPosition(e->pos());
        setLinkClickPos(e->pos());
        mClickStart = !(e->modifiers() & Qt::ShiftModifier);
        if (!resolveHRef(cursor.charFormat().anchorHref()).isEmpty() && !(e->modifiers() & CAnyModifier)) {
            CodeEdit::mousePressEvent(e);
            return;
        }
        if (e->buttons() == Qt::LeftButton) {
            if (!mClickStart)
                mMapper.setPosRelative(cursor.blockNumber(), cursor.positionInBlock(), QTextCursor::KeepAnchor);
            else mMapper.setPosRelative(cursor.blockNumber(), cursor.positionInBlock(), QTextCursor::MoveAnchor);
        }
    }
    CodeEdit::mousePressEvent(e);
}

void TextViewEdit::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::LeftButton && !(e->modifiers() & CAnyModifier)) {
        mScrollDelta = e->position().y() < 0 ? e->position().y() : (e->position().y() < viewport()->height() ? 0 : e->position().y() - viewport()->height());
        if (mScrollDelta) {
            if (!mScrollTimer.isActive()) {
                mScrollTimer.start(0);
            } else {
                int remain = qMax(0, scrollMs(mScrollDelta) - mScrollTimer.interval() + mScrollTimer.remainingTime());
                mScrollTimer.start(remain);
            }
        } else {
            mScrollTimer.stop();
            bool valid = mClickStart && (linkClickPos() - e->pos()).manhattanLength() < 4;
            if (valid) return;
            viewport()->setCursor((e->pos().x() < 0) ? Qt::ArrowCursor : Qt::IBeamCursor);
            mClickStart = false;
            QTextCursor cursor = cursorForPosition(e->pos());
            mMapper.setPosRelative(cursor.blockNumber(), cursor.positionInBlock(), QTextCursor::KeepAnchor);
            emit updatePosAndAnchor();
        }
    } else {
        CodeEdit::mouseMoveEvent(e);
    }
}

void TextViewEdit::mouseReleaseEvent(QMouseEvent *e)
{
    CodeEdit::mouseReleaseEvent(e);
    mScrollDelta = 0;
    mScrollTimer.stop();
}

void TextViewEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
    QTextCursor cursor = cursorForPosition(event->pos());
    if (event->modifiers() & Qt::ControlModifier || mMapper.kind() != AbstractTextMapper::memoryMapper) {
        QTextCursor cur = textCursor();
        cur.movePosition(QTextCursor::StartOfWord);
        int from = cur.positionInBlock();
        cur.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        emit selectWord(cur.blockNumber(), from, cur.positionInBlock());
    } else {
        QRect rect = cursorRect(cursor);
        if (event->pos().x() - rect.right() < 3 && !resolveHRef(cursor.charFormat().anchorHref()).isEmpty()) return;
        emit findClosestLstRef(cursor);
    }
}

TextLinkType TextViewEdit::checkLinks(const QPoint &mousePos, bool greedy, QString *fName)
{
    TextLinkType res = linkNone;
    if (!marks() || marks()->isEmpty()) {
        QTextCursor cur = cursorForPositionCut(mousePos);
        if (!cur.charFormat().anchorHref().isEmpty()) {
            QString fileName = resolveHRef(cur.charFormat().anchorHref());
            res = (fileName.isEmpty()) ? linkMiss : linkMark;
            if (fName) *fName = fileName;
        }
    } else {
        res = CodeEdit::checkLinks(mousePos, greedy, fName);
    }
    return res;
}

//bool TextViewEdit::viewportEvent(QEvent *event)
//{
//    if (event->type() == QEvent::Resize) {
//        mResizeTimer.start();
//    }
//    return QAbstractScrollArea::viewportEvent(event);
//}

QVector<int> TextViewEdit::toolTipLstNumbers(const QPoint &mousePos)
{
    QVector<int> res = CodeEdit::toolTipLstNumbers(mousePos);
    if (res.isEmpty()) {
        QTextCursor cur = cursorForPositionCut(mousePos);
        if (cur.isNull()) return res;
        cur.setPosition(cur.block().position());
        if (cur.charFormat().anchorHref().length() > 4 && cur.charFormat().anchorHref().at(3) == ':') {
            bool ok = false;
            int lstNr = cur.charFormat().anchorHref().mid(4, cur.charFormat().anchorHref().length()-4).toInt(&ok);
            if (ok) res << lstNr;
        }
    }
    return res;
}

void TextViewEdit::paintEvent(QPaintEvent *e)
{
    AbstractEdit::paintEvent(e);
}

QString TextViewEdit::getToolTipText(const QPoint &pos)
{
    QString res = AbstractEdit::getToolTipText(pos);
    if (!res.isEmpty()) return res;
    QString fileName;
    checkLinks(pos, true, &fileName);
    if (!fileName.isEmpty()) {
        fileName = QDir::toNativeSeparators(fileName);
        fileName = "<p style='white-space:pre'>"+fileName;
    }
    return fileName;
}

int TextViewEdit::topVisibleLine()
{
    return mMapper.visibleTopLine();
}

} // namespace studio
} // namespace gams
