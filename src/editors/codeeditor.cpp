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
#include <QtWidgets>
#include "editors/codeeditor.h"
#include "studiosettings.h"
#include "searchwidget.h"
#include "exception.h"
#include "logger.h"
#include "syntax.h"
#include "keys.h"

namespace gams {
namespace studio {

inline const KeySeqList &hotkey(Hotkey _hotkey) { return Keys::instance().keySequence(_hotkey); }

inline int charCategory(const QChar &ch)
{
    if (ch.isSpace()) return 0;
    if (ch.isLetterOrNumber() || ch=='.' || ch==',') return 1;
    if (ch.isPunct()) return 2;
    return 3;
}

inline void nextCharClass(int offset,int &size, const QString &text)
{
    bool hadSpace = false;
    if (size+offset < text.length()) {
        int startCat = charCategory(text.at(size+offset));
        while (++size+offset < text.length()) {
            int cat = charCategory(text.at(size+offset));
            if (startCat == 0 || cat == 0) hadSpace = true;
            if (startCat == 0) startCat = cat;
            if (startCat > 0 && cat > 0 && (startCat != cat || hadSpace)) return;
        }
    } else {
        ++size;
    }
}

inline void prevCharClass(int offset, int &size, const QString &text)
{
    bool hadSpace = false;
    if (size+offset > 0) {
        --size;
        int startCat = (size+offset < text.length()) ? charCategory(text.at(size+offset)) : 0;
        if (size+offset == 0) return;
        while (--size+offset > 0) {
            int cat = (size+offset < text.length()) ? charCategory(text.at(size+offset)) : 0;
            if (startCat == 0 || cat == 0) hadSpace = true;
            if (startCat == 0) startCat = cat;
            if (startCat > 0 && cat > 0 && (startCat != cat || hadSpace)) {
                ++size;
                return;
            }
        }
    }
}

CodeEditor::CodeEditor(StudioSettings *settings, QWidget *parent)
    : AbstractEditor(settings, parent)
{
    mLineNumberArea = new LineNumberArea(this);
    mLineNumberArea->setMouseTracking(true);
    mBlinkBlockEdit.setInterval(500);
    mWordDelay.setSingleShot(true);
    mParenthesesDelay.setSingleShot(true);

    connect(&mBlinkBlockEdit, &QTimer::timeout, this, &CodeEditor::blockEditBlink);
    connect(&mWordDelay, &QTimer::timeout, this, &CodeEditor::updateExtraSelections);
    connect(&mParenthesesDelay, &QTimer::timeout, this, &CodeEditor::updateExtraSelections);
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::recalcExtraSelections);
    connect(this, &CodeEditor::textChanged, this, &CodeEditor::recalcExtraSelections);
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &CodeEditor::updateExtraSelections);

    updateLineNumberAreaWidth(0);
    recalcExtraSelections();
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    if(mSettings->lineWrapEditor())
        setLineWrapMode(AbstractEditor::WidgetWidth);
    else
        setLineWrapMode(AbstractEditor::NoWrap);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 0;

    if(mSettings->showLineNr())
        space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    bool marksEmpty = true;
    emit requestMarksEmpty(&marksEmpty);
    space += (marksEmpty ? 0 : iconSize());

    return space;
}

int CodeEditor::iconSize()
{
    return fontMetrics().height()-3;
}

LineNumberArea* CodeEditor::lineNumberArea()
{
    return mLineNumberArea;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        mLineNumberArea->scroll(0, dy);
    } else {
        int top = rect.y();
        int bottom = top + rect.height();
        QTextBlock b = firstVisibleBlock();
        while (b.isValid() && b.isVisible()) {
            QRect blockBounds = blockBoundingGeometry(b).translated(contentOffset()).toAlignedRect();
            if (top > blockBounds.top() && top < blockBounds.bottom())
                top = blockBounds.top();
            if (bottom > blockBounds.top() && bottom < blockBounds.bottom()-1)
                bottom = blockBounds.bottom()-1;
            if (blockBounds.bottom() >= rect.bottom())
                break;
            b = b.next();
        }
        mLineNumberArea->update(0, top, mLineNumberArea->width(), bottom-top);
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::blockEditBlink()
{
    if (mBlockEdit) mBlockEdit->refreshCursors();
}

void CodeEditor::clearSelection()
{
    if (isReadOnly()) return;
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->replaceBlockText(QStringList()<<QString());
    } else {
        textCursor().clearSelection();
    }
}

void CodeEditor::cutSelection()
{
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->selectionToClipboard();
        mBlockEdit->replaceBlockText(QStringList()<<QString());
    } else {
        cut();
    }
}

void CodeEditor::copySelection()
{
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->selectionToClipboard();
    } else {
        copy();
    }
}

void CodeEditor::pasteClipboard()
{
    bool isBlock;
    QStringList texts = clipboard(&isBlock);
    if (!mBlockEdit) {
        if (isBlock) {
            QTextCursor c = textCursor();
            if (c.hasSelection()) c.removeSelectedText();
            startBlockEdit(c.blockNumber(), c.columnNumber());
            mBlockEdit->replaceBlockText(texts);
        } else {
            paste();
        }
    } else {
        mBlockEdit->replaceBlockText(texts);
    }
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    AbstractEditor::resizeEvent(e);

    QRect cr = contentsRect();
    mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    updateExtraSelections();
}

void CodeEditor::keyPressEvent(QKeyEvent* e)
{
    if (!mBlockEdit && e == Hotkey::BlockEditStart) {
        QTextCursor c = textCursor();
        QTextCursor anc = c;
        anc.setPosition(c.anchor());
        startBlockEdit(anc.blockNumber(), anc.columnNumber());
    }

    if (mBlockEdit) {
        if (e->key() == Hotkey::NewLine || e == Hotkey::BlockEditEnd || e == Hotkey::Undo || e == Hotkey::Redo) {
            endBlockEdit();
        } else {
            mBlockEdit->keyPressEvent(e);
            return;
        }
    } else {
        if (e == Hotkey::MatchParentheses || e == Hotkey::SelectParentheses) {
            ParenthesesMatch pm = matchParentheses();
            QTextCursor::MoveMode mm = (e == Hotkey::SelectParentheses) ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
            if (pm.inOutMatch >= 0) {
                QTextCursor cur = textCursor();
                cur.setPosition(pm.inOutMatch, mm);
                setTextCursor(cur);
            }
        } else if (e == Hotkey::MoveCharGroupRight || e == Hotkey::SelectCharGroupRight) {
            QTextCursor::MoveMode mm = (e == Hotkey::SelectCharGroupRight) ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
            QTextCursor cur = textCursor();
            int p = cur.positionInBlock();
            nextCharClass(0, p, cur.block().text());
            if (p >= cur.block().length()) {
                QTextBlock block = cur.block().next();
                if (block.isValid()) cur.setPosition(block.position(), mm);
                else cur.movePosition(QTextCursor::EndOfBlock, mm);
            } else {
                cur.setPosition(cur.block().position() + p, mm);
            }
            setTextCursor(cur);
            e->accept();
            return;
        } else if (e == Hotkey::MoveCharGroupLeft || e == Hotkey::SelectCharGroupLeft) {
            QTextCursor::MoveMode mm = (e == Hotkey::SelectCharGroupLeft) ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
            QTextCursor cur = textCursor();
            int p = cur.positionInBlock();
            if (p == 0) {
                QTextBlock block = cur.block().previous();
                if (block.isValid()) cur.setPosition(block.position()+block.length()-1, mm);
            } else {
                prevCharClass(0, p, cur.block().text());
                cur.setPosition(cur.block().position() + p, mm);
            }
            setTextCursor(cur);
            e->accept();
            return;
        }
    }

    if (!isReadOnly()) {
        if (e->key() == Hotkey::NewLine) {
            QTextCursor cursor = textCursor();
            int pos = cursor.positionInBlock();
            cursor.beginEditBlock();
            QString leadingText = cursor.block().text().left(pos).trimmed();
            if (leadingText.isEmpty()) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.insertText("\n");
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);
            } else {
                cursor.insertText("\n");
                if (cursor.block().previous().isValid())
                    truncate(cursor.block().previous());
                adjustIndent(cursor);
            }
            cursor.endEditBlock();
            setTextCursor(cursor);
            e->accept();
            return;
        }
        if (e == Hotkey::Indent) {
            indent(mSettings->tabSize());
            e->accept();
            return;
        }
        if (e == Hotkey::Outdent) {
            indent(-mSettings->tabSize());
            e->accept();
            return;
        }
    }

    if (e->modifiers() & Qt::ShiftModifier && (e->key() == Qt::Key_F3))
        emit searchFindPrevPressed();
    else if (e->key() == Qt::Key_F3)
        emit searchFindNextPressed();

    AbstractEditor::keyPressEvent(e);
}


void CodeEditor::keyReleaseEvent(QKeyEvent* e)
{
    if (isReadOnly()) {
        AbstractEditor::keyReleaseEvent(e);
        return;
    }
    // return pressed: ignore here
    if (!isReadOnly() && e->key() == Hotkey::NewLine) {
        e->accept();
        return;
    } else {
        AbstractEditor::keyReleaseEvent(e);
    }
}

void CodeEditor::adjustIndent(QTextCursor cursor)
{
    if (!mSettings->autoIndent()) return;

    QRegularExpression rex("^(\\s*).*$");
    QRegularExpressionMatch match = rex.match(cursor.block().text());
    if (match.hasMatch()) {
        QTextBlock prev = cursor.block().previous();
        QRegularExpression pRex("^((\\s+)|([^\\s]+))");
        QRegularExpressionMatch pMatch = pRex.match(prev.text());
        while (true) {
            if (pMatch.hasMatch()) {
                if (pMatch.capturedLength(2) < 1)
                    break;
                QString spaces = pMatch.captured(2);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, match.capturedLength(1));
                cursor.removeSelectedText();
                cursor.insertText(spaces);
                setTextCursor(cursor);
                break;
            }
            prev = prev.previous();
            if (!prev.isValid() || prev.blockNumber() < cursor.blockNumber()-50)
                break;
            pMatch = pRex.match(prev.text());
        }
    }
}


void CodeEditor::truncate(QTextBlock block)
{
    QRegularExpression pRex("(^.*[^\\s]|^)(\\s+)$");
    QRegularExpressionMatch match = pRex.match(block.text());
    if (match.hasMatch()) {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, match.capturedLength(2));
        cursor.removeSelectedText();
    }
}

int CodeEditor::textCursorColumn(QPoint mousePos)
{
    QTextCursor cursor = cursorForPosition(mousePos);
    int col = cursor.columnNumber();
    int addX = mousePos.x()-cursorRect(cursor).right();
    if (addX > 0) {
        QFontMetrics metric(font());
        col += addX / metric.width(' ');
    }
    return col;
}

void CodeEditor::mousePressEvent(QMouseEvent* e)
{
    this->setContextMenuPolicy(Qt::DefaultContextMenu);
    if (e->modifiers() == (Qt::AltModifier | Qt::ShiftModifier))
        this->setContextMenuPolicy(Qt::PreventContextMenu);
    if (e->modifiers() & Qt::AltModifier) {
        QTextCursor cursor = cursorForPosition(e->pos());
        QTextCursor anchor = textCursor();
        anchor.setPosition(anchor.anchor());
        if (e->modifiers() == Qt::AltModifier) {
            if (mBlockEdit) endBlockEdit();
            startBlockEdit(cursor.blockNumber(), textCursorColumn(e->pos()));
        } else if (e->modifiers() == (Qt::AltModifier | Qt::ShiftModifier)) {
            if (mBlockEdit) mBlockEdit->selectTo(cursor.blockNumber(), textCursorColumn(e->pos()));
            else startBlockEdit(anchor.blockNumber(), anchor.columnNumber());
        }
        if (mBlockEdit) {
            mBlockEdit->selectTo(cursor.blockNumber(), textCursorColumn(e->pos()));
            emit cursorPositionChanged();
        }
    } else {
        if (mBlockEdit && (e->modifiers() || e->buttons() != Qt::RightButton))
            endBlockEdit();
        AbstractEditor::mousePressEvent(e);
    }
}

void CodeEditor::mouseMoveEvent(QMouseEvent* e)
{
    if (mBlockEdit) {
        if ((e->buttons() & Qt::LeftButton) && (e->modifiers() & Qt::AltModifier)) {
            mBlockEdit->selectTo(cursorForPosition(e->pos()).blockNumber(), textCursorColumn(e->pos()));
        }
    } else
        AbstractEditor::mouseMoveEvent(e);
}

void CodeEditor::wheelEvent(QWheelEvent *e) {
    if (e->modifiers() & Qt::ControlModifier) {
        const int delta = e->delta();
        if (delta < 0) {
            int pix = fontInfo().pixelSize();
            zoomOut();
            if (pix == fontInfo().pixelSize() && fontInfo().pointSize() > 1) zoomIn();
        } else if (delta > 0) {
            int pix = fontInfo().pixelSize();
            zoomIn();
            if (pix == fontInfo().pixelSize()) zoomOut();
        }
        updateTabSize();
        return;
    }
    AbstractEditor::wheelEvent(e);
}

void CodeEditor::paintEvent(QPaintEvent* e)
{
    int cw = mBlockEdit ? 0 : 2;
    if (cursorWidth()!=cw) setCursorWidth(cw);
    AbstractEditor::paintEvent(e);
    if (mBlockEdit) {
        mBlockEdit->paintEvent(e);
    }
}

void CodeEditor::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu *menu = createStandardContextMenu();
    bool hasBlockSelection = mBlockEdit && !mBlockEdit->blockText().isEmpty();
    QAction *lastAct = nullptr;
    for (int i = menu->actions().count()-1; i >= 0; --i) {
        QAction *act = menu->actions().at(i);
        if (act->objectName() == "select-all" && mBlockEdit) {
            act->setEnabled(false);
        } else if (act->objectName() == "edit-paste" && act->isEnabled()) {
            menu->removeAction(act);
            act->disconnect();
            connect(act, &QAction::triggered, this, &CodeEditor::pasteClipboard);
            menu->insertAction(lastAct, act);
        } else if (hasBlockSelection) {
            if (act->objectName() == "edit-cut") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEditor::cutSelection);
                menu->insertAction(lastAct, act);
            } else if (act->objectName() == "edit-copy") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEditor::copySelection);
                menu->insertAction(lastAct, act);
            } else if (act->objectName() == "edit-delete") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEditor::clearSelection);
                menu->insertAction(lastAct, act);
            }
        }
        lastAct = act;
    }
    QMenu *submenu = menu->addMenu(tr("Advanced"));
    QList<QAction*> ret;
    emit requestAdvancedActions(&ret);
    submenu->addActions(ret);
    menu->exec(e->globalPos());
    delete menu;
}

void CodeEditor::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls()) {
        e->ignore(); // paste to parent widget
    } else {
        AbstractEditor::dragEnterEvent(e);
    }
}

void CodeEditor::duplicateLine()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.insertText(cursor.block().text()+'\n');
    cursor.endEditBlock();
}

void CodeEditor::removeLine()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}

void CodeEditor::commentLine()
{
    QPoint beFrom;
    QPoint beTo;
    bool inBlockEdit = mBlockEdit;
    QTextCursor cursor = textCursor();
    if (inBlockEdit) {
        beFrom.setX(mBlockEdit->colTo());
        beTo.setX(mBlockEdit->colFrom());
        beFrom.setY(mBlockEdit->startLine());
        beTo.setY(mBlockEdit->currentLine());
        QTextBlock anc = cursor.document()->findBlockByNumber(mBlockEdit->startLine());
        cursor.setPosition(anc.position() + mBlockEdit->colFrom());
        cursor.setPosition(textCursor().block().position() + mBlockEdit->colTo(), QTextCursor::KeepAnchor);
        endBlockEdit();
    }
    QTextBlock startBlock = cursor.document()->findBlock(qMin(cursor.position(), cursor.anchor()));
    int lastBlockNr = cursor.document()->findBlock(qMax(cursor.position(), cursor.anchor())).blockNumber();
    bool removeComment = true;
    for (QTextBlock block = startBlock; block.blockNumber() <= lastBlockNr; block = block.next()) {
        if (!block.text().startsWith('*')) {
            removeComment = false;
            break;
        }
    }
    cursor.beginEditBlock();
    QTextCursor anchor = cursor;
    anchor.setPosition(anchor.anchor());
    for (QTextBlock block = startBlock; block.blockNumber() <= lastBlockNr; block = block.next()) {
        cursor.setPosition(block.position());
        if (removeComment) {
            cursor.deleteChar();
        } else {
            cursor.insertText("*");
        }
    }
    cursor.setPosition(anchor.position());
    cursor.setPosition(textCursor().position(), QTextCursor::KeepAnchor);
    cursor.endEditBlock();
    setTextCursor(cursor);
    if (inBlockEdit) {
        int offset = removeComment ? -1 : 1;
        startBlockEdit(beFrom.y(), beFrom.x()+offset);
        mBlockEdit->selectTo(beTo.y(), beTo.x()+offset);
    }
    recalcExtraSelections();
}

int CodeEditor::minIndentCount(int fromLine, int toLine)
{
    QTextCursor cursor = textCursor();
    QTextBlock block = (fromLine < 0) ? document()->findBlock(cursor.anchor()) : document()->findBlockByNumber(fromLine);
    QTextBlock last = (toLine < 0) ? document()->findBlock(cursor.position()) : document()->findBlockByNumber(toLine);
    if (block.blockNumber() > last.blockNumber()) qSwap(block, last);
    int res = block.text().length();
    QRegularExpression rex("^(\\s*)");
    while (true) {
        QRegularExpressionMatch match = rex.match(block.text());
        if (res > match.capturedLength(1)) res = match.capturedLength(1);
        if (block == last) break;
        block = block.next();
    }
    return res;
}

int CodeEditor::indent(int size, int fromLine, int toLine)
{
    if (!size) return 0;
    QTextCursor savePos;
    QTextCursor saveAnc;
    // determine affected lines
    bool force = true;
    if (fromLine < 0 || toLine < 0) {
        if (fromLine >= 0) toLine = fromLine;
        else if (toLine >= 0) fromLine = toLine;
        else if (mBlockEdit) {
            force = false;
            fromLine = mBlockEdit->startLine();
            toLine = mBlockEdit->currentLine();
        } else {
            force = textCursor().hasSelection();
            fromLine = document()->findBlock(textCursor().anchor()).blockNumber();
            toLine = textCursor().block().blockNumber();
        }
    }
    if (force) {
        savePos = textCursor();
        saveAnc = textCursor();
        saveAnc.setPosition(saveAnc.anchor());
    }
    if (fromLine > toLine) qSwap(fromLine, toLine);

    // smallest indent of affected lines
    QTextBlock block = document()->findBlockByNumber(fromLine);
    int minIndentPos = block.length();
    while (block.isValid() && block.blockNumber() <= toLine) {
        QString text = block.text();
        int w = 0;
        while (w < text.length() && text.at(w).isSpace()) w++;
        if (w < text.length() && w < minIndentPos) minIndentPos = w;
        block = block.next();
    }

    // adjust justInsert if current position is beyond minIndentPos
    if (!force) {
        if (mBlockEdit)
            force = (mBlockEdit->colFrom() != mBlockEdit->colTo() || mBlockEdit->colFrom() <= minIndentPos);
        else {
            force = (textCursor().positionInBlock() <= minIndentPos);
        }
    }
    // determine insertPos
    int insertPos = mBlockEdit ? mBlockEdit->colFrom() : textCursor().positionInBlock();
    if (force) insertPos = minIndentPos;
    if (size < 0 && insertPos == 0) return 0;

    // check if all lines contain enough spaces to remove them
    if (size < 0 && !force && insertPos+size >= 0) {
        bool canRemoveSpaces = true;
        block = document()->findBlockByNumber(fromLine);
        while (block.isValid() && block.blockNumber() <= toLine && canRemoveSpaces) {
            QString text = block.text();
            int w = insertPos + size;
            while (w < text.length() && w < insertPos) {
                if (!text.at(w).isSpace()) canRemoveSpaces = false;
                w++;
            }
            block = block.next();
        }
        if (!canRemoveSpaces) return 0;
    }

    // store current blockEdit mode
    bool inBlockEdit = mBlockEdit;
    QPoint beFrom;
    QPoint beTo;
    if (mBlockEdit) {
        beFrom = QPoint(mBlockEdit->colTo(), mBlockEdit->startLine());
        beTo = QPoint(mBlockEdit->colFrom(), mBlockEdit->currentLine());
        endBlockEdit();
    }

    // perform deletion
    int charCount;
    if (size < 0) charCount = ((insertPos-1) % qAbs(size)) + 1;
    else charCount = size - insertPos % size;
    QString insText(charCount, ' ');
    block = document()->findBlockByNumber(fromLine);
    QTextCursor editCursor = textCursor();
    editCursor.beginEditBlock();
    while (block.isValid() && block.blockNumber() <= toLine) {
        editCursor.setPosition(block.position());
        if (size < 0) {
            if (insertPos - charCount < block.length()) {
                editCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, insertPos - charCount);
                int tempCount = qMin(charCount, block.length() - (insertPos - charCount));
                editCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, tempCount);
                editCursor.removeSelectedText();
            }
        } else {
            if (insertPos < block.length()) {
                editCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, insertPos);
                editCursor.insertText(insText);
            }
        }
        block = block.next();
    }
    editCursor.endEditBlock();
    // restore normal selection
    if (!savePos.isNull()) {
        editCursor.setPosition(saveAnc.position());
        editCursor.setPosition(savePos.position(), QTextCursor::KeepAnchor);
    }
    setTextCursor(editCursor);

    if (inBlockEdit) {
        int add = (size > 0) ? charCount : -charCount;
        startBlockEdit(beFrom.y(), qMax(beFrom.x() + add, 0));
        mBlockEdit->selectTo(beTo.y(), qMax(beTo.x() + add, 0));
    }
    return charCount;
}

void CodeEditor::startBlockEdit(int blockNr, int colNr)
{
    if (mBlockEdit) endBlockEdit();
    bool overwrite = overwriteMode();
    if (overwrite) setOverwriteMode(false);
    mBlockEdit = new BlockEdit(this, blockNr, colNr);
    mBlockEdit->setOverwriteMode(overwrite);
    mBlockEdit->startCursorTimer();
    updateLineNumberAreaWidth(0);
}

void CodeEditor::endBlockEdit()
{
    mBlockEdit->stopCursorTimer();
    mBlockEdit->adjustCursor();
    bool overwrite = mBlockEdit->overwriteMode();
    delete mBlockEdit;
    mBlockEdit = nullptr;
    setOverwriteMode(overwrite);
}

void dumpClipboard()
{
    QClipboard *clip = QGuiApplication::clipboard();
    const QMimeData * mime = clip->mimeData();
    DEB() << "----------------- Clipboard --------------------" ;
    DEB() << "  C " << clip->ownsClipboard()<< "  F " << clip->ownsFindBuffer()<< "  S " << clip->ownsSelection();
    for (QString fmt: mime->formats()) {
        DEB() << "   -- " << fmt << "   L:" << mime->data(fmt).length()
              << "\n" << mime->data(fmt)
              << "\n" << QString(mime->data(fmt).toHex(' '));
    }
}

QStringList CodeEditor::clipboard(bool *isBlock)
{
//    dumpClipboard();
    QString mimes = "|" + QGuiApplication::clipboard()->mimeData()->formats().join("|") + "|";
    bool asBlock = (mimes.indexOf("MSDEVColumnSelect") >= 0) || (mimes.indexOf("Borland IDE Block Type") >= 0);
    QStringList texts = QGuiApplication::clipboard()->mimeData()->text().split("\n");
    if (texts.last().isEmpty()) texts.removeLast();
    if (!asBlock || texts.count() <= 1) {
        if (isBlock) *isBlock = false;
        texts = QStringList() << QGuiApplication::clipboard()->mimeData()->text();
    }
    if (isBlock && (asBlock || mBlockEdit))
        *isBlock = true;

    return texts;
}

CharType CodeEditor::charType(QChar c)
{
    switch (c.category()) {
    case QChar::Number_DecimalDigit:
        return CharType::Number;
    case QChar::Separator_Space:
    case QChar::Separator_Line:
    case QChar::Separator_Paragraph:
        return CharType::Seperator;
    case QChar::Letter_Uppercase:
        return CharType::LetterUCase;
    case QChar::Letter_Lowercase:
        return CharType::LetterLCase;
    case QChar::Punctuation_Dash:
    case QChar::Punctuation_Open:
    case QChar::Punctuation_Close:
    case QChar::Punctuation_InitialQuote:
    case QChar::Punctuation_FinalQuote:
    case QChar::Punctuation_Other:
    case QChar::Symbol_Math:
    case QChar::Symbol_Currency:
    case QChar::Symbol_Modifier:
    case QChar::Symbol_Other:
        return CharType::Punctuation;
    default:
        break;
    }
    return CharType::Other;
}

void CodeEditor::updateTabSize()
{
    QFontMetrics metric(font());
    setTabStopDistance(8*metric.width(' '));
}

int CodeEditor::findAlphaNum(const QString &text, int start, bool back)
{
    QChar c = ' ';
    int pos = (back && start == text.length()) ? start-1 : start;
    while (pos >= 0 && pos < text.length()) {
        c = text.at(pos);
        if (!c.isLetterOrNumber() && c != '_' && (pos != start || !back)) break;
        pos = pos + (back?-1:1);
    }
    pos = pos - (back?-1:1);
    if (pos == start) {
        c = (pos >= 0 && pos < text.length()) ? text.at(pos) : ' ';
        if (!c.isLetterOrNumber() && c != '_') return -1;
    }
    if (pos >= 0 && pos < text.length()) { // must not start with number
        c = text.at(pos);
        if (!c.isLetterOrNumber() && c != '_') return -1;
    }
    return pos;
}

CodeEditor::BlockEdit *CodeEditor::blockEdit() const
{
    return mBlockEdit;
}

AbstractEditor::EditorType CodeEditor::type()
{
    return EditorType::CodeEditor;
}

void CodeEditor::wordInfo(QTextCursor cursor, QString &word, int &intState)
{
    QString text = cursor.block().text();
    int start = cursor.positionInBlock();
    int from = findAlphaNum(text, start, true);
    int to = findAlphaNum(text, from, false);
    if (from >= 0 && from <= to) {
        word = text.mid(from, to-from+1);
        start = from + cursor.block().position();
        emit requestSyntaxState(start+1, intState);
//        cursor.setPosition(start+1);
//        intState = cursor.charFormat().property(QTextFormat::UserProperty).toInt();
    } else {
        word = "";
        intState = 0;
    }
}

void CodeEditor::getPositionAndAnchor(QPoint &pos, QPoint &anchor)
{
    if (mBlockEdit) {
        pos = QPoint(mBlockEdit->colFrom()+1, mBlockEdit->currentLine()+1);
        anchor = QPoint(mBlockEdit->colTo()+1, mBlockEdit->startLine()+1);
    } else {
        QTextCursor cursor = textCursor();
        pos = QPoint(cursor.positionInBlock()+1, cursor.blockNumber()+1);
        if (cursor.hasSelection()) {
            cursor.setPosition(cursor.anchor());
            anchor = QPoint(cursor.positionInBlock()+1, cursor.blockNumber()+1);
        }
    }
}

ParenthesesMatch CodeEditor::matchParentheses()
{
    static QString parentheses("{[(/}])\\");
    QTextBlock block = textCursor().block();
    if (!block.userData()) return ParenthesesMatch();
    QVector<ParenthesesPos> parList = static_cast<BlockData*>(block.userData())->parentheses();
    int pos = textCursor().positionInBlock();
    int start = -1;
    for (int i = parList.count()-1; i >= 0; --i) {
        if (parList.at(i).relPos == pos || parList.at(i).relPos == pos-1) {
            start = i;
            break;
        }
    }
    if (start < 0) return ParenthesesMatch();
    // prepare matching search
    int ci = parentheses.indexOf(parList.at(start).character);
    bool back = ci > 3;
    ci = ci % 4;
    bool inPar = back ^ (parList.at(start).relPos != pos);
    ParenthesesMatch result(block.position() + parList.at(start).relPos);
    QStringRef parEnter = parentheses.midRef(back ? 4 : 0, 4);
    QStringRef parLeave = parentheses.midRef(back ? 0 : 4, 4);
    QVector<QChar> parStack;
    parStack << parLeave.at(ci);
    int pi = start;
    while (block.isValid()) {
        // get next parentheses entry
        if (back ? --pi < 0 : ++pi >= parList.count()) {
            bool isEmpty = true;
            while (block.isValid() && isEmpty) {
                block = back ? block.previous() : block.next();
                if (block.isValid() && block.userData()) {
                    parList = static_cast<BlockData*>(block.userData())->parentheses();
                    if (!parList.isEmpty()) isEmpty = false;
                }
            }
            if (isEmpty) continue;
            parList = static_cast<BlockData*>(block.userData())->parentheses();
            pi = back ? parList.count()-1 : 0;
        }

        int i = parEnter.indexOf(parList.at(pi).character);
        if (i < 0) {
            // Only last stacked character is valid
            if (parList.at(pi).character == parStack.last()) {
                parStack.removeLast();
                if (parStack.isEmpty()) {
                    result.valid = true;
                    result.match = block.position() + parList.at(pi).relPos;
                    result.inOutMatch = result.match + (inPar^back ? 0 : 1);
                    return result;
                }
            } else {
                // Mark bad parentheses
                parStack.clear();
                result.match = block.position() + parList.at(pi).relPos;
                result.inOutMatch = result.match + (inPar^back ? 0 : 1);
                return result;
            }
        } else {
            // Stack new character
            parStack << parLeave.at(i);
        }

    }
    return ParenthesesMatch();
}

void CodeEditor::setOverwriteMode(bool overwrite)
{
    if (mBlockEdit) mBlockEdit->setOverwriteMode(overwrite);
    else AbstractEditor::setOverwriteMode(overwrite);
}

bool CodeEditor::overwriteMode() const
{
    if (mBlockEdit) return mBlockEdit->overwriteMode();
    return AbstractEditor::overwriteMode();
}

inline int CodeEditor::assignmentKind(int p)
{
    int preState = 0;
    int postState = 0;
    emit requestSyntaxState(p-1, preState);
    emit requestSyntaxState(p+1, postState);
    if (postState == (int)SyntaxState::IdentifierAssignment) return 1;
    if (preState == (int)SyntaxState::IdentifierAssignment) return -1;
    if (preState == (int)SyntaxState::IdentifierAssignmentEnd) return -1;
    return 0;
}

void CodeEditor::recalcExtraSelections()
{
    QList<QTextEdit::ExtraSelection> selections;
    mParenthesesMatch = ParenthesesMatch();
    if (!isReadOnly() && !mBlockEdit) {
        extraSelCurrentLine(selections);

        mWordUnderCursor = "";
        QTextEdit::ExtraSelection selection;
        selection.cursor = textCursor();
        QString text = selection.cursor.block().text();
        int start = qMin(selection.cursor.position(), selection.cursor.anchor()) - selection.cursor.block().position();
        int from = findAlphaNum(text, start, true);
        int to = findAlphaNum(text, from, false);
        if (from >= 0 && from <= to) {
            if (!textCursor().hasSelection() || text.mid(from, to-from+1) == textCursor().selectedText())
                mWordUnderCursor = text.mid(from, to-from+1);
        }
        mParenthesesDelay.start(100);
        mWordDelay.start(500);
    }
    extraSelBlockEdit(selections);
    setExtraSelections(selections);
}

void CodeEditor::updateExtraSelections()
{
    QList<QTextEdit::ExtraSelection> selections;
    extraSelCurrentLine(selections);
    if (!mBlockEdit) {
        if (!extraSelMatchParentheses(selections, sender() == &mParenthesesDelay) && sender() == &mWordDelay)
            extraSelCurrentWord(selections);
    }
    extraSelBlockEdit(selections);
    setExtraSelections(selections);
}

void CodeEditor::extraSelBlockEdit(QList<QTextEdit::ExtraSelection>& selections)
{
    if (mBlockEdit) {
        selections.append(mBlockEdit->extraSelections());
    }
}

void CodeEditor::extraSelCurrentLine(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!mSettings->highlightCurrentLine()) return;

    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(255, 250, 170); // TODO: read from style sheet
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.movePosition(QTextCursor::StartOfBlock);
    selection.cursor.clearSelection();
    selections.append(selection);
}

void CodeEditor::extraSelCurrentWord(QList<QTextEdit::ExtraSelection> &selections)
{
    if (!mSettings->wordUnderCursor()) return;

    QHash<int, TextMark*> textMarks;
    emit requestMarkHash(&textMarks, TextMark::match);

    if (textMarks.size() > 0) return;  // no word highlighting when a user searches

    if (!mWordUnderCursor.isEmpty()) {
        QTextBlock block = firstVisibleBlock();
        QRegularExpression rex(QString("(?i)(^|[^\\w]|-)(%1)($|[^\\w]|-)").arg(mWordUnderCursor));
        QRegularExpressionMatch match;
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        while (block.isValid() && top < viewport()->height()) {
            int i = 0;
            while (true) {
                i = block.text().indexOf(rex, i, &match);
                if (i < 0) break;
                QTextEdit::ExtraSelection selection;
                selection.cursor = textCursor();
                selection.cursor.setPosition(block.position()+match.capturedStart(2));
                selection.cursor.setPosition(block.position()+match.capturedEnd(2), QTextCursor::KeepAnchor);
//                QPen outlinePen( Qt::lightGray, 1);
//                selection.format.setProperty(QTextFormat::OutlinePen, outlinePen);
                QColor wordColor = QColor(Qt::lightGray)/*.lighter(115)*/;
                selection.format.setBackground(wordColor);
                selections << selection;
                i += match.capturedLength(1) + match.capturedLength(2);
            }
            top += qRound(blockBoundingRect(block).height());
            block = block.next();
        }
    }
}

bool CodeEditor::extraSelMatchParentheses(QList<QTextEdit::ExtraSelection> &selections, bool first)
{
    if (!mParenthesesMatch.isValid())
        mParenthesesMatch = matchParentheses();

    if (!mParenthesesMatch.isValid()) return false;

    if (mParenthesesMatch.pos == mParenthesesMatch.match) mParenthesesMatch.valid = false;
    QColor fgColor = mParenthesesMatch.valid ? QColor(Qt::red) : QColor(Qt::black);
    QColor bgColor = mParenthesesMatch.valid ? QColor(Qt::green).lighter(170) : QColor(Qt::red).lighter(150);
    QColor bgBlinkColor = mParenthesesMatch.valid ? QColor(Qt::green).lighter(130) : QColor(Qt::red).lighter(115);
    QTextEdit::ExtraSelection selection;
    selection.cursor = textCursor();
    selection.cursor.setPosition(mParenthesesMatch.pos);
    selection.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    selection.format.setForeground(fgColor);
    selection.format.setBackground(bgColor);
    selections << selection;
    if (mParenthesesMatch.match >= 0) {
        selection.cursor = textCursor();
        selection.cursor.setPosition(mParenthesesMatch.match);
        selection.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        selection.format.setForeground(fgColor);
        selection.format.setBackground(first ? bgBlinkColor : bgColor);
        selections << selection;
    }
    return true;
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(mLineNumberArea);
    QHash<int, TextMark*> textMarks;
    emit requestMarkHash(&textMarks, TextMark::all);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());
    int markFrom = textCursor().blockNumber();
    int markTo = textCursor().blockNumber();
    if (markFrom > markTo) qSwap(markFrom, markTo);

    QRect paintRect(event->rect());
    painter.fillRect(paintRect, QColor(245,245,245));

    QRect markRect(paintRect.left(), top, paintRect.width(), static_cast<int>(blockBoundingRect(block).height())+1);
    while (block.isValid() && top <= paintRect.bottom()) {
        if (block.isVisible() && bottom >= paintRect.top()) {
            bool mark = mBlockEdit ? mBlockEdit->hasBlock(block.blockNumber())
                                   : blockNumber >= markFrom && blockNumber <= markTo;
            if (mark) {
                markRect.moveTop(top);
                markRect.setHeight(bottom-top);
                painter.fillRect(markRect, QColor(225,255,235));
            }
            QString number = QString::number(blockNumber + 1);
            QFont f = font();
            f.setBold(mark);
            painter.setFont(f);
            painter.setPen(mark ? Qt::black : Qt::gray);
            int realtop = top; // (top+bottom-fontMetrics().height())/2;

            if(mSettings->showLineNr())
                painter.drawText(0, realtop, mLineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);

            if (textMarks.contains(blockNumber)) {
                int iTop = (2+top+bottom-iconSize())/2;
                painter.drawPixmap(1, iTop, textMarks.value(blockNumber)->icon().pixmap(QSize(iconSize(),iconSize())));
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

CodeEditor::BlockEdit::BlockEdit(CodeEditor* edit, int blockNr, int colNr)
    : mEdit(edit)
{
    if (!edit) FATAL() << "BlockEdit needs a valid editor";
    mStartLine = blockNr;
    mCurrentLine = blockNr;
    mColumn = colNr;
    mSize = 0;
}

CodeEditor::BlockEdit::~BlockEdit()
{
    mSelections.clear();
    mEdit->updateExtraSelections();
}

void CodeEditor::BlockEdit::selectTo(int blockNr, int colNr)
{
    mCurrentLine = blockNr;
    mSize = colNr - mColumn;
    updateExtraSelections();
    startCursorTimer();
}

void CodeEditor::BlockEdit::selectToEnd()
{
    int end = 0;
    for (QTextBlock block = mEdit->document()->findBlockByNumber(qMin(mStartLine, mCurrentLine))
         ; block.blockNumber() <= qMax(mStartLine, mCurrentLine); block=block.next()) {
        if (end < block.length()-1) end = block.length()-1;
    }
    mSize = end - mColumn;
}

QString CodeEditor::BlockEdit::blockText()
{
    QString res = "";
    if (!mSize) return res;
    QTextBlock block = mEdit->document()->findBlockByNumber(qMin(mStartLine, mCurrentLine));
    int from = qMin(mColumn, mColumn+mSize);
    int to = qMax(mColumn, mColumn+mSize);

    if (qMin(mStartLine, mCurrentLine) == qMax(mStartLine, mCurrentLine)) {
        QString text = block.text();
        if (text.length()-1 < to) text.append(QString(qAbs(mSize), ' '));
        res.append(text.mid(from, to-from));
    } else {
        for (int i = qMin(mStartLine, mCurrentLine); i <= qMax(mStartLine, mCurrentLine); ++i) {
            QString text = block.text();
            if (text.length()-1 < to) text.append(QString(qAbs(mSize), ' '));
            res.append(text.mid(from, to-from)+"\n");
            block = block.next();
        }
    }

    return res;
}

void CodeEditor::BlockEdit::selectionToClipboard()
{
    QMimeData *mime = new QMimeData();
    mime->setText(blockText());
    mime->setData("application/x-qt-windows-mime;value=\"MSDEVColumnSelect\"", QByteArray());
    mime->setData("application/x-qt-windows-mime;value=\"Borland IDE Block Type\"", QByteArray(1,char(2)));
    QApplication::clipboard()->setMimeData(mime);
}

int CodeEditor::BlockEdit::currentLine() const
{
    return mCurrentLine;
}

int CodeEditor::BlockEdit::column() const
{
    return mColumn;
}

void CodeEditor::BlockEdit::setColumn(int column)
{
    mColumn = column;
}

void CodeEditor::BlockEdit::setOverwriteMode(bool overwrite)
{
    mOverwrite = overwrite;
}

bool CodeEditor::BlockEdit::overwriteMode() const
{
    return mOverwrite;
}

int CodeEditor::BlockEdit::startLine() const
{
    return mStartLine;
}

void CodeEditor::BlockEdit::keyPressEvent(QKeyEvent* e)
{
    QSet<int> moveKeys;
    moveKeys << Qt::Key_Home << Qt::Key_End << Qt::Key_Down << Qt::Key_Up << Qt::Key_Left << Qt::Key_Right
             << Qt::Key_PageUp << Qt::Key_PageDown;
    if (moveKeys.contains(e->key())) {
        if (e->key() == Qt::Key_Down && mCurrentLine < mEdit->document()->blockCount()-1) mCurrentLine++;
        if (e->key() == Qt::Key_Up && mCurrentLine > 0) mCurrentLine--;
        if (e->key() == Qt::Key_Home) mSize = -mColumn;
        if (e->key() == Qt::Key_End) selectToEnd();
        QTextBlock block = mEdit->document()->findBlockByNumber(mCurrentLine);
        if ((e->modifiers()&Qt::ControlModifier) != 0 && e->key() == Qt::Key_Right) {
            nextCharClass(mColumn, mSize, block.text());
        } else if (e->key() == Qt::Key_Right) mSize++;
        if ((e->modifiers()&Qt::ControlModifier) != 0 && e->key() == Qt::Key_Left && mColumn+mSize > 0) {
            prevCharClass(mColumn, mSize, block.text());
        } else if (e->key() == Qt::Key_Left && mColumn+mSize > 0) mSize--;
        QTextCursor cursor(block);
        if (block.length() > mColumn+mSize)
            cursor.setPosition(block.position()+mColumn+mSize);
        else
            cursor.setPosition(block.position()+block.length()-1);
        mEdit->setTextCursor(cursor);
        updateExtraSelections();
        emit mEdit->cursorPositionChanged();
    } else if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
        if (!mSize && mColumn >= 0) mSize = (e->key() == Qt::Key_Backspace) ? -1 : 1;
        replaceBlockText("");
    } else if (e == Hotkey::Indent) {
        mEdit->indent(mEdit->mSettings->tabSize());
        return;
    } else if (e == Hotkey::Outdent) {
        mEdit->indent(-mEdit->mSettings->tabSize());
        return;
    } else if (e->text().length()) {
        replaceBlockText(e->text());
    }

    startCursorTimer();
}

void CodeEditor::BlockEdit::startCursorTimer()
{
    mEdit->mBlinkBlockEdit.start();
    mBlinkStateHidden = true;
    mEdit->lineNumberArea()->update(mEdit->lineNumberArea()->visibleRegion());
    refreshCursors();
}

void CodeEditor::BlockEdit::stopCursorTimer()
{
    mEdit->mBlinkBlockEdit.stop();
    mBlinkStateHidden = true;
    mEdit->lineNumberArea()->update(mEdit->lineNumberArea()->visibleRegion());
    refreshCursors();
}

void CodeEditor::BlockEdit::refreshCursors()
{
    // TODO(JM) generate drawCursor-event for every line
    mBlinkStateHidden = !mBlinkStateHidden;
    mEdit->viewport()->update(mEdit->viewport()->visibleRegion());
}

void CodeEditor::BlockEdit::paintEvent(QPaintEvent *e)
{
    QPainter painter(mEdit->viewport());
    QPointF offset(mEdit->contentOffset()); //
    QRect evRect = e->rect();
    bool editable = !mEdit->isReadOnly();
    painter.setClipRect(evRect);
    int cursorColumn = mColumn+mSize;
    QFontMetrics metric(mEdit->font());
    double spaceWidth = metric.width(QString(100,' ')) / 100.0;
    QTextBlock block = mEdit->firstVisibleBlock();
    QTextCursor cursor(block);
    cursor.setPosition(block.position()+block.length()-1);
//    qreal cursorOffset = 0; //mEdit->cursorRect(cursor).left()-block.layout()->minimumWidth();

    while (block.isValid()) {
        QRectF blockRect = mEdit->blockBoundingRect(block).translated(offset);
        if (!hasBlock(block.blockNumber()) || !block.isVisible()) {
            offset.ry() += blockRect.height();
            block = block.next();
            continue;
        }
        // draw extended extra-selection for lines past line-end
        int beyondEnd = qMax(mColumn, mColumn+mSize);
        if (beyondEnd >= block.length()) {
            cursor.setPosition(block.position()+block.length()-1);
            // we have to draw selection beyond the line-end
            int beyondStart = qMax(block.length()-1, qMin(mColumn, mColumn+mSize));
            QRectF selRect = mEdit->cursorRect(cursor);
//            qreal x = block.layout()->minimumWidth()+cursorOffset;
//            selRect.setLeft(x);
            if (block.length() <= beyondStart)
                selRect.translate(((beyondStart-block.length()+1) * spaceWidth), 0);
            selRect.setWidth((beyondEnd-beyondStart) * spaceWidth);
            QColor beColor = QColor(Qt::cyan).lighter(150);
            painter.fillRect(selRect, QBrush(beColor));
        }

        if (mBlinkStateHidden) {
            offset.ry() += blockRect.height();
            block = block.next();
            continue;
        }

        cursor.setPosition(block.position()+qMin(block.length()-1, cursorColumn));
        QRectF cursorRect = mEdit->cursorRect(cursor);
        if (block.length() <= cursorColumn) {
            cursorRect.translate((cursorColumn-block.length()+1) * spaceWidth, 0);
        }
        cursorRect.setWidth(mOverwrite ? spaceWidth : 2);

        if (cursorRect.bottom() >= evRect.top() && cursorRect.top() <= evRect.bottom()) {
            bool drawCursor = ((editable || (mEdit->textInteractionFlags() & Qt::TextSelectableByKeyboard)));
            if (drawCursor) {
                bool toggleAntialiasing = !(painter.renderHints() & QPainter::Antialiasing)
                                          && (painter.transform().type() > QTransform::TxTranslate);
                if (toggleAntialiasing)
                    painter.setRenderHint(QPainter::Antialiasing);
                QPainter::CompositionMode origCompositionMode = painter.compositionMode();
                if (painter.paintEngine()->hasFeature(QPaintEngine::RasterOpModes))
                    painter.setCompositionMode(QPainter::RasterOp_NotDestination);
                painter.fillRect(cursorRect, painter.pen().brush());
                painter.setCompositionMode(origCompositionMode);
                if (toggleAntialiasing)
                    painter.setRenderHint(QPainter::Antialiasing, false);

            }
        }

        offset.ry() += blockRect.height();
        if (offset.y() > mEdit->viewport()->rect().height())
            break;
        block = block.next();
    }

}

void CodeEditor::BlockEdit::updateExtraSelections()
{
    mSelections.clear();
    QTextCursor cursor(mEdit->document());
    for (int lineNr = qMin(mStartLine, mCurrentLine); lineNr <= qMax(mStartLine, mCurrentLine); ++lineNr) {
        QTextBlock block = mEdit->document()->findBlockByNumber(lineNr);
        QTextEdit::ExtraSelection select;
        // TODO(JM) Later, this color has to be retrieved from StyleSheet-definitions
        QColor beColor = QColor(Qt::cyan).lighter(150);
        select.format.setBackground(beColor);

        int start = qMin(block.length()-1, qMin(mColumn, mColumn+mSize));
        int end = qMin(block.length()-1, qMax(mColumn, mColumn+mSize));
        cursor.setPosition(block.position()+start);
        if (end>start) cursor.setPosition(block.position()+end, QTextCursor::KeepAnchor);
        select.cursor = cursor;
        mSelections << select;
        if (cursor.blockNumber() == mCurrentLine) {
            QTextCursor c = mEdit->textCursor();
            c.setPosition(cursor.position());
            mEdit->setTextCursor(c);
        }
    }
    mEdit->updateExtraSelections();
}

void CodeEditor::BlockEdit::adjustCursor()
{
    QTextBlock block = mEdit->document()->findBlockByNumber(mCurrentLine);
    QTextCursor cursor(block);
    cursor.setPosition(block.position() + qMin(block.length()-1, mColumn+mSize));
    mEdit->setTextCursor(cursor);
}

void CodeEditor::BlockEdit::replaceBlockText(QString text)
{
    replaceBlockText(QStringList() << text);
}

void CodeEditor::BlockEdit::replaceBlockText(QStringList texts)
{
    if (mEdit->isReadOnly()) return;
    if (texts.isEmpty()) texts << "";
    CharType charType = texts.at(0).length()>0 ? mEdit->charType(texts.at(0).at(0)) : CharType::None;
    bool newUndoBlock = texts.count()>1 || mLastCharType!=charType || texts.at(0).length()>1;
    // append empty lines if needed
    int missingLines = qMin(mStartLine, mCurrentLine) + texts.count() - mEdit->document()->lineCount();
    if (missingLines > 0) {
        QTextBlock block = mEdit->document()->lastBlock();
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::End);
        cursor.beginEditBlock();
        cursor.insertText(QString(missingLines, '\n'));
        cursor.endEditBlock();
        newUndoBlock = false;
    }
    if (qAbs(mStartLine-mCurrentLine) < texts.count() - 1) {
        if (mStartLine > mCurrentLine) mStartLine = mCurrentLine + texts.count() - 1;
        else mCurrentLine = mStartLine + texts.count() - 1;
    }

    int i = (qAbs(mStartLine-mCurrentLine)) % texts.count();
    QTextBlock block = mEdit->document()->findBlockByNumber(qMax(mCurrentLine, mStartLine));
    int fromCol = qMin(mColumn, mColumn+mSize);
    int toCol = qMax(mColumn, mColumn+mSize);
    if (fromCol == toCol && mOverwrite) ++toCol;
    QTextCursor cursor = mEdit->textCursor();
    int maxLen = 0;
    for (const QString &s: texts) {
        if (maxLen < s.length()) maxLen = s.length();
    }

    if (newUndoBlock) cursor.beginEditBlock();
    else cursor.joinPreviousEditBlock();

    QChar ch(' ');
    while (block.blockNumber() >= qMin(mCurrentLine, mStartLine)) {
//        if (ch=='.') ch='0'; else ch=',';
        QString addText = texts.at(i);
        if (maxLen && addText.length() < maxLen)
            addText += QString(maxLen-addText.length(), ch);
        int offsetFromEnd = fromCol - block.length()+1;
        if (offsetFromEnd > 0 && !texts.at(i).isEmpty()) {
            // line ends before start of mark -> calc additional spaces
            cursor.setPosition(block.position()+block.length()-1);
            QString s(offsetFromEnd, ch);
            addText = s + addText;
        } else if (fromCol != toCol) {
            // block-edit contains marking -> remove to end of block/line
            int pos = block.position()+fromCol;
            int rmSize = block.position()+qMin(block.length()-1, toCol) - pos;
            cursor.setPosition(pos);
            if (rmSize) {
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, rmSize);
                cursor.removeSelectedText();
            }
        } else {
            cursor.setPosition(block.position() + qMin(block.length()-1, fromCol));
        }
        if (!addText.isEmpty()) cursor.insertText(addText);
        block = block.previous();
        i = (i>0) ? i-1 : texts.count()-1;
    }
    cursor.endEditBlock();
    if (mSize < 0) mColumn += mSize;
    int insertWidth = -1;
    for (QString s: texts) {
        for (QStringRef ref: s.splitRef('\n')) {
            if (insertWidth < ref.length()) insertWidth = ref.length();
        }
    }
    mColumn += insertWidth;
    mSize = 0;
    mLastCharType = charType;
}

QChar BlockData::charForPos(int relPos)
{
    for (int i = mparentheses.count()-1; i >= 0; --i) {
        if (mparentheses.at(i).relPos == relPos || mparentheses.at(i).relPos-1 == relPos) {
            return mparentheses.at(i).character;
        }
    }
    return QChar();
}

QVector<ParenthesesPos> BlockData::parentheses() const
{
    return mparentheses;
}

void BlockData::setParentheses(const QVector<ParenthesesPos> &parentheses)
{
    mparentheses = parentheses;
}

void BlockData::addTextMark(TextMark *mark)
{
    if (mMarks.contains(mark)) return;
    mMarks << mark;
    mark->setBlockData(this);
}

void BlockData::removeTextMark(TextMark *mark)
{
    mMarks.removeAll(mark);
}


} // namespace studio
} // namespace gams
