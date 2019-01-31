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
#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "textmapper.h"
#include "syntax/textmarkrepo.h"
#include "editors/abstractedit.h"
#include <QAbstractScrollArea>
#include <QPlainTextEdit>
#include <QStringBuilder>
#include <QScrollBar>
#include <QTimer>

namespace gams {
namespace studio {

class TextViewEdit;

class TextView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit TextView(QWidget *parent = nullptr);
    void loadFile(const QString &fileName, int codecMib);
    void closeFile();
    qint64 fileSize() const;
    int lineCount() const;
    int knownLines() const;
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    bool jumpTo(int lineNr, int charNr);
    QPoint position() const;
    QPoint anchor() const;
    bool hasSelection() const;
//    int findLine(int lineNr);
    void copySelection();
    QString selectedText() const;
    void selectAllText();
    AbstractEdit *edit();
    void setLineWrapMode(QPlainTextEdit::LineWrapMode mode);
    bool findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind);
    bool findText(QRegularExpression seachRegex, QTextDocument::FindFlags flags);

signals:
    void blockCountChanged(int newBlockCount);
    void loadAmountChanged(int knownLineCount);
    void selectionChanged();
    void searchFindNextPressed();
    void searchFindPrevPressed();
//    void toggleBookmark(FileId fileId, NodeId groupId, int lineNr, int posInLine);
//    void jumpToNextBookmark(bool back, FileId refFileId, NodeId refGroupId, int refLineNr);
//    void cursorPositionChanged();

public slots:
    void updateExtraSelections();

private slots:
    void editScrollChanged();
    void peekMoreLines();
    void outerScrollAction(int action);
    void adjustOuterScrollAction();
    void editKeyPressEvent(QKeyEvent *event);
    void handleSelectionChange();
    void updatePosAndAnchor();

protected slots:
    void marksChanged();

protected:
    friend class FileMeta;
    void setMarks(const LineMarks *marks);
    const LineMarks* marks() const;

    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    inline FileId fileId() {
        bool ok;
        FileId file = property("fileId").toInt(&ok);
        return ok ? file : FileId();
    }
    inline NodeId groupId() {
        bool ok;
        NodeId group = property("groupId").toInt(&ok);
        return ok ? group : NodeId();
    }

private:
    void init();
    void updateVScrollZone();
    void syncVScroll();
    void topLineMoved();

private:
    int mTopLine = 0;
    int mTopVisibleLine = 0;
    int mVisibleLines = 0;
    const int mDocChanging = 0;
    bool mInit = true;

    TextMapper mMapper;
    TextViewEdit *mEdit;
    QTimer mPeekTimer;
    QTextCodec *mCodec = nullptr;
    int mLineToFind = -1;
    int mTopBufferLines = 100;
    QScrollBar::SliderAction mActiveScrollAction = QScrollBar::SliderNoAction;
    LineMarks *mMarks = nullptr;

private:

    class ChangeKeeper { //
        int &changeCounter;
    public:
        ChangeKeeper(const int &_changeCounter) : changeCounter(const_cast<int&>(_changeCounter)) {++changeCounter;}
        ~ChangeKeeper() {--changeCounter;}
    };

};

} // namespace studio
} // namespace gams

#endif // TEXTVIEW_H
