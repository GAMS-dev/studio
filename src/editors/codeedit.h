/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "syntax/syntaxcommon.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

namespace gams {
namespace studio {

class Settings;
class LineNumberArea;
class SearchWidget;
class CodeCompleter;

enum BreakpointType {bpNone, bpAimedBp, bpRealBp};

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
    enum BlockSelectState {bsNone, bsKey, bsMouse};
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
    void moveLines(bool moveLinesUp);
    int minIndentCount(int fromLine = -1, int toLine = -1);
    void wordInfo(const QTextCursor &cursor, QString &word, int &intKind);
    void getPositionAndAnchor(QPoint &pos, QPoint &anchor);
    PositionPair matchParentheses(const QTextCursor &cursor, bool all = false, int *foldCount = nullptr) const;
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
    int foldStart(int line, bool &folded, QString *closingSymbol = nullptr, const QString *usedParenheses = nullptr) const;
    void foldAll(bool onlyDCO = false);
    void unfoldAll();
    void jumpTo(int line, int column = 0) override;
    void setCompleter(CodeCompleter *completer);
    void clearSearchSelection() override;
    void setSearchSelectionActive(bool active) override;
    void updateSearchSelection() override;
    void findInSelection(QList<search::Result> &results) override;
    void replaceNext(const QRegularExpression &regex, const QString &replaceText, bool selectionScope) override;
    int replaceAll(FileMeta *fm, const QRegularExpression &regex, const QString &replaceText,
                   QFlags<QTextDocument::FindFlag> options, bool selectionScope) override;
    QStringList getEnabledContextActions() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

    void showLineNrContextMenu(const QPoint &pos);
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
    void extraSelSearchSelection(QList<QTextEdit::ExtraSelection>& selections) override;
    QTimer &wordDelayTimer() { return mWordDelay; }
    QPoint toolTipPos(const QPoint &mousePos) override;
    QString getToolTipText(const QPoint &pos) override;
    bool ensureUnfolded(int line) override;
    QString resolveHRef(const QString &href);
    QString getIncludeFile(int line, qsizetype &fileStart, QString &code);
    TextLinkType checkLinks(const QPoint &mousePos, bool greedy, QString *fName = nullptr) override;
    void jumpToCurrentLink(const QPoint &mousePos) override;

signals:
    void requestMarkHash(QHash<int, gams::studio::TextMark*>* marks, gams::studio::TextMark::Type filter);
    void requestMarksEmpty(bool* marksEmpty);
    void requestSyntaxKind(int position, int &intKind, int &flavor);
    void scanSyntax(QTextBlock block, QMap<int, QPair<int,int>> &blockSyntax, int pos = -1);
    void syntaxDocAt(QTextBlock block, int pos, QStringList &syntaxDoc);
    void syntaxFlagData(QTextBlock block, syntax::SyntaxFlag flag, QString &value);
    void searchFindNextPressed();
    void searchFindPrevPressed();
    void requestAdvancedActions(QList<QAction*>* actions);
    void hasHRef(const QString &href, QString &fileName);
    void jumpToHRef(const QString &href);
    void addBreakpoint(int line);
    void delBreakpoint(int line);
    void delBreakpoints(int line, bool before);
    void delAllBreakpoints();
    void getProjectHasErrors(bool *hasErrors);
    void delAllProjectErrors();

public slots:
    void clearSelection();
    void deleteSelection();
    void cutSelection();
    virtual void copySelection();
    virtual void selectAllText();
    virtual void pasteClipboard();
    void updateExtraSelections() override;
    void unfold(const QTextBlock &block) override;
    void breakpointsChanged(const SortedIntMap &bpLines, const SortedIntMap &abpLines);
    void setPausedPos(int line);

protected slots:
    void marksChanged(const QSet<int> &dirtyLines = QSet<int>()) override;

private slots:
    void blockCountHasChanged(int newBlockCount);
    void updateLineNumberAreaWidth(/*int newBlockCount*/);
    void recalcExtraSelections();
    void startCompleterTimer();
    void updateCompleter();
    void updateLineNumberArea(const QRect &, int);
    void blockEditBlink();
    void checkBlockInsertion();
    void undoCommandAdded();
    void switchCurrentFolding();

private:
    friend class BlockEdit;
    friend class LineNumberArea;

    void adjustIndent(QTextCursor cursor);
    void truncate(const QTextBlock &block);
    int textCursorColumn(QPoint mousePos);
    void startBlockEdit(int blockNr, int colNr);
    void endBlockEdit(bool adjustCursor = true);
    QStringList clipboard(bool* isBlock = nullptr); // on relevant Block-Edit data returns multiple strings
    CharType charType(QChar c);
    bool hasLineComment(const QTextBlock &startBlock, int lastBlockNr);
    void applyLineComment(QTextCursor cursor, const QTextBlock &startBlock, int lastBlockNr);
    void checkCompleterAutoOpen();
    bool prepareCompleter();
    void showCompleter();
    QStringList tabsToSpaces(const QStringList &source, int indent, int tabSize);

    static int findAlphaNum(const QString &text, int start, bool back);
    void rawKeyPressEvent(QKeyEvent *e);
    void updateBlockEditPos();
    void updateLinkAppearance(QPoint pos, bool active = true);
    bool allowClosing(qsizetype chIndex);
    bool toggleFolding(QTextBlock block);
    LinePair findFoldBlock(int line, bool onlyThisLine = false) const override;
    QTextBlock findFoldStart(QTextBlock block) const;
    bool unfoldBadBlock(QTextBlock block);
    void checkCursorAfterFolding();
    BreakpointType breakpointType(int line);

protected:
    class BlockEdit
    {
    public:
        BlockEdit(CodeEdit* edit, int blockNr, int colNr);
        BlockEdit(const BlockEdit &other);
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
        void replaceBlockText(const QString &text);
        void replaceBlockText(const QStringList &inTexts);
        void updateExtraSelections();
        static QList<QTextEdit::ExtraSelection> generateExtraSelections(CodeEdit *edit, const QRect &rect, bool isSearchSel);
        void adjustCursor();
        void selectTo(int blockNr, int colNr);
        void toEnd(bool select);
        QString blockText() const;
        QRect block() const;
        inline QList<QTextEdit::ExtraSelection> extraSelections() const { return mSelections; }
        void selectionToClipboard();
        int startLine() const;
        int currentLine() const;
        int column() const;
        void setColumn(int column);
        void setOverwriteMode(bool overwrite);
        bool overwriteMode() const;
        int size() const;
        void setSize(int size);
        void shiftVertical(int offset);
        void checkHorizontalScroll();
        void setBlockSelectState(BlockSelectState state);
        BlockSelectState blockSelectState();
        void normalizeSelDirection();

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
        bool mIsSearchSelection = false;
        bool mTopFocus = false; // true: moving the cursor keeps the top line of BlockEdit in view
    };
    class BlockEditCursorWatch
    {
        int mColFrom = 0;
        BlockEdit *mBlockEdit;
    public:
        BlockEditCursorWatch(BlockEdit *bEdit, BlockSelectState state) : mBlockEdit(bEdit) {
            mColFrom = mBlockEdit->colFrom();
            mBlockEdit->setBlockSelectState(state);
        }
        ~BlockEditCursorWatch() {
            mBlockEdit->setBlockSelectState(bsNone);
            mBlockEdit->checkHorizontalScroll();
        }
    };

protected:
    BlockEdit* blockEdit() {return mBlockEdit;}
    void scrollContentsBy(int dx, int dy) override;

private:
    LineNumberArea *mLineNumberArea;
    CodeCompleter *mCompleter = nullptr;
    QTimer mCompleterTimer;
    int mCurrentCol;
    QTimer mCursorTimer;
    QPoint mDragStart;
    BlockEdit* mBlockEdit = nullptr;
    BlockEdit* mBlockEditSelection = nullptr;
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
    BlockSelectState mBlockSelectState = bsNone;
    SortedIntMap mBreakpoints;
    SortedIntMap mAimedBreakpoints;
    int mBreakLine = -1;
    QTextCursor mPreDebugCursor;

    static QRegularExpression mRex0LeadingSpaces;
    static QRegularExpression mRex1LeadingSpaces;
    static QRegularExpression mRexIncFile;
    static QRegularExpression mRexTruncate;
    static QRegularExpression mRexIndent;
    static QRegularExpression mRexIndentPre;
    static QRegularExpression mRexWordStart;
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
