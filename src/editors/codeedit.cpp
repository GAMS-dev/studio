/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include <QPalette>
#include <QDir>

#include "editors/codeedit.h"
#include "settings.h"
#include "search/searchdialog.h"
#include "logger.h"
#include "syntax.h"
#include "keys.h"
#include "theme.h"
#include "editorhelper.h"
#include "viewhelper.h"
#include "search/searchlocator.h"
#include "editors/navigationhistory.h"
#include "editors/navigationhistorylocator.h"

namespace gams {
namespace studio {

inline const KeySeqList &hotkey(Hotkey _hotkey) { return Keys::instance().keySequence(_hotkey); }

CodeEdit::CodeEdit(QWidget *parent)
    : AbstractEdit(parent)
{
    mLineNumberArea = new LineNumberArea(this);
    mLineNumberArea->setMouseTracking(true);
    mBlinkBlockEdit.setInterval(500);
    mWordDelay.setSingleShot(true);
    mParenthesesDelay.setSingleShot(true);
    mSettings = Settings::settings();

    connect(&mBlinkBlockEdit, &QTimer::timeout, this, &CodeEdit::blockEditBlink);
    connect(&mWordDelay, &QTimer::timeout, this, &CodeEdit::updateExtraSelections);
    connect(&mParenthesesDelay, &QTimer::timeout, this, &CodeEdit::updateExtraSelections);
    connect(this, &CodeEdit::blockCountChanged, this, &CodeEdit::blockCountHasChanged);
    connect(this, &CodeEdit::updateRequest, this, &CodeEdit::updateLineNumberArea);
    connect(this, &CodeEdit::cursorPositionChanged, this, &CodeEdit::recalcExtraSelections);
    connect(this, &CodeEdit::textChanged, this, &CodeEdit::recalcExtraSelections);
    connect(this->verticalScrollBar(), &QScrollBar::actionTriggered, this, &CodeEdit::updateExtraSelections);
    connect(document(), &QTextDocument::undoCommandAdded, this, &CodeEdit::undoCommandAdded);

    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    QTimer::singleShot(0, [this](){ setCursorWidth(2); });
}

CodeEdit::~CodeEdit()
{
    while (mBlockEditPos.size())
        delete mBlockEditPos.takeLast();
}

int CodeEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 0;

    if (showLineNr()) {
        space = fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
        if (showFolding())
            space += iconSize();
    }

    if (marks() && marks()->hasVisibleMarks()) {
        space += iconSize();
        mIconCols = 1;
    } else {
        mIconCols = 0;
    }

    if (space) space += 3; // add margin if visible

    return space;
}

int CodeEdit::iconSize()
{
    return fontMetrics().height()-3;
}

LineNumberArea* CodeEdit::lineNumberArea()
{
    return mLineNumberArea;
}

void CodeEdit::updateLineNumberAreaWidth()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    if (viewportMargins().left() != mLnAreaWidth) {
        mLineNumberArea->repaint();
        mLnAreaWidth = viewportMargins().left();
    }
}

void CodeEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        mLineNumberArea->scroll(0, dy);
    } else {
        int top = rect.y();
        int bottom = top + rect.height();
        // NOTE! major performance issue on calling :blockBoundingGeometry()
        mLineNumberArea->update(0, top, mLineNumberArea->width(), bottom-top);
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void CodeEdit::blockEditBlink()
{
    if (mBlockEdit) mBlockEdit->refreshCursors();
}

void CodeEdit::checkBlockInsertion()
{
    bool extraJoin = mBlockEditInsText.isNull();
    QTextCursor cur = textCursor();
    bool validText = (mBlockEditRealPos != cur.position());
    if (validText) {
        cur.setPosition(mBlockEditRealPos, QTextCursor::KeepAnchor);
        mBlockEditInsText = cur.selectedText();
        cur.removeSelectedText();
        if (!extraJoin) cur.endEditBlock();
        mBlockEdit->replaceBlockText(mBlockEditInsText);
        mBlockEditInsText = "";
    }
    if (!validText || extraJoin) {
        cur.endEditBlock();
    }
    mBlockEditRealPos = -1;
}

void CodeEdit::undoCommandAdded()
{
    while (document()->availableUndoSteps()-1 < mBlockEditPos.size())
        delete mBlockEditPos.takeLast();
    while (document()->availableUndoSteps() > mBlockEditPos.size()) {
        BlockEditPos *bPos = nullptr;
        if (mBlockEdit) bPos = new BlockEditPos(mBlockEdit->startLine(), mBlockEdit->currentLine(), mBlockEdit->column());
        mBlockEditPos.append(bPos);
    }
}

void CodeEdit::switchCurrentFolding()
{
    toggleFolding(textCursor().block());
}

void CodeEdit::extendedRedo()
{
    if (mBlockEdit) endBlockEdit();
    redo();
    updateBlockEditPos();
}

void CodeEdit::extendedUndo()
{
    if (mBlockEdit) endBlockEdit();
    undo();
    updateBlockEditPos();
}

void CodeEdit::convertToLower()
{
    if (isReadOnly()) return;

    if (mBlockEdit) {
        QStringList lowerLines = mBlockEdit->blockText().toLower()
                                 .split("\n", Qt::SkipEmptyParts);
        mBlockEdit->replaceBlockText(lowerLines);
    } else {
        QTextCursor cursor = textCursor();
        QPair<int,int> sel(cursor.anchor(), cursor.position());
        textCursor().insertText(cursor.selectedText().toLower());
        cursor.setPosition(sel.first);
        cursor.setPosition(sel.second, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
}

void CodeEdit::convertToUpper()
{
    if (isReadOnly()) return;
    if (mBlockEdit) {
        QStringList lowerLines = mBlockEdit->blockText().toUpper()
                                 .split("\n", Qt::SkipEmptyParts);
        mBlockEdit->replaceBlockText(lowerLines);
    } else {
        QTextCursor cursor = textCursor();
        QPair<int,int> sel(cursor.anchor(), cursor.position());
        textCursor().insertText(cursor.selectedText().toUpper());
        cursor.setPosition(sel.first);
        cursor.setPosition(sel.second, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
}

void CodeEdit::updateBlockEditPos()
{
    if (document()->availableUndoSteps() <= 0 || document()->availableUndoSteps() > mBlockEditPos.size())
        return;
//    debugUndoStack(mBlockEditPos, document()->availableUndoSteps()-1);
    BlockEditPos * bPos = mBlockEditPos.at(document()->availableUndoSteps()-1);
    if (mBlockEdit) endBlockEdit();
    if (bPos && !mBlockEdit && mAllowBlockEdit) {
        startBlockEdit(bPos->startLine, bPos->column);
        mBlockEdit->selectTo(bPos->currentLine, bPos->column);
    }
}

QString CodeEdit::wordUnderCursor() const
{
    return mWordUnderCursor;
}

bool CodeEdit::hasSelection() const
{
    return textCursor().hasSelection();
}

void CodeEdit::disconnectTimers()
{
    AbstractEdit::disconnectTimers();
    disconnect(&mWordDelay, &QTimer::timeout, this, &CodeEdit::updateExtraSelections);
    disconnect(&mParenthesesDelay, &QTimer::timeout, this, &CodeEdit::updateExtraSelections);
}

void CodeEdit::clearSelection()
{
    if (isReadOnly()) return;
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->replaceBlockText(QStringList()<<QString());
    } else {
        textCursor().clearSelection();
    }
}

void CodeEdit::cutSelection()
{
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->selectionToClipboard();
        mBlockEdit->replaceBlockText(QStringList()<<QString());
    } else {
        cut();
    }
}

void CodeEdit::copySelection()
{
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->selectionToClipboard();
    } else {
        copy();
    }
}

void CodeEdit::selectAllText()
{
    selectAll();
}

void CodeEdit::pasteClipboard()
{
    bool isBlock;
    QStringList texts = clipboard(&isBlock);
    if (!mBlockEdit) {
        if (isBlock && mAllowBlockEdit) {
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

void CodeEdit::resizeEvent(QResizeEvent *e)
{
    AbstractEdit::resizeEvent(e);

    QRect cr = contentsRect();
    mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    updateLineNumberAreaWidth();
    updateExtraSelections();
}

void CodeEdit::keyPressEvent(QKeyEvent* e)
{
    int vertScroll = verticalScrollBar()->sliderPosition();
    int topBlock = firstVisibleBlock().blockNumber();
    if (!mBlockEdit && mAllowBlockEdit && e == Hotkey::BlockEditStart) {
        QTextCursor c = textCursor();
        QTextCursor anc = c;
        anc.setPosition(c.anchor());
        startBlockEdit(anc.blockNumber(), anc.columnNumber());
    }
    e->ignore();
    if (mBlockEdit) {
        if (e == Hotkey::SelectAll || e == Hotkey::ToggleBlockFolding) {
            endBlockEdit();
        } else if (e->key() == Hotkey::NewLine || e == Hotkey::BlockEditEnd) {
            endBlockEdit();
            return;
        } else {
            mBlockEdit->keyPressEvent(e);
            return;
        }
    }
    QTextCursor cur = textCursor();
    QTextCursor::MoveMode mm = (e->modifiers() & Qt::ShiftModifier) ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
    if (e == Hotkey::MatchParentheses || e == Hotkey::SelectParentheses) {
        PositionPair pm = matchParentheses(textCursor());
        bool sel = (e == Hotkey::SelectParentheses);
        if (pm.match >= 0) {
            if (sel) cur.clearSelection();
            if (cur.position() != pm.pos) cur.movePosition(QTextCursor::Left);
            cur.setPosition(pm.match+1, mm);
            e->accept();
        }
    } else if (e == Hotkey::MoveToEndOfLine) {
        cur.movePosition(QTextCursor::EndOfLine, mm);
        e->accept();
    } else if (e == Hotkey::MoveToStartOfLine) {
        QTextBlock block = cur.block();
        QRegularExpression leadingSpaces("^(\\s*)");
        QRegularExpressionMatch lsMatch = leadingSpaces.match(block.text());

        if (cur.positionInBlock()==0 || lsMatch.capturedLength(1) < cur.positionInBlock() || vertScroll != verticalScrollBar()->sliderPosition())
            cur.setPosition(block.position() + lsMatch.capturedLength(1), mm);
        else cur.setPosition(block.position(), mm);
        e->accept();
    } else if (e == Hotkey::MoveCharGroupRight) {
        int p = cur.positionInBlock();
        EditorHelper::nextWord(0, p, cur.block().text());
        if (p >= cur.block().length()) {
            QTextBlock block = cur.block().next();
            if (block.isValid()) cur.setPosition(block.position(), mm);
            else cur.movePosition(QTextCursor::EndOfBlock, mm);
        } else {
            cur.setPosition(cur.block().position() + p, mm);
        }
        e->accept();
    } else if (e == Hotkey::MoveCharGroupLeft) {
        int p = cur.positionInBlock();
        if (p == 0) {
            QTextBlock block = cur.block().previous();
            if (block.isValid()) cur.setPosition(block.position()+block.length()-1, mm);
        } else {
            EditorHelper::prevWord(0, p, cur.block().text());
            cur.setPosition(cur.block().position() + p, mm);
        }
        e->accept();
    }
    if (e->isAccepted()) {
        setTextCursor(cur);
        if (e->key() == Qt::Key_Control || topBlock != firstVisibleBlock().blockNumber()) {
            QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
            QMouseEvent me(QEvent::MouseMove, mousePos, Qt::NoButton, qApp->mouseButtons(), e->modifiers());
            emit mouseMoveEvent(&me);
        }
        return;
    }

    if (!isReadOnly()) {
        if (e->key() == Hotkey::JumpToContext) {
            jumpToCurrentLink(cursorRect().topLeft());
            e->accept();
            return;
        }
        if (e->key() == Hotkey::NewLine) {
            QTextCursor cursor = textCursor();
            int pos = cursor.positionInBlock();
            cursor.beginEditBlock();
            QString leadingText = cursor.block().text().left(pos).trimmed();
            cursor.insertText("\n");
            if (!leadingText.isEmpty()) {
                if (cursor.block().previous().isValid())
                    truncate(cursor.block().previous());
                adjustIndent(cursor);
            }
            cursor.endEditBlock();
            setTextCursor(cursor);
            e->accept();
            return;
        }
        if (e == Hotkey::ToggleBlockFolding) {
            QTextBlock block = textCursor().block();
            toggleFolding(block);
            return;
        }
        if (e == Hotkey::Indent) {
            indent(mSettings->toInt(skEdTabSize));
            e->accept();
            return;
        }
        if (e == Hotkey::Outdent) {
            indent(-mSettings->toInt(skEdTabSize));
            e->accept();
            return;
        }
        if (mSettings->toBool(skEdAutoIndent) && e->key() == Qt::Key_Backspace) {
            int pos = textCursor().positionInBlock();

            QString line = textCursor().block().text();
            QRegularExpression regex("^(\\s+)");
            QRegularExpressionMatch match = regex.match(line);
            bool allWhitespace = match.hasMatch();

            if (allWhitespace && !textCursor().hasSelection() && match.capturedLength() == pos) {
                indent(-mSettings->toInt(skEdTabSize));
                e->accept();
                return;
            }
        }
    }

    if (e == Hotkey::SearchFindPrev)
        emit searchFindPrevPressed();
    else if (e == Hotkey::SearchFindNext)
        emit searchFindNextPressed();

    // smart typing:
    if (Settings::settings()->toBool(skEdAutoCloseBraces) && !isReadOnly())  {
        QSet<int> moveKeys;
        moveKeys << Qt::Key_Home << Qt::Key_End << Qt::Key_Down << Qt::Key_Up
                 << Qt::Key_Left << Qt::Key_Right << Qt::Key_PageUp << Qt::Key_PageDown;
        // deactivate when manual cursor movement was detected
        if (moveKeys.contains(e->key())) mSmartType = false;

        int indexOpening = mOpening.indexOf(e->text());
        int indexClosing = mClosing.indexOf(e->text());

        // exclude modifier combinations
        if (e->text().isEmpty()) {
            indexOpening = -1;
            indexClosing = -1;
        }

        // surround selected text with characters
        if ((indexOpening != -1) && (textCursor().hasSelection())) {
            QTextCursor tc(textCursor());
            QString selection(tc.selectedText());
            selection = mOpening.at(indexOpening) + selection + mClosing.at(indexOpening);
            tc.insertText(selection);
            setTextCursor(tc);
            return;

        // jump over closing character if already in place
        } else if (mSmartType && indexClosing != -1 &&
                   mClosing.indexOf(document()->characterAt(textCursor().position())) == indexClosing) {
            QTextCursor tc = textCursor();
            tc.movePosition(QTextCursor::NextCharacter);
            setTextCursor(tc);
            e->accept();
            return;

        // insert closing characters
        } else if (indexOpening != -1 && allowClosing(indexOpening)) {
            mSmartType = true;
            QTextCursor tc = textCursor();
            tc.insertText(e->text());
            tc.insertText(mClosing.at(indexOpening));
            tc.movePosition(QTextCursor::PreviousCharacter);
            setTextCursor(tc);
            e->accept();
            return;
        }

        if (mSmartType && e->key() == Qt::Key_Backspace) {

            QChar a = document()->characterAt(textCursor().position()-1);
            QChar b = document()->characterAt(textCursor().position());

            // delete auto insertable char
            if (mOpening.indexOf(a) != -1 && (mOpening.indexOf(a) ==  mClosing.indexOf(b))) {
                textCursor().deleteChar();
                textCursor().deletePreviousChar();
                e->accept();

                // check cursor surrounding characters again
                a = document()->characterAt(textCursor().position()-1);
                b = document()->characterAt(textCursor().position());

                // keep smarttype on conditionally; e.g. for ((|)); qt creator does this, too
                mSmartType = (mOpening.indexOf(a) == mClosing.indexOf(b));
                return;
            }
        }
    }
    AbstractEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_Control || topBlock != firstVisibleBlock().blockNumber() || vertScroll != verticalScrollBar()->sliderPosition()) {
        QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
        QMouseEvent me(QEvent::MouseMove, mousePos, Qt::NoButton, qApp->mouseButtons(), e->modifiers());
        emit mouseMoveEvent(&me);
    }
}

bool CodeEdit::allowClosing(int chIndex)
{
    const QString allowingChars(",;)}] \x9");

    // if next character is in the list of allowing characters
    bool nextAllows = allowingChars.indexOf(document()->characterAt(textCursor().position())) != -1
        // AND next character is not closing partner of current
            && mClosing.indexOf(document()->characterAt(textCursor().position())) != chIndex;

    bool nextIsLinebreak = textCursor().positionInBlock() == textCursor().block().length()-1;
    // if char before and after the cursor are a matching pair: allow anyways
    QChar prior = document()->characterAt(textCursor().position() - 1);

    bool matchingPairExists = (mOpening.indexOf(prior) != -1) && ( mOpening.indexOf(prior) == mClosing.indexOf(document()->characterAt(textCursor().position())) );

    // insert closing if next char permits or is end of line
    bool allowAutoClose =  nextAllows || nextIsLinebreak || matchingPairExists;

    // next is allowed char && if brackets are there and matching && no quotes after letters or numbers
    return allowAutoClose && (!prior.isLetterOrNumber() || chIndex < 3);
}

bool CodeEdit::toggleFolding(QTextBlock block)
{
    bool folded;
    block = findFoldStart(block);
    if (!block.isValid()) return false;
    int foldPos = foldStart(block.blockNumber(), folded);
    if (foldPos < 0) return false;
    QTextCursor cursor(block);
    cursor.setPosition(cursor.position() + foldPos+1);
    int foldCount = 0;
    PositionPair pp = matchParentheses(cursor, true, &foldCount);
    if (!pp.isNull() && pp.valid && block.userData()) {
        syntax::BlockData *startDat = syntax::BlockData::fromTextBlock(block);
        int foldSkip = 0;
        startDat->setFoldCount(folded ? 0 : foldCount);
        while (foldCount--) {
            block = block.next();
            if (!block.isValid()) break;
            if (foldSkip) {
                --foldSkip;
            } else {
                block.setVisible(folded);
                syntax::BlockData *dat = syntax::BlockData::fromTextBlock(block);
                if (dat && dat->isFolded()) foldSkip = dat->foldCount();
            }
        }
    }
    checkCursorAfterFolding();
    document()->adjustSize();
    viewport()->repaint();
    mLineNumberArea->repaint();
    return true;
}

void CodeEdit::foldAll()
{
    if (mBlockEdit) endBlockEdit();
    // TODO(JM) the current implementation could be improved for nested blocks
//    QList<BlockData*> stack;

    int foldRemain = 0;
    QTextBlock block = document()->firstBlock();
    while (block.isValid()) {
        if (foldRemain-- > 0) block.setVisible(false);
        bool folded;
        int foldPos = foldStart(block.blockNumber(), folded);
        if (foldPos >= 0) {
//            stack << static_cast<BlockData*>(block.userData());
            QTextCursor cursor(block);
            cursor.setPosition(cursor.position() + foldPos+1);
            int foldCount;
            PositionPair pp = matchParentheses(cursor, true, &foldCount);
            syntax::BlockData *dat = syntax::BlockData::fromTextBlock(block);
            if (!pp.isNull() && pp.valid && dat) {
                dat->setFoldCount(foldCount);
                if (foldRemain < foldCount) foldRemain = foldCount;
            }
        }
        block = block.next();
    }
    checkCursorAfterFolding();
    mFoldMark = LinePair();
    document()->adjustSize();
    viewport()->repaint();
    mLineNumberArea->repaint();
}

void CodeEdit::unfoldAll()
{
    QTextBlock block = document()->firstBlock();
    while (block.isValid()) {
        if (!block.isVisible()) block.setVisible(true);
        syntax::BlockData *dat = syntax::BlockData::fromTextBlock(block);
        if (dat) dat->setFoldCount(0);
        block = block.next();
    }
    mFoldMark = LinePair();
    document()->adjustSize();
    viewport()->repaint();
    mLineNumberArea->repaint();
}

LinePair CodeEdit::findFoldBlock(int line, bool onlyThisLine) const
{
    LinePair res;
    if (!showFolding()) return res;

    // find open marker
    QTextBlock block = document()->findBlockByNumber(line);
    while (block.isValid()) {
        bool folded;
        int foldPos = foldStart(block.blockNumber(), folded);
        if (foldPos >= 0) {
            QTextCursor cursor(block);
            cursor.setPosition(cursor.position() + foldPos);
            PositionPair pp = matchParentheses(cursor, true);
            if (!pp.isNull()) {
                res.pos = block.blockNumber();
                res.match = document()->findBlock(pp.match).blockNumber();
                res.valid = pp.valid;
                break;
            }
        }
        if (onlyThisLine) break;
        block = block.previous();
    }
    return res;
}

QTextBlock CodeEdit::findFoldStart(QTextBlock block) const
{
    int count = 0;
    int depth = 0;
    syntax::BlockData *dat = syntax::BlockData::fromTextBlock(block);
    if (dat) {
        if (dat->nestingImpact().rightOpen()) return block;
        if (dat->nestingImpact().leftOpen())
            depth = dat->nestingImpact().leftOpen() + 1;
    }
    while (block.isValid() && count < 1000) {
        block = block.previous();
        ++count;
        syntax::BlockData *dat = syntax::BlockData::fromTextBlock(block);
        if (dat) depth += dat->nestingImpact().rightOpen();
        if (depth > 0) return block;
        if (dat) depth += dat->nestingImpact().leftOpen();
    }
    return QTextBlock();
}

bool CodeEdit::unfoldBadBlock(QTextBlock block)
{
    if (!block.isVisible()) return false;
    block = block.next();
    int skip = 0;
    while (block.isValid() && !block.isVisible()) {
        if (!skip)
            block.setVisible(true);
        else
            --skip;
        syntax::BlockData *dat = syntax::BlockData::fromTextBlock(block);
        if (dat) skip = dat->foldCount();
        block = block.next();
    }
    return true;
}

void CodeEdit::checkCursorAfterFolding()
{
    if (!textCursor().block().isVisible()) {
        QTextBlock block = textCursor().block();
        while (block.isValid() && !block.isVisible())
            block = block.previous();
        QTextCursor cur = textCursor();
        cur.setPosition(block.position() + qMin(block.length()-1, cur.positionInBlock()));
        setTextCursor(cur);
    } else if (hasSelection() && !document()->findBlock(textCursor().anchor()).isVisible()) {
        QTextCursor cur = textCursor();
        cur.clearSelection();
        setTextCursor(cur);
    }
}

bool CodeEdit::ensureUnfolded(int line)
{
    int lastUnfoldedNr = -1;
    QTextBlock block = document()->findBlockByNumber(line);
    if (block.isValid() && block.isVisible()) {
        block = block.next();
        ++line;
    }
    // repeatly check the block of the line number until it is visible
    while (block.isValid() && !block.isVisible()) {
        // find the first visible previous line, that's the fold-head
        while (block.isValid() && !block.isVisible())
            block = block.previous();
        if (block.blockNumber() != line) {
            bool ok = toggleFolding(block);
            if (!ok)
                return unfoldBadBlock(block);
            if (block.blockNumber() == lastUnfoldedNr) {
                DEB() << "ERROR on unfolding line " << QString::number(lastUnfoldedNr);
                break;
            }
            lastUnfoldedNr = block.blockNumber();
        }
        block = document()->findBlockByNumber(line);
    }
    return  lastUnfoldedNr >= 0;
}

QString CodeEdit::resolveHRef(QString href)
{
    QString fileName;
    emit hasHRef(href, fileName);
    return fileName;
}

QString CodeEdit::getIncludeFile(int line, int &fileStart, QString &code)
{
    QString res;
    code = "INC";
    QTextBlock block = document()->findBlockByNumber(line);
    fileStart = block.length();
    if (block.isValid()) {
        QRegularExpression rex;
        rex.setPattern(QString("(^%1|%1%1)\\s*([\\w]+)\\s*").arg(QRegularExpression::escape("$")));
        QRegularExpressionMatch match = rex.match(block.text());
        if (match.captured(2).length() < block.length()) {
            QChar endChar(' ');
            QString command = match.captured(2).toUpper();
            if (command == "INCLUDE") {
                fileStart = match.capturedEnd();
                int fileEnd = block.text().length();
                while (fileEnd >= fileStart) {
                    int kind;
                    emit requestSyntaxKind(block.position() + fileEnd, kind);
                    if (kind == int(syntax::SyntaxKind::DirectiveBody)) break;
                    --fileEnd;
                }
                endChar = QChar();
                res = block.text().mid(fileStart, fileEnd-fileStart).trimmed();
            } else if (command.endsWith("INCLUDE")) { // batInclude, sysInclude, libInclude
                if (command.at(0) != 'B') code = command.left(3);
                fileStart = match.capturedEnd();
                res = block.text().mid(fileStart, block.text().length()).trimmed();
            }
            if (!res.isEmpty()) {
                if (block.text().at(fileStart) == '\"') endChar = '\"';
                if (block.text().at(fileStart) == '\'') endChar = '\'';
                if (endChar != QChar()) {
                    int w = (endChar==' ') ? 0 : 1;
                    int end = res.indexOf(endChar, 1) + w;
                    if (end) {
                        res = res.mid(w, end - 2*w);
                        fileStart += w;
                    }
                }
            }
        }
    }
//    DEB() << "FILE: " << res;
    return res;
}

TextLinkType CodeEdit::checkLinks(const QPoint &mousePos, bool greedy, QString *fName)
{
    // greedy extends the scope (e.g. when control modifier is pressed)
    TextLinkType linkType = linkNone;
    if (!showFolding() || mousePos.x() >= 0 || mousePos.x() < -iconSize())
        linkType = AbstractEdit::checkLinks(mousePos, greedy, fName);
    if (greedy && linkType == linkNone) {
        QTextCursor cur = cursorForPosition(mousePos); //cursorForPositionCut(mousePos);
        if (cur.isNull()) return linkType;
        int fileStart;
        QString command;
        QString file = getIncludeFile(cur.blockNumber(), fileStart, command);
        int boundPos = qBound(fileStart, cur.positionInBlock(), fileStart+file.length()-1);
        if (!file.isEmpty() && cur.positionInBlock() == boundPos) {
            QString fileName = resolveHRef(command+" "+file);
            linkType = fileName.isEmpty() ? linkMiss : linkDirect;
            if (fName) *fName = fileName;
        }
    }
    return linkType;

}

void CodeEdit::jumpToCurrentLink(const QPoint &mousePos)
{
    QTextCursor cur = textCursor();
    TextLinkType linkType = checkLinks(mousePos, true);
    if (linkType == linkMark) {
        if (!marks() || marks()->isEmpty()) {
            // no regular marks, check for temporary hrefs
            QTextCursor cursor = cursorForPosition(mousePos);
            if (resolveHRef(cursor.charFormat().anchorHref()).isEmpty()) return;
            cur.clearSelection();
            setTextCursor(cur);
            emit jumpToHRef(cursor.charFormat().anchorHref());
        } else {
            cur.clearSelection();
            setTextCursor(cur);
            marksAtMouse().first()->jumpToRefMark();
        }
    }
    if (linkType == linkDirect) {
        QTextCursor cur = cursorForPosition(mousePos);
        int fileStart;
        QString command;
        QString file = getIncludeFile(cur.blockNumber(), fileStart, command);
        int boundPos = qBound(fileStart, cur.positionInBlock(), fileStart+file.length()-1);
        if (!file.isEmpty() && cur.positionInBlock() == boundPos) {
            mIncludeLinkLine = cursorForPosition(mousePos).blockNumber();
            cur.clearSelection();
            setTextCursor(cur);
            emit jumpToHRef(command+" "+file);
        }
    }
}

void CodeEdit::keyReleaseEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Control) {
        QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
        QMouseEvent me(QEvent::MouseMove, mousePos, Qt::NoButton, qApp->mouseButtons(), e->modifiers());
        emit mouseMoveEvent(&me);
    }
    // return pressed: ignore here
    if (!isReadOnly() && e->key() == Hotkey::NewLine) {
        e->accept();
        return;
    }
    AbstractEdit::keyReleaseEvent(e);
}

void CodeEdit::adjustIndent(QTextCursor cursor)
{
    if (!mSettings->toBool(skEdAutoIndent)) return;

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
                cursor.setPosition(cursor.position() + match.capturedLength(1), QTextCursor::KeepAnchor);
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


void CodeEdit::truncate(QTextBlock block)
{
    QRegularExpression pRex("(^.*[^\\s]|^)(\\s+)$");
    QRegularExpressionMatch match = pRex.match(block.text());
    if (match.hasMatch()) {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.setPosition(cursor.position() - match.capturedLength(2), QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }
}

int CodeEdit::textCursorColumn(QPoint mousePos)
{
    QTextCursor cursor = cursorForPosition(mousePos);
    int col = cursor.columnNumber();
    int addX = mousePos.x()-cursorRect(cursor).right();
    if (addX > 0) {
        QFontMetrics metric(font());
        col += addX / metric.horizontalAdvance(' ');
    }
    return col;
}

void CodeEdit::mousePressEvent(QMouseEvent* e)
{
    bool done = false;
    mSmartType = false; // exit on mouse navigation
    setContextMenuPolicy(Qt::DefaultContextMenu);
    if (e->modifiers() == (Qt::AltModifier | Qt::ShiftModifier) && mAllowBlockEdit)
        setContextMenuPolicy(Qt::PreventContextMenu);
    if (e->modifiers() & Qt::AltModifier && mAllowBlockEdit) {
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
        done = true;
    } else if (mBlockEdit) {
        if (e->modifiers() || e->buttons() != Qt::RightButton) {
            endBlockEdit(false);
        } else if (e->button() == Qt::RightButton) {
            QTextCursor mouseTC = cursorForPosition(e->pos());
            if (mouseTC.blockNumber() < qMin(mBlockEdit->startLine(), mBlockEdit->currentLine())
                    || mouseTC.blockNumber() > qMax(mBlockEdit->startLine(), mBlockEdit->currentLine())) {
                endBlockEdit(false);
                setTextCursor(mouseTC);
            }
            done = true;
        }
    }
    if (!done) {
        AbstractEdit::mousePressEvent(e);
    }
}

void CodeEdit::mouseMoveEvent(QMouseEvent* e)
{
    NavigationHistoryLocator::navigationHistory()->stopRecord();

    if (mBlockEdit) {
        if ((e->buttons() & Qt::LeftButton) && (e->modifiers() & Qt::AltModifier)) {
            mBlockEdit->selectTo(cursorForPosition(e->pos()).blockNumber(), textCursorColumn(e->pos()));
        }
    } else {
        AbstractEdit::mouseMoveEvent(e);
    }
    bool direct = e->modifiers() & Qt::ControlModifier || e->pos().x() < 0
            || (type() != CodeEditor && type() != LstView);
    QPoint pos = e->buttons()==Qt::NoButton ? e->pos() : QPoint();
    updateToolTip(pos, direct);
    updateLinkAppearance(e->pos(), e->modifiers() & Qt::ControlModifier);
    lineNumberArea()->setCursor(viewport()->cursor().shape());

    NavigationHistoryLocator::navigationHistory()->startRecord();
}

void CodeEdit::wheelEvent(QWheelEvent *e) {
    if (e->modifiers() & Qt::ControlModifier) {
        const int delta = e->angleDelta().y();
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
    AbstractEdit::wheelEvent(e);
}

void CodeEdit::paintEvent(QPaintEvent* e)
{
    AbstractEdit::paintEvent(e);
    if (mBlockEdit) {
        mBlockEdit->paintEvent(e);
    }
    if (showFolding()) {
        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        QRectF fRect = blockBoundingGeometry(block).translated(contentOffset());
        int top = static_cast<int>(fRect.top());
        int bottom = top + static_cast<int>(fRect.height());
        QPainter painter(viewport());
        QRect paintRect(e->rect());
        while (block.isValid() && top <= paintRect.bottom()) {
            if (block.isVisible() && bottom >= paintRect.top()) {
                int foldPos = 0;
                bool folded = false;
                QString closingSymbol;
                int foldRes = foldState(blockNumber, folded, &foldPos, &closingSymbol);
                if (foldRes % 3 == 2) {
                    QString text = QString("... ") + closingSymbol;
                    // calculate rect
                    QTextLine lin = block.layout()->lineAt(block.lineCount()-1);
                    QRectF r = lin.naturalTextRect();
                    r.moveTop(r.top() + top);
                    r.moveLeft(r.left() + r.width() + r.height()/2);
                    QRectF tRect = painter.boundingRect(r, Qt::AlignLeft | Qt::AlignVCenter, text);
                    r.setWidth(tRect.width()+2);

                    // draw the additional line for folded blocks
                    QRect foldRect(0, bottom-1, int(r.left()+r.height()), 1);
                    painter.fillRect(foldRect, toColor(Theme::Edit_foldLineBg));

                    // draw folding block
                    qreal rad = r.height() / 3;
                    painter.setPen(toColor(Theme::Edit_foldLineBg));
                    painter.setBrush(toColor(Theme::Edit_foldLineBg));
                    painter.drawRoundedRect(r, rad, rad);
                    QFont f = painter.font();
                    f.setBold(true);
                    painter.setFont(f);
                    r.setHeight(r.height()-1);
                    painter.setPen(toColor(Theme::Edit_foldLineFg));
                    painter.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, text);
                }
            }
            block = block.next();
            top = bottom;
            bottom = top + static_cast<int>(blockBoundingRect(block).height());
            ++blockNumber;
        }
    }
}

void CodeEdit::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu *menu = createStandardContextMenu();
    bool hasBlockSelection = mBlockEdit && !mBlockEdit->blockText().isEmpty();
    QAction *lastAct = nullptr;
    for (int i = menu->actions().count()-1; i >= 0; --i) {
        QAction *act = menu->actions().at(i);
        if (act->objectName() == "select-all") {
            if (mBlockEdit) act->setEnabled(false);
            menu->removeAction(act);
            act->disconnect();
            connect(act, &QAction::triggered, this, &CodeEdit::selectAllText);
            menu->insertAction(lastAct, act);
        } else if (act->objectName() == "edit-paste" && act->isEnabled()) {
            menu->removeAction(act);
            act->disconnect();
            connect(act, &QAction::triggered, this, &CodeEdit::pasteClipboard);
            menu->insertAction(lastAct, act);
        } else if (hasBlockSelection) {
            if (act->objectName() == "edit-cut") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEdit::cutSelection);
                menu->insertAction(lastAct, act);
            } else if (act->objectName() == "edit-copy") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEdit::copySelection);
                menu->insertAction(lastAct, act);
            } else if (act->objectName() == "edit-delete") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEdit::clearSelection);
                menu->insertAction(lastAct, act);
            }
        }
        lastAct = act;
    }
    if (!isReadOnly()) {
        QString fileName;
        TextLinkType linkType = checkLinks(e->pos(), true, &fileName);
        updateLinkAppearance(e->pos(), false);

        QAction *actLink = menu->addAction("Open link", [this, e]() { jumpToCurrentLink(e->pos()); });
        actLink->setShortcut(Keys::instance().keySequence(Hotkey::JumpToContext).first());
        if (linkType != linkDirect/* || linkType == linkMark*/) {
            actLink->setEnabled(false);
        }

        QMenu *submenu = menu->addMenu(tr("Advanced"));
        QList<QAction*> ret;
        emit requestAdvancedActions(&ret);
        submenu->addActions(ret);
        menu->addSeparator();
        emit cloneBookmarkMenu(menu);
        QAction *act = new QAction("Toggle &folding", menu);
        act->setShortcut(Keys::instance().keySequence(Hotkey::ToggleBlockFolding).first());
        connect(act, &QAction::triggered, this, &CodeEdit::switchCurrentFolding);
        menu->addAction(act);
        if (textCursor().block().userData()) {
            bool folded;
            int foldRes = foldState(textCursor().blockNumber(), folded);
            act->setEnabled(foldRes % 3 > 0);
        } else act->setEnabled(false);
    }
    menu->exec(e->globalPos());
    delete menu;
}

void CodeEdit::updateLinkAppearance(QPoint pos, bool active)
{
    QTextCursor cur = cursorForPosition(pos);
    int old = mIncludeLinkLine;
    if (active) {
        QString file;
        if (checkLinks(pos, active, &file) == linkDirect)
            mIncludeLinkLine = cur.blockNumber();
        else
            mIncludeLinkLine = -1;
    } else mIncludeLinkLine = -1;
    if (old != mIncludeLinkLine || mLinkActive != active) {
        updateExtraSelections();
    }
    mLinkActive = active;
    lineNumberArea()->setCursor(viewport()->cursor().shape());
}


void CodeEdit::marksChanged(const QSet<int> dirtyLines)
{
    AbstractEdit::marksChanged(dirtyLines);
    bool doPaint = dirtyLines.isEmpty() || dirtyLines.size() > 5;
    if (!doPaint) {
        int firstLine = topVisibleLine();
        for (const int &line: dirtyLines) {
            if (line >= firstLine && line <= firstLine+100) {
                doPaint = true;
                break;
            }
        }
    }
    if (doPaint) {
        mLineNumberArea->update();
        updateLineNumberAreaWidth();
    }
}

void CodeEdit::blockCountHasChanged(int newBlockCount)
{
    Q_UNUSED(newBlockCount)
    mFoldMark = LinePair();
    mLineNumberArea->update();
    updateLineNumberAreaWidth();
}

void CodeEdit::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls()) {
        e->ignore(); // paste to parent widget
    } else {
        AbstractEdit::dragEnterEvent(e);
    }
}

void CodeEdit::duplicateLine()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.insertText(cursor.block().text()+'\n');
    cursor.endEditBlock();
}

void CodeEdit::removeLine()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}

void CodeEdit::commentLine()
{
    QTextCursor cursor = textCursor();
    if (mBlockEdit) {
        QTextBlock startBlock = cursor.document()->findBlockByNumber(mBlockEdit->startLine());
        QTextBlock endBlock = cursor.document()->findBlockByNumber(mBlockEdit->currentLine());
        int columnFrom = mBlockEdit->colFrom();
        int columnTo = mBlockEdit->colTo();
        cursor.setPosition(startBlock.position() + mBlockEdit->colFrom());
        cursor.setPosition(textCursor().block().position() + mBlockEdit->colTo(), QTextCursor::KeepAnchor);

        endBlockEdit();
        applyLineComment(cursor, qMin(startBlock, endBlock), qMax(startBlock.blockNumber(), endBlock.blockNumber()));
        setTextCursor(cursor);
        startBlockEdit(startBlock.blockNumber(), columnTo);
        mBlockEdit->selectTo(endBlock.blockNumber(), columnFrom);
    } else {
        QTextBlock startBlock = cursor.document()->findBlock(qMin(cursor.position(), cursor.anchor()));
        int lastBlockNr = cursor.document()->findBlock(qMax(cursor.position(), cursor.anchor())).blockNumber();
        applyLineComment(cursor, startBlock, lastBlockNr);
        setTextCursor(cursor);
    }

    recalcExtraSelections();
}

bool CodeEdit::hasLineComment(QTextBlock startBlock, int lastBlockNr) {
    bool hasComment = true;
    for (QTextBlock block = startBlock; block.blockNumber() <= lastBlockNr; block = block.next()) {
        if (!block.isValid())
            break;
        if (!block.text().startsWith('*'))
            hasComment = false;
    }
    return hasComment;
}

void CodeEdit::applyLineComment(QTextCursor cursor, QTextBlock startBlock, int lastBlockNr)
{
    bool hasComment = hasLineComment(startBlock, lastBlockNr);
    cursor.beginEditBlock();
    QTextCursor anchor = cursor;
    anchor.setPosition(anchor.anchor());
    for (QTextBlock block = startBlock; block.blockNumber() <= lastBlockNr; block = block.next()) {
        if (!block.isValid()) break;

        cursor.setPosition(block.position());
        if (hasComment)
            cursor.deleteChar();
        else
            cursor.insertText("*");
    }
    cursor.setPosition(anchor.position());
    cursor.setPosition(textCursor().position(), QTextCursor::KeepAnchor);
    cursor.endEditBlock();
}


int CodeEdit::minIndentCount(int fromLine, int toLine)
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

int CodeEdit::indent(int size, int fromLine, int toLine)
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
        if (w <= text.length() && w <= minIndentPos) minIndentPos = w;
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
                editCursor.setPosition(block.position() + insertPos - charCount);
                int tempCount = qMin(charCount, block.length() - (insertPos - charCount));
                editCursor.setPosition(editCursor.position() + tempCount, QTextCursor::KeepAnchor);
                editCursor.removeSelectedText();
            }
        } else {
            if (insertPos < block.length()) {
                editCursor.setPosition(block.position() + insertPos);
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

void CodeEdit::startBlockEdit(int blockNr, int colNr)
{
    if (!mAllowBlockEdit) return;
    if (mBlockEdit) endBlockEdit();
    bool overwrite = overwriteMode();
    if (overwrite) setOverwriteMode(false);
    mBlockEdit = new BlockEdit(this, blockNr, colNr);
    setCursorWidth(0);
    mBlockEdit->setOverwriteMode(overwrite);
    mBlockEdit->startCursorTimer();
    updateLineNumberAreaWidth();
}

void CodeEdit::endBlockEdit(bool adjustCursor)
{
    mBlockEdit->stopCursorTimer();
    if (adjustCursor) mBlockEdit->adjustCursor();
    bool overwrite = mBlockEdit->overwriteMode();
    delete mBlockEdit;
    mBlockEdit = nullptr;
    setCursorWidth(2);
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

QStringList CodeEdit::clipboard(bool *isBlock)
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

CodeEdit::CharType CodeEdit::charType(QChar c)
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

int CodeEdit::findAlphaNum(const QString &text, int start, bool back)
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

void CodeEdit::rawKeyPressEvent(QKeyEvent *e)
{
    AbstractEdit::keyPressEvent(e);
}

AbstractEdit::EditorType CodeEdit::type() const
{
    return EditorType::CodeEditor;
}

void CodeEdit::wordInfo(QTextCursor cursor, QString &word, int &intKind)
{
    QString text = cursor.block().text();
    int start = cursor.positionInBlock();
    int from = findAlphaNum(text, start, true);
    int to = findAlphaNum(text, from, false);
    if (from >= 0 && from <= to) {
        word = text.mid(from, to-from+1);
        start = from + cursor.block().position();
        emit requestSyntaxKind(start+1, intKind);
//        cursor.setPosition(start+1);
//        intState = cursor.charFormat().property(QTextFormat::UserProperty).toInt();
    } else {
        word = "";
        intKind = 0;
    }
}

void CodeEdit::getPositionAndAnchor(QPoint &pos, QPoint &anchor)
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

int CodeEdit::foldStart(int line, bool &folded, QString *closingSymbol) const
{
    int res = -1;
    static QString parentheses("{[(/EMTCPIOFU}])\\emtcpiofu");
    static QVector<QString> closingSymbols {
        "}", "]", ")", "/", "embeddedCode", "embeddedCode", "text", "echo", "put", "externalInput", "externalOutput", "endIf", "fold"
    };
    static int pSplit = parentheses.length()/2;
    QTextBlock block = document()->findBlockByNumber(line);
    syntax::BlockData* dat = syntax::BlockData::fromTextBlock(block);
    if (!dat) return -1;

    folded = dat->isFolded();
    QVector<syntax::ParenthesesPos> parList = dat->parentheses();
    int depth = 0;
//    if (parList.count())
//        DEB() << "parenthesis " << parList.at(0).character << " at " << parList.at(0).relPos;
    for (int i = 0; i < parList.count(); ++i) {
        if (parentheses.indexOf(parList.at(i).character) >= pSplit) {
            if (depth) --depth;
            if (!depth) res = -1;
        } else {
            ++depth;
            if (depth == 1) {
                res = parList.at(i).relPos;
                if (closingSymbol) {
                    int cs = parentheses.indexOf(parList.at(i).character);
                    if (cs < closingSymbols.size())
                        *closingSymbol = closingSymbols.at(cs);
                }
            }
        }
    }
    return res;
}

void CodeEdit::jumpTo(int line, int column)
{
    ensureUnfolded(line);
    AbstractEdit::jumpTo(line, column);
}

PositionPair CodeEdit::matchParentheses(QTextCursor cursor, bool all, int *foldCount) const
{
    static QString parentheses("{[(/EMTCPIOFU}])\\emtcpiofu");
    static int pSplit = parentheses.length()/2;
    static int pAll = parentheses.indexOf("/");
    QTextBlock block = cursor.block();
    if (!block.userData()) return PositionPair();
    syntax::BlockData *startDat = syntax::BlockData::fromTextBlock(block);
//    int state = block.userState();
    QVector<syntax::ParenthesesPos> parList = startDat->parentheses();
    int pos = cursor.positionInBlock();
    int start = -1;
    for (int i = parList.count()-1; i >= 0; --i) {
        if (parList.at(i).relPos == pos || parList.at(i).relPos == pos-1) {
            start = i;
        }
    }
    if (start < 0) return PositionPair();
    // prepare matching search
    int ci = parentheses.indexOf(parList.at(start).character);
    bool back = ci >= pSplit;
    ci = ci % pSplit;
    PositionPair result(block.position() + parList.at(start).relPos);
    result.match = result.pos;
    QStringRef parEnter = parentheses.midRef(back ? pSplit : 0, pSplit);
    QStringRef parLeave = parentheses.midRef(back ? 0 : pSplit, pSplit);
    QVector<QChar> parStack;
    parStack << parLeave.at(ci);
    int startBlockNr = block.blockNumber();
    int pi = start;
    while (block.isValid()) {
        // get next parentheses entry
        if (back ? --pi < 0 : ++pi >= parList.count()) {
            bool isEmpty = true;
            while (block.isValid() && isEmpty) {
                block = back ? block.previous() : block.next();
                if (!block.isValid()) break;
                if (foldCount) *foldCount = block.blockNumber() - startBlockNr;
                syntax::BlockData *dat = syntax::BlockData::fromTextBlock(block);
                if (dat) {
                    parList = dat->parentheses();
                    if (!parList.isEmpty()) isEmpty = false;
                }
            }
            if (isEmpty) continue;
            parList = syntax::BlockData::fromTextBlock(block)->parentheses();
            pi = back ? parList.count()-1 : 0;
        }

        int i = parEnter.indexOf(parList.at(pi).character);
        if (i < 0) {
            // Only last stacked character is valid
            if (parList.at(pi).character == parStack.last()) {
                parStack.removeLast();
                if (parStack.isEmpty()) {
                    if (!all && ci > pAll) return PositionPair(); // only mark embedded on mismatch
                    result.valid = true;
                    result.match = block.position() + parList.at(pi).relPos;
                    return result;
                }
            } else {
                // Mark bad parentheses
                parStack.clear();
                result.match = block.position() + parList.at(pi).relPos;
                return result;
            }
        } else {
            // Stack new character
            parStack << parLeave.at(i);
        }

    }
    return result;
}

void CodeEdit::setOverwriteMode(bool overwrite)
{
    if (mBlockEdit) mBlockEdit->setOverwriteMode(overwrite);
    else AbstractEdit::setOverwriteMode(overwrite);
}

bool CodeEdit::overwriteMode() const
{
    if (mBlockEdit) return mBlockEdit->overwriteMode();
    return AbstractEdit::overwriteMode();
}

inline int CodeEdit::assignmentKind(int p)
{
    int preKind = 0;
    int postKind = 0;
    emit requestSyntaxKind(p-1, preKind);
    emit requestSyntaxKind(p+1, postKind);
    if (postKind == static_cast<int>(syntax::SyntaxKind::IdentifierAssignment)) return 1;
    if (preKind == static_cast<int>(syntax::SyntaxKind::IdentifierAssignment)) return -1;
    if (preKind == static_cast<int>(syntax::SyntaxKind::IdentifierAssignmentEnd)) return -1;
    return 0;
}

void CodeEdit::recalcWordUnderCursor()
{
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
}

void CodeEdit::recalcExtraSelections()
{
    QList<QTextEdit::ExtraSelection> selections;
    mParenthesesMatch = PositionPair();
    if (!mBlockEdit) {
        extraSelCurrentLine(selections);
        recalcWordUnderCursor();
        mParenthesesDelay.start(100);
        int wordDelay = 10;
        if (mSettings->toBool(skEdWordUnderCursor)) wordDelay = 500;
        mWordDelay.start(wordDelay);
    } else {
        extraSelBlockEdit(selections);
        setExtraSelections(selections);
    }
}

void CodeEdit::updateExtraSelections()
{
    QList<QTextEdit::ExtraSelection> selections;
    extraSelCurrentLine(selections);
    extraSelMarks(selections);
    if (!mBlockEdit) {
        QString selectedText = textCursor().selectedText();
        QRegularExpression regexp = search::SearchLocator::search()->regex();

        // word boundary (\b) only matches start-of-string when first character is \w
        // so \b will only be added when first character of selectedText is a \w
        // if first character is not \w  the whole string needs to be matched in order to deactivate HWUC
        QRegularExpression startsWithW("^\\w");
        if (startsWithW.match(selectedText).hasMatch())
            regexp.setPattern("\\b" + regexp.pattern());

        QRegularExpressionMatch match = regexp.match(selectedText);
        bool skipWordTimer = (sender() == &mParenthesesDelay
                              || sender() == this->verticalScrollBar()
                              || sender() == nullptr);

        //    (  not caused by parentheses matching                               ) OR has selection
        if ( (( !extraSelMatchParentheses(selections, sender() == &mParenthesesDelay) || hasSelection())
               // ( depending on settings: no selection necessary OR has selection )
               && (mSettings->toBool(skEdWordUnderCursor) || hasSelection())
               // (  depending on settings: no selection necessary skip word-timer )
               && (mSettings->toBool(skEdWordUnderCursor) || skipWordTimer))
             // AND deactivate when navigating search results
             && match.captured(0).isEmpty()) {
            extraSelCurrentWord(selections);
        }
    }
    extraSelMatches(selections);
    extraSelBlockEdit(selections);
    extraSelIncludeLink(selections);
    setExtraSelections(selections);
}

void CodeEdit::unfold(QTextBlock block)
{
    if (block.userData() && syntax::BlockData::fromTextBlock(block)->foldCount())
        toggleFolding(block);
}

void CodeEdit::extraSelBlockEdit(QList<QTextEdit::ExtraSelection>& selections)
{
    if (mBlockEdit) {
        selections.append(mBlockEdit->extraSelections());
    }
}

void CodeEdit::extraSelCurrentWord(QList<QTextEdit::ExtraSelection> &selections)
{
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
                selection.format.setBackground(toColor(Theme::Edit_currentWordBg));
                selections << selection;
                i += match.capturedLength(1) + match.capturedLength(2);
            }
            top += qRound(blockBoundingRect(block).height());
            block = block.next();
        }
    }
}

bool CodeEdit::extraSelMatchParentheses(QList<QTextEdit::ExtraSelection> &selections, bool first)
{
    if (mParenthesesMatch.isNull())
        mParenthesesMatch = matchParentheses(textCursor());

    if (mParenthesesMatch.isNull()) return false;

    if (mParenthesesMatch.pos == mParenthesesMatch.match) mParenthesesMatch.valid = false;
    QColor fgColor = mParenthesesMatch.valid ? toColor(Theme::Edit_parenthesesValidFg)
                                             : toColor(Theme::Edit_parenthesesInvalidFg);
    QColor bgColor = mParenthesesMatch.valid ? toColor(Theme::Edit_parenthesesValidBg)
                                             : toColor(Theme::Edit_parenthesesInvalidBg);
    QColor bgBlinkColor = mParenthesesMatch.valid ? toColor(Theme::Edit_parenthesesValidBgBlink)
                                                  : toColor(Theme::Edit_parenthesesInvalidBgBlink);
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

void CodeEdit::extraSelMatches(QList<QTextEdit::ExtraSelection> &selections)
{
    search::Search* search = search::SearchLocator::search();
    if (search->regex().pattern().isEmpty() || search->filteredResultList(ViewHelper::location(this)).isEmpty())
        return;

    QRegularExpression regEx = search->regex();

    QTextBlock block = firstVisibleBlock();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    while (block.isValid() && top < viewport()->height()) {
        top += qRound(blockBoundingRect(block).height());

        QRegularExpressionMatchIterator i = regEx.globalMatch(block.text());
        while (i.hasNext()) {
            QRegularExpressionMatch m = i.next();
            QTextEdit::ExtraSelection selection;
            QTextCursor tc(document());
            tc.setPosition(block.position() + m.capturedStart(0));
            tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, m.capturedLength(0));
            selection.cursor = tc;
            selection.format.setForeground(Qt::white);
            selection.format.setBackground(toColor(Theme::Edit_matchesBg));
            selections << selection;
        }

        block = block.next();
    }
}

void CodeEdit::extraSelIncludeLink(QList<QTextEdit::ExtraSelection> &selections)
{
    if (mIncludeLinkLine < 0) return;
    QTextBlock block = document()->findBlockByNumber(mIncludeLinkLine);
    if (!block.isValid()) return;
    QTextEdit::ExtraSelection selection;
    QTextCursor cur(document());
    QString command;
    int fileStart;
    QString file = getIncludeFile(mIncludeLinkLine, fileStart, command);
    cur.setPosition(block.position() + fileStart);
    cur.setPosition(cur.position() + file.length(), QTextCursor::KeepAnchor);
    selection.cursor = cur;
    selection.format.setAnchorHref('"'+file+'"');
    selection.format.setForeground(Theme::color(Theme::Syntax_directiveBody));
    selection.format.setUnderlineColor(Theme::color(Theme::Syntax_directiveBody));
    selection.format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    selections << selection;
}

QPoint CodeEdit::toolTipPos(const QPoint &mousePos)
{
    QPoint pos = AbstractEdit::toolTipPos(mousePos);
    if (mousePos.x() < mLineNumberArea->width())
        pos.setX(mLineNumberArea->width()+6);
    else
        pos.setX(pos.x() + mLineNumberArea->width()+2);
    return pos;
}

QString CodeEdit::getToolTipText(const QPoint &pos)
{
    QString res = AbstractEdit::getToolTipText(pos);
    if (!res.isEmpty()) return res;
    QString fileName;
    checkLinks(pos, true, &fileName);
    if (!fileName.isEmpty()) {
        fileName = QDir::toNativeSeparators(fileName);
        fileName = "<p style='white-space:pre'>"+fileName+"<br><b>Ctrl-click</b> to open</p>";
    }
    return fileName;
}

QString CodeEdit::lineNrText(int blockNr)
{
    return QString::number(blockNr);
}

bool CodeEdit::showLineNr() const
{
    return mSettings->toBool(skEdShowLineNr);
}

bool CodeEdit::showFolding() const
{
    return true;
}

void CodeEdit::setAllowBlockEdit(bool allow)
{
    mAllowBlockEdit = allow;
    if (mBlockEdit) endBlockEdit();
}

int CodeEdit::foldState(int line, bool &folded, int *start, QString *closingSymbol) const
{
    int res = 0;
    int foldPos = foldStart(line, folded, closingSymbol);
    if (start) *start = foldPos;
    bool marked = (mFoldMark.pos <= mFoldMark.match && line >= mFoldMark.pos && line <= mFoldMark.match);
    if (foldPos >= 0) res = folded ? 2 : 1;
    if (marked) res += 3;
    return res;
}

void CodeEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(mLineNumberArea);
    bool hasMarks = marks() && marks()->hasVisibleMarks();
    if (hasMarks && mIconCols == 0) {
        QTimer::singleShot(0, this, &CodeEdit::updateLineNumberAreaWidth);
        event->accept();
        return;
    }
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    QRectF fRect = blockBoundingGeometry(block).translated(contentOffset());
    int top = static_cast<int>(fRect.top());
    int bottom = top + static_cast<int>(fRect.height());
    int markFrom = textCursor().blockNumber();
    int markTo = textCursor().blockNumber();
    if (markFrom > markTo) qSwap(markFrom, markTo);

    QRect paintRect(event->rect());
    painter.fillRect(paintRect, toColor(Theme::Edit_linenrAreaBg));
    int widthForNr = mLineNumberArea->width() - (showFolding() ? iconSize() : 0);

    QRect markRect(paintRect.left(), top, paintRect.width(), static_cast<int>(fRect.height())+1);
    QRect foldRect(widthForNr, top, paintRect.width()-widthForNr, static_cast<int>(fRect.height())+1);
    while (block.isValid() && top <= paintRect.bottom()) {
        if (block.isVisible() && bottom >= paintRect.top()) {
            bool mark = mBlockEdit ? mBlockEdit->hasBlock(block.blockNumber())
                                   : blockNumber >= markFrom && blockNumber <= markTo;
            if (mark) {
                markRect.moveTop(top);
                markRect.setHeight(bottom-top);
                painter.fillRect(markRect, toColor(Theme::Edit_linenrAreaMarkBg));
            }

            if(showLineNr()) {
                QString number = lineNrText(blockNumber + 1);
                QFont f = font();
                f.setBold(mark);
                painter.setFont(f);
                painter.setPen(mark ? toColor(Theme::Edit_linenrAreaMarkFg)
                                    : toColor(Theme::Edit_linenrAreaFg));
                painter.drawText(0, top, widthForNr, fontMetrics().height(), Qt::AlignRight, number);
            }

            if (hasMarks && marks()->contains(absoluteBlockNr(blockNumber))) {
                int iTop = (2+top+bottom-iconSize())/2;
                painter.drawPixmap(1, iTop, marks()->value(absoluteBlockNr(blockNumber))->icon().pixmap(iconSize(),iconSize()));
            }
            if (showFolding()) {
                bool folded = false;
                int foldRes = foldState(blockNumber, folded);
                if (foldRes > 2) {
                    foldRect.moveTop(top);
                    foldRect.setHeight(bottom-top);
                    if (mFoldMark.valid) {
                        painter.fillRect(foldRect, toColor(Theme::Edit_linenrAreaFoldBg));
                    } else {
                        painter.fillRect(foldRect, toColor(Theme::Edit_parenthesesInvalidBg));
                    }
                }
                if (foldRes % 3 > 0) {
                    int iTop = top + (4+fontMetrics().height()-iconSize())/2;
                    QIcon icon = Theme::icon(foldRes % 3 == 2 ? ":/solid/triangle-right" : ":/solid/triangle-down", true);
                    painter.drawPixmap(widthForNr+1, iTop, icon.pixmap(iconSize()-2,iconSize()-2));
                    if (foldRes % 3 == 2) {
                        QRect foldRect(0, bottom-1, width(), 1);
                        painter.fillRect(foldRect, toColor(Theme::Edit_foldLineBg));
                    }
                }
          }
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

CodeEdit::BlockEdit::BlockEdit(CodeEdit* edit, int blockNr, int colNr)
    : mEdit(edit)
{
    Q_ASSERT_X(edit, "BlockEdit constructor", "BlockEdit needs a valid editor");
    mStartLine = blockNr;
    mCurrentLine = blockNr;
    mColumn = colNr;
    setSize(0);
}

CodeEdit::BlockEdit::~BlockEdit()
{
    mSelections.clear();
    mEdit->updateExtraSelections();
}

void CodeEdit::BlockEdit::selectTo(int blockNr, int colNr)
{
    mCurrentLine = blockNr;
    setSize(colNr - mColumn);
    updateExtraSelections();
    startCursorTimer();
}

void CodeEdit::BlockEdit::toEnd(bool select)
{
    int end = 0;
    for (QTextBlock block = mEdit->document()->findBlockByNumber(qMin(mStartLine, mCurrentLine))
         ; block.blockNumber() <= qMax(mStartLine, mCurrentLine); block=block.next()) {
        if (end < block.length()-1) end = block.length()-1;
        if (!block.isValid()) break;
    }
    if (select) setSize(end - mColumn);
    else {
        mColumn = end;
        setSize(0);
    }
}

QString CodeEdit::BlockEdit::blockText()
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

void CodeEdit::BlockEdit::selectionToClipboard()
{
    QMimeData *mime = new QMimeData();
    mime->setText(blockText());
    mime->setData("application/x-qt-windows-mime;value=\"MSDEVColumnSelect\"", QByteArray());
    mime->setData("application/x-qt-windows-mime;value=\"Borland IDE Block Type\"", QByteArray(1,char(2)));
    QApplication::clipboard()->setMimeData(mime);
}

int CodeEdit::BlockEdit::currentLine() const
{
    return mCurrentLine;
}

int CodeEdit::BlockEdit::column() const
{
    return mColumn;
}

void CodeEdit::BlockEdit::setColumn(int column)
{
    mColumn = column;
}

void CodeEdit::BlockEdit::setOverwriteMode(bool overwrite)
{
    mOverwrite = overwrite;
}

bool CodeEdit::BlockEdit::overwriteMode() const
{
    return mOverwrite;
}

void CodeEdit::BlockEdit::setSize(int size)
{
    //const ensures the size is only changed HERE
    *(const_cast<int*>(&mSize)) = size;
    mLastCharType = CharType::None;
}

int CodeEdit::BlockEdit::startLine() const
{
    return mStartLine;
}

void CodeEdit::BlockEdit::keyPressEvent(QKeyEvent* e)
{
    QSet<int> moveKeys;
    moveKeys << Qt::Key_Home << Qt::Key_End << Qt::Key_Down << Qt::Key_Up
             << Qt::Key_Left << Qt::Key_Right << Qt::Key_PageUp << Qt::Key_PageDown;
    if (moveKeys.contains(e->key())) {
        QTextBlock block = mEdit->document()->findBlockByNumber(mCurrentLine);
        bool isMove = e->modifiers() & Qt::AltModifier;
        bool isShift = e->modifiers() & Qt::ShiftModifier;
        bool isWord = e->modifiers() & Qt::ControlModifier;
#ifdef __APPLE__
//        bool isWord = (e->modifiers() & Qt::MetaModifier);
#else
#endif
        if (e->key() == Qt::Key_Home) {
            if (isShift) setSize(-mColumn);
            else {
                mColumn = 0;
                setSize(0);
            }
        } else if (e->key() == Qt::Key_End) {
            toEnd(isShift);
        } else if (e->key() == Qt::Key_Right) {
            if (isWord) {
                int size = mSize;
                EditorHelper::nextWord(mColumn, size, block.text());
                if (isShift) setSize(size);
                else {
                    mColumn += size;
                    setSize(0);
                }
            } else if (isMove) {
                setSize(mSize+1);
            } else {
                mColumn += 1;
                if (isShift)
                    setSize(mSize-1);
            }
        } else if (e->key() == Qt::Key_Left && mColumn+mSize > 0) {
            if (isWord) {
                int size = mSize;
                EditorHelper::prevWord(mColumn, size, block.text());
                if (isShift) setSize(size);
                else {
                    mColumn += size;
                    setSize(0);
                }
            } else if (isMove) {
                setSize(mSize-1);
            } else if (mColumn > 0) {
                mColumn -= 1;
                if (isShift)
                    setSize(mSize+1);
            } else {
                e->accept();
                return;
            }
        } else if (e->key() == Qt::Key_Down) {
            if ((isWord || isMove)  && mCurrentLine < mEdit->blockCount()-1) {
                mCurrentLine += 1;
            } else if (isShift && mStartLine < mEdit->blockCount()-1) {
                mStartLine += 1;
            } else if (qMax(mStartLine, mCurrentLine) < mEdit->blockCount()-1) {
                mCurrentLine += 1;
                mStartLine += 1;
            } else {
                e->accept();
                return;
            }
        } else if (e->key() == Qt::Key_Up) {
            if ((isWord || isMove) && mCurrentLine > 0) {
                mCurrentLine -= 1;
            } else if (isShift && mStartLine < mEdit->blockCount()-1) {
                mStartLine -= 1;
            } else if (qMin(mStartLine, mCurrentLine) > 0) {
                mCurrentLine -= 1;
                mStartLine -= 1;
            } else {
                e->accept();
                return;
            }
        } else {
            e->accept();
            return;
        }
        QTextCursor cursor(block);
        if (block.length() > mColumn+mSize)
            cursor.setPosition(block.position()+mColumn+mSize);
        else
            cursor.setPosition(block.position()+block.length()-1);
        mEdit->setTextCursor(cursor);
        updateExtraSelections();
        e->accept();
        emit mEdit->cursorPositionChanged();
    } else if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
        if (!mSize && mColumn >= 0) {
            mLastCharType = CharType::None;
            setSize((e->key() == Qt::Key_Backspace) ? -1 : 1);
        }
        replaceBlockText("");
    } else if (e == Hotkey::Indent) {
        mEdit->indent(mEdit->mSettings->toInt(skEdTabSize));
        e->accept();
        return;
    } else if (e == Hotkey::Outdent) {
        mEdit->indent(-mEdit->mSettings->toInt(skEdTabSize));
        e->accept();
        return;
    } else if (e->text().length()) {
        mEdit->mBlockEditRealPos = mEdit->textCursor().position();
        QTextCursor cur = mEdit->textCursor();
        cur.joinPreviousEditBlock();
        mEdit->setTextCursor(cur);
        mEdit->rawKeyPressEvent(e);
        QTimer::singleShot(0, mEdit, &CodeEdit::checkBlockInsertion);
    }

    startCursorTimer();
}

void CodeEdit::BlockEdit::startCursorTimer()
{
    mEdit->mBlinkBlockEdit.start();
    mBlinkStateHidden = true;
    mEdit->lineNumberArea()->update(mEdit->lineNumberArea()->visibleRegion());
    refreshCursors();
}

void CodeEdit::BlockEdit::stopCursorTimer()
{
    mEdit->mBlinkBlockEdit.stop();
    mBlinkStateHidden = true;
    mEdit->lineNumberArea()->update(mEdit->lineNumberArea()->visibleRegion());
    refreshCursors();
}

void CodeEdit::BlockEdit::refreshCursors()
{
    mBlinkStateHidden = !mBlinkStateHidden;
    mEdit->viewport()->update(mEdit->viewport()->visibleRegion());
}

void CodeEdit::BlockEdit::paintEvent(QPaintEvent *e)
{
    QPainter painter(mEdit->viewport());
    QPointF offset(mEdit->contentOffset()); //
    QRect evRect = e->rect();
    bool editable = !mEdit->isReadOnly();
    painter.setClipRect(evRect);
    int cursorColumn = mColumn+mSize;
    QFontMetricsF metric(mEdit->font());
    double spaceWidth = metric.averageCharWidth();
    QTextBlock block = mEdit->firstVisibleBlock();
    QTextCursor cursor(block);
    cursor.setPosition(block.position()+block.length()-1);
    double left = block.layout()->lineAt(0).rect().left();

    while (block.isValid()) {
        QRectF blockRect = mEdit->blockBoundingRect(block).translated(offset);
        if (!hasBlock(block.blockNumber()) || !block.isVisible()) {
            offset.ry() += blockRect.height();
            block = block.next();
            continue;
        }

        // draw extended extra-selection for lines past line-end
        int beyondEnd = qMax(mColumn, mColumn+mSize);
        QString str = block.text();
        if (beyondEnd >= block.length()) {
            str += QString(beyondEnd - str.length()+1, ' ');
            cursor.setPosition(block.position()+block.length()-1);
            // we have to draw selection beyond the line-end
            int beyondStart = qMax(block.length()-1, qMin(mColumn, mColumn+mSize));
            QRectF selRect = mEdit->cursorRect(cursor);
            if (block.length() <= beyondStart) {
                selRect.moveLeft(left + metric.width(str.left(beyondStart)));
            }
            selRect.setRight(left + metric.width(str.left(beyondEnd)));
            painter.fillRect(selRect, mEdit->palette().highlight());
        }

        if (mBlinkStateHidden) {
            offset.ry() += blockRect.height();
            block = block.next();
            continue;
        }

        cursor.setPosition(block.position()+qMin(block.length()-1, cursorColumn));
        QRectF cursorRect = mEdit->cursorRect(cursor);
        if (block.length() <= cursorColumn) {
            cursorRect.setX(left + metric.width(str.left(cursorColumn)));
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

void CodeEdit::BlockEdit::updateExtraSelections()
{
    mSelections.clear();
    QTextCursor cursor(mEdit->document());
    for (int lineNr = qMin(mStartLine, mCurrentLine); lineNr <= qMax(mStartLine, mCurrentLine); ++lineNr) {
        QTextBlock block = mEdit->document()->findBlockByNumber(lineNr);
        QTextEdit::ExtraSelection select;
        select.format.setBackground(mEdit->palette().highlight());
        select.format.setForeground(mEdit->palette().highlightedText());

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

void CodeEdit::BlockEdit::adjustCursor()
{
    QTextBlock block = mEdit->document()->findBlockByNumber(mCurrentLine);
    QTextCursor cursor(block);
    cursor.setPosition(block.position() + qMin(block.length()-1, mColumn+mSize));
    mEdit->setTextCursor(cursor);
}

void CodeEdit::BlockEdit::replaceBlockText(QString text)
{
    replaceBlockText(QStringList() << text);
}

void CodeEdit::BlockEdit::replaceBlockText(QStringList texts)
{
    if (mEdit->isReadOnly()) return;
    if (texts.isEmpty()) texts << "";
    CharType charType = texts.at(0).length()>0 ? mEdit->charType(texts.at(0).at(0)) : CharType::None;
    bool newUndoBlock = texts.count() > 1 || mLastCharType != charType || texts.at(0).length() != 1;
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
            if (rmSize > 0) {
                cursor.setPosition(cursor.position()+rmSize, QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
            }
        } else {
            cursor.setPosition(block.position() + qMin(block.length()-1, fromCol));
        }
        if (!addText.isEmpty()) cursor.insertText(addText);
        block = block.previous();
        i = (i>0) ? i-1 : texts.count()-1;
    }
    // unjoin the Block-Edit insertion from the may-follow normal insertion
    cursor.insertText(" ");
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    if (mSize < 0) mColumn += mSize;
    int insertWidth = -1;
    for (QString s: texts) {
        for (QStringRef ref: s.splitRef('\n')) {
            if (insertWidth < ref.length()) insertWidth = ref.length();
        }
    }
    mColumn += insertWidth;
    setSize(0);
    mLastCharType = charType;
    cursor.endEditBlock();
}

void LineNumberArea::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    pos.setX(pos.x()-width());
    QMouseEvent e(event->type(), pos, event->button(), event->buttons(), event->modifiers());
    if (mCodeEditor->showFolding() && e.pos().x() > -mCodeEditor->iconSize()) {
        QTextBlock block = mCodeEditor->cursorForPosition(e.pos()).block();
        block = mCodeEditor->findFoldStart(block);
        if (mCodeEditor->toggleFolding(block)) {
            mNoCursorFocus = true;
            event->accept();
            return;
        }
    }

    mCodeEditor->mousePressEvent(&e);
}

void LineNumberArea::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    LinePair newFoldMark;
    pos.setX(pos.x()-width());
    if (mCodeEditor->showFolding() && pos.x() > -mCodeEditor->iconSize()) {
        QTextBlock block = mCodeEditor->cursorForPosition(pos).block();
        block = mCodeEditor->findFoldStart(block);
        newFoldMark = mCodeEditor->findFoldBlock(block.blockNumber(), true);
    }
    if (newFoldMark != mCodeEditor->mFoldMark) {
        mCodeEditor->mFoldMark = newFoldMark;
        update(rect());
    }
    if (mNoCursorFocus) {
        event->accept();
        return;
    }
    QMouseEvent e(event->type(), pos, event->button(), event->buttons(), event->modifiers());
    mCodeEditor->mouseMoveEvent(&e);
}

void LineNumberArea::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    if (mNoCursorFocus) {
        mNoCursorFocus = false;
        event->accept();
        return;
    }
    pos.setX(pos.x()-width());
    QMouseEvent e(event->type(), pos, event->button(), event->buttons(), event->modifiers());
    mCodeEditor->mouseReleaseEvent(&e);
}

void LineNumberArea::wheelEvent(QWheelEvent *event)
{
    mCodeEditor->wheelEvent(event);
}

void LineNumberArea::leaveEvent(QEvent *event)
{
    mCodeEditor->mFoldMark = LinePair();
    repaint(rect());
    QWidget::leaveEvent(event);
}


void CodeEdit::moveLines(bool moveLinesUp)
{
    if (mBlockEdit)
        return;
    QTextCursor cursor(textCursor());
    QTextCursor anchor = cursor;
    cursor.beginEditBlock();
    anchor.setPosition(cursor.anchor());
    QTextBlock firstBlock;
    QTextBlock lastBlock;
    if (cursor.anchor() >= cursor.position()) {
        firstBlock = cursor.block();
        lastBlock = anchor.block();
        if ((!anchor.positionInBlock()) && (cursor.hasSelection())) {
            lastBlock = lastBlock.previous();
        }
    } else {
        firstBlock = anchor.block();
        lastBlock = cursor.block();
        if ((!cursor.positionInBlock()) && (cursor.hasSelection())) {
            lastBlock = lastBlock.previous();
        }
    }
    int shift = 0;
    QPoint selection(anchor.position(), cursor.position());
    if (moveLinesUp && firstBlock.blockNumber()) {
        QTextCursor ncur(firstBlock.previous());
        ncur.setPosition(firstBlock.position(), QTextCursor::KeepAnchor);
        QString temp = ncur.selectedText();
        ncur.removeSelectedText();
        if (!lastBlock.next().isValid()) {
            ncur.setPosition(lastBlock.position() + lastBlock.length() - 1);
            temp = '\n' + temp.left(temp.size() - 1);
        } else
            ncur.setPosition(lastBlock.next().position());
        ncur.insertText(temp);
        shift = -temp.length();
    } else if (!moveLinesUp && lastBlock != document()->lastBlock()) {
        QTextCursor ncur(lastBlock.next());
        bool isEnd = !lastBlock.next().next().isValid();
        if (isEnd)
            ncur.setPosition(ncur.position() + ncur.block().length() - 1, QTextCursor::KeepAnchor);
        else
            ncur.setPosition(ncur.position() + ncur.block().length(), QTextCursor::KeepAnchor);
        QString temp = ncur.selectedText();
        ncur.removeSelectedText();
        if (isEnd) {
            temp += '\n';
            ncur.deletePreviousChar();
        }
        ncur.setPosition(firstBlock.position());
        ncur.insertText(temp);
        shift = temp.length();
    }
    cursor.setPosition(selection.x() + shift);
    cursor.setPosition(selection.y() + shift, QTextCursor::KeepAnchor);
    cursor.endEditBlock();
    setTextCursor(cursor);
}



} // namespace studio
} // namespace gams
