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
#ifndef CODEEDIT_H
#define CODEEDIT_H

#include <QTextBlockUserData>
#include <QHash>
#include <QIcon>
#include <QTimer>
#include "editors/abstractedit.h"
#include "syntax/textmark.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

namespace gams {
namespace studio {

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

struct ParenthesesMatch {
    ParenthesesMatch(int _pos = -1, int _match = -1, int _inOutMatch = -1, bool _valid = false)
        : pos(_pos), match(_match), inOutMatch(_inOutMatch), valid(_valid) {}
    bool isValid() {return pos>=0;}
    int pos;
    int match;
    int inOutMatch;
    bool valid;
};

struct ParenthesesPos
{
    ParenthesesPos() : character(QChar()), relPos(-1) {}
    ParenthesesPos(QChar _character, int _relPos) : character(_character), relPos(_relPos) {}
    QChar character;
    int relPos;
};

class BlockData : public QTextBlockUserData
{
public:
    BlockData() { mparentheses.reserve(10);}
    ~BlockData() {}
    QChar charForPos(int relPos);
    bool isEmpty() {return mparentheses.isEmpty();}
    QVector<ParenthesesPos> parentheses() const;
    void setparentheses(const QVector<ParenthesesPos> &parentheses);

private:
    // if extending the data remember to enhance isEmpty()
    QVector<ParenthesesPos> mparentheses;
};

struct BlockEditPos
{
    BlockEditPos(int _startLine, int _endLine, int _column)
        : startLine(_startLine), currentLine(_endLine), column(_column) {}
    int startLine;
    int currentLine;
    int column;
};

class CodeEdit : public AbstractEdit
{
    Q_OBJECT

public:
    CodeEdit(QWidget *parent = nullptr);
    ~CodeEdit() override;

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    int iconSize();
    LineNumberArea* lineNumberArea();

    /// Indents a part of the text. If the cursor is beyond the shortest leading whitespace-part the indent- or
    /// outdentation is performed at the cursor position.
    ///
    /// \param size The base value to indent. A negative value outdents the lines.
    /// \param fromLine Defaults to the line with the cursors anchor
    /// \param toLine Defaults to the line with the cursors position
    /// \return The number of chars indented (or outdented if negative)
    int indent(int size, int fromLine = -1, int toLine = -1);
    void duplicateLine();
    void removeLine();
    void commentLine();
    int minIndentCount(int fromLine = -1, int toLine = -1);
    void wordInfo(QTextCursor cursor, QString &word, int &intState);
    void getPositionAndAnchor(QPoint &pos, QPoint &anchor);
    ParenthesesMatch matchParentheses();
    void setOverwriteMode(bool overwrite) override;
    bool overwriteMode() const override;
    void setSettings(StudioSettings *settings);
    void extendedRedo();
    void extendedUndo();
    void convertToLower();
    void convertToUpper();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

signals:
    void requestMarkHash(QHash<int, TextMark*>* marks, TextMark::Type filter);
    void requestMarksEmpty(bool* marksEmpty);
    void requestSyntaxState(int position, int &intState);
    void searchFindNextPressed();
    void searchFindPrevPressed();
    void requestAdvancedActions(QList<QAction*>* actions);

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
    void blockEditBlink();
    void checkBlockInsertion();
    void undoCommandAdded();

private:
    friend class BlockEdit;
    void adjustIndent(QTextCursor cursor);
    void truncate(QTextBlock block);
    void extraSelBlockEdit(QList<QTextEdit::ExtraSelection>& selections);
    void extraSelCurrentLine(QList<QTextEdit::ExtraSelection>& selections);
    void extraSelCurrentWord(QList<QTextEdit::ExtraSelection>& selections);
    bool extraSelMatchParentheses(QList<QTextEdit::ExtraSelection>& selections, bool first);
    void extraSelMatches(QList<QTextEdit::ExtraSelection> &selections);
    int textCursorColumn(QPoint mousePos);
    void startBlockEdit(int blockNr, int colNr);
    void endBlockEdit();
    QStringList clipboard(bool* isBlock = nullptr); // on relevant Block-Edit data returns multiple strings
    CharType charType(QChar c);
    void updateTabSize();
    inline int assignmentKind(int p);
    bool hasLineComment(QTextBlock startBlock, int lastBlockNr);
    void applyLineComment(QTextCursor cursor, QTextBlock startBlock, int lastBlockNr);

    static int findAlphaNum(const QString &text, int start, bool back);
    void rawKeyPressEvent(QKeyEvent *e);
    void updateBlockEditPos();

private:
    class BlockEdit
    {
    public:
        BlockEdit(CodeEdit* edit, int blockNr, int colNr);
        virtual ~BlockEdit();
        void keyPressEvent(QKeyEvent *e);
        inline int hasBlock(int blockNr) {
            return blockNr>=qMin(mCurrentLine,mStartLine) && blockNr<=qMax(mCurrentLine,mStartLine); }
        int colFrom() { return mColumn+mSize; }
        int colTo() { return mColumn; }
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
        int startLine() const;
        int currentLine() const;
        int column() const;
        void setColumn(int column);
        void setOverwriteMode(bool overwrite);
        bool overwriteMode() const;

    private:
        CodeEdit* mEdit;
        int mStartLine = 0;
        int mCurrentLine = 0;
        int mColumn = 0;
        int mSize = 0;
        bool mBlinkStateHidden = false;
        CharType mLastCharType = CharType::None;
        QList<QTextEdit::ExtraSelection> mSelections;
        bool mOverwrite = false;
    };

private:
    LineNumberArea *mLineNumberArea;
    int mCurrentCol;
    QTimer mCursorTimer;
    QPoint mDragStart;
    BlockEdit* mBlockEdit = nullptr;
    QTimer mBlinkBlockEdit;
    QString mWordUnderCursor;
    bool mOverwriteActivated = false;
    QTimer mWordDelay;
    QTimer mParenthesesDelay;
    ParenthesesMatch mParenthesesMatch;
    StudioSettings *mSettings = nullptr;
    int mBlockEditRealPos = -1;
    QString mBlockEditInsText;
    QVector<BlockEditPos*> mBlockEditPos;

public:
    BlockEdit *blockEdit() const;

    // AbstractEditor interface
public:
    EditorType type() override;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEdit *editor) : QWidget(editor) {
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
    CodeEdit *mCodeEditor;
    QHash<int, QIcon> mIcons;

};


} // namespace studio
} // namespace gams

#endif // CODEEDIT_H
