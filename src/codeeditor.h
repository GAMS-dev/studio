/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QtWidgets>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

namespace gams {
namespace studio {

class TextMark;
class StudioSettings;
class LineNumberArea;
class SearchWidget;

enum class CharType {
    None,
    Ctrl,
    Seperator,
    Punctuation,
    Other,
    Number,
    LetterUCase,
    LetterLCase,
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(StudioSettings *settings, QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    int iconSize();
    LineNumberArea* lineNumberArea();
    QMimeData* createMimeDataFromSelection() const override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e);

signals:
    void requestMarkHash(QHash<int, TextMark*>* marks);
    void requestMarksEmpty(bool* marksEmpty);
    void highlightWordUnderCursor(QString word);

public slots:
    void clearSelection();
    void cutSelection();
    void copySelection();
    void pasteClipboard();

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void recalcExtraSelections();
    void updateExtraSelections();
    void updateLineNumberArea(const QRect &, int);
    void onCursorIdle();
    void onCursorPositionChanged();
    void blockEditBlink();

private:
    friend class BlockEdit;
    void adjustIndent(QTextCursor cursor);
    void truncate(QTextBlock block);
    void duplicateLine();
    void removeLine();
    int minIndentCount(int fromLine = -1, int toLine = -1);
    int indent(int size, int fromLine = -1, int toLine = -1);
    void extraSelBlockEdit(QList<QTextEdit::ExtraSelection>& selections);
    void extraSelCurrentLine(QList<QTextEdit::ExtraSelection>& selections);
    void extraSelCurrentWord(QList<QTextEdit::ExtraSelection>& selections);

    int textCursorColumn(QPoint mousePos);
    void startBlockEdit(int blockNr, int colNr);
    void endBlockEdit();
    QStringList clipboard(bool* isBlock = nullptr); // on relevant Block-Edit data returns multiple strings
    CharType charType(QChar c);

private:
    class BlockEdit
    {
    public:
        BlockEdit(CodeEditor* edit, int blockNr, int colNr);
        virtual ~BlockEdit();
        void keyPressEvent(QKeyEvent *e);
        void keyReleaseEvent(QKeyEvent *e);
        inline int hasBlock(int blockNr) {
            return blockNr>=qMin(mCurrentLine,mStartLine) && blockNr<=qMax(mCurrentLine,mStartLine); }
        int colFrom() { return 0; }
        int colTo() { return 0; }
        void startCursorTimer();
        void stopCursorTimer();
        void refreshCursors();
        void paintEvent(QPaintEvent *e);
        void replaceBlockText(QString text);
        void replaceBlockText(QStringList texts);
        void updateExtraSelections();
        void adjustCursor();
        void selectTo(int blockNr, int colNr);
        void selectToEnd();
        QString blockText();
        inline QList<QTextEdit::ExtraSelection> extraSelections() const { return mSelections; }
        void selectionToClipboard();

    private:
        CodeEditor* mEdit;
        int mStartLine = 0;
        int mCurrentLine = 0;
        int mColumn = 0;
        int mSize = 0;
        bool mBlinkStateHidden = false;
        CharType mLastCharType = CharType::None;
        QList<QTextEdit::ExtraSelection> mSelections;
    };

private:
    const int WORD_UNDER_CURSOR_HIGHLIGHT_TIMER = 600;
    LineNumberArea *mLineNumberArea;
    int mCurrentCol;
    StudioSettings *mSettings;
    QTimer mCursorTimer;
    QPoint mDragStart;
    BlockEdit* mBlockEdit = nullptr;
    QTimer mBlinkBlockEdit;
    QString mWordUnderCursor;
    QTimer mWordDelay;
};



class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        mCodeEditor = editor;
    }

    QSize sizeHint() const override {
        return QSize(mCodeEditor->lineNumberAreaWidth(), 0);
    }
    QHash<int, QIcon> &icons() {
        return mIcons;
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        mCodeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *mCodeEditor;
    QHash<int, QIcon> mIcons;

};


} // namespace studio
} // namespace gams

#endif // CODEEDITOR_H
