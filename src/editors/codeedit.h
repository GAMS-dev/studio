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
#ifndef CODEEDIT_H
#define CODEEDIT_H

#include <QTextBlockUserData>
#include <QHash>
#include <QIcon>
#include <QTimer>
#include "editors/abstractedit.h"
#include "syntax/textmark.h"
#include "syntax/blockdata.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

namespace gams {
namespace studio {

class Settings;
class LineNumberArea;
class SearchWidget;

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
    Q_ENUM(CharType)

public:
    CodeEdit(QWidget *parent = nullptr);
    ~CodeEdit() override;

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    virtual int lineNumberAreaWidth();
    virtual int foldState(int line, bool &folded, int *start = nullptr, QString *closingSymbol = nullptr) const;
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
    void wordInfo(QTextCursor cursor, QString &word, int &intKind);
    void getPositionAndAnchor(QPoint &pos, QPoint &anchor);
    PositionPair matchParentheses(QTextCursor cursor, bool all = false, int *foldCount = nullptr) const;
    void setOverwriteMode(bool overwrite) override;
    bool overwriteMode() const override;
    void extendedRedo();
    void extendedUndo();
    void convertToLower();
    void convertToUpper();
    EditorType type() const override;
    QString wordUnderCursor() const;
    virtual bool hasSelection() const;
    void disconnectTimers() override;
    int foldStart(int line, bool &folded, QString *closingSymbol = nullptr) const;
    void foldAll();
    void unfoldAll();
    void jumpTo(int line, int column = 0) override;

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
    virtual QString lineNrText(int blockNr);
    virtual bool showLineNr() const;
    virtual bool showFolding() const;
    void setAllowBlockEdit(bool allow);
    virtual void recalcWordUnderCursor();
    void extraSelBlockEdit(QList<QTextEdit::ExtraSelection>& selections);
    virtual void extraSelCurrentWord(QList<QTextEdit::ExtraSelection>& selections);
    bool extraSelMatchParentheses(QList<QTextEdit::ExtraSelection>& selections, bool first);
    virtual void extraSelMatches(QList<QTextEdit::ExtraSelection> &selections);
    void extraSelIncludeLink(QList<QTextEdit::ExtraSelection> &selections);
    QTimer &wordDelayTimer() { return mWordDelay; }
    QPoint toolTipPos(const QPoint &mousePos) override;
    QString getToolTipText(const QPoint &pos) override;
    bool ensureUnfolded(int line) override;
    QString resolveHRef(QString href);
    QString getIncludeFile(int line, int &fileStart, QString &code);
    TextLinkType checkLinks(const QPoint &mousePos, bool greedy, QString *fName = nullptr) override;
    void jumpToCurrentLink(const QPoint &mousePos) override;

signals:
    void requestMarkHash(QHash<int, TextMark*>* marks, TextMark::Type filter);
    void requestMarksEmpty(bool* marksEmpty);
    void requestSyntaxKind(int position, int &intKind);
    void searchFindNextPressed();
    void searchFindPrevPressed();
    void requestAdvancedActions(QList<QAction*>* actions);
    void hasHRef(const QString &href, QString &fileName);
    void jumpToHRef(const QString &href);

public slots:
    void clearSelection();
    void cutSelection();
    virtual void copySelection();
    virtual void selectAllText();
    virtual void pasteClipboard();
    void updateExtraSelections() override;
    void unfold(QTextBlock block) override;

protected slots:
    void marksChanged(const QSet<int> dirtyLines = QSet<int>()) override;

private slots:
    void blockCountHasChanged(int newBlockCount);
    void updateLineNumberAreaWidth(/*int newBlockCount*/);
    void recalcExtraSelections();
    void updateLineNumberArea(const QRect &, int);
    void blockEditBlink();
    void checkBlockInsertion();
    void undoCommandAdded();
    void switchCurrentFolding();

private:
    friend class BlockEdit;
    friend class LineNumberArea;

    void adjustIndent(QTextCursor cursor);
    void truncate(QTextBlock block);
    int textCursorColumn(QPoint mousePos);
    void startBlockEdit(int blockNr, int colNr);
    void endBlockEdit(bool adjustCursor = true);
    QStringList clipboard(bool* isBlock = nullptr); // on relevant Block-Edit data returns multiple strings
    CharType charType(QChar c);
    void updateTabSize();
    inline int assignmentKind(int p);
    bool hasLineComment(QTextBlock startBlock, int lastBlockNr);
    void applyLineComment(QTextCursor cursor, QTextBlock startBlock, int lastBlockNr);

    static int findAlphaNum(const QString &text, int start, bool back);
    void rawKeyPressEvent(QKeyEvent *e);
    void updateBlockEditPos();
    void updateLinkAppearance(QPoint pos, bool active = true);
    bool allowClosing(int chIndex);
    bool toggleFolding(QTextBlock block);
    LinePair findFoldBlock(int line, bool onlyThisLine = false) const override;
    QTextBlock findFoldStart(QTextBlock block) const;
    bool unfoldBadBlock(QTextBlock block);
    void checkCursorAfterFolding();

protected:
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
        void setSize(int size);

    private:
        CodeEdit* mEdit;
        int mStartLine = 0;
        int mCurrentLine = 0;
        int mColumn = 0;
        const int mSize = 0;
        bool mBlinkStateHidden = false;
        CharType mLastCharType = CharType::None;
        QList<QTextEdit::ExtraSelection> mSelections;
        bool mOverwrite = false;
    };

protected:
    BlockEdit* blockEdit() {return mBlockEdit;}

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
    PositionPair mParenthesesMatch;
    Settings *mSettings = nullptr;
    int mBlockEditRealPos = -1;
    QString mBlockEditInsText;
    QVector<BlockEditPos*> mBlockEditPos;
    bool mSmartType = false;
    int mIconCols = 0;
    const QString mOpening = "([{'\""; // characters that will be auto closed if mSmartType is true
    const QString mClosing = ")]}'\""; // and their corresponding partner
    bool mAllowBlockEdit = true;
    int mLnAreaWidth = 0;
    LinePair mFoldMark;
    int mIncludeLinkLine = -1;
    bool mLinkActive = false;
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
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    CodeEdit *mCodeEditor;
    QHash<int, QIcon> mIcons;
    bool mNoCursorFocus = false;

};


} // namespace studio
} // namespace gams

#endif // CODEEDIT_H
