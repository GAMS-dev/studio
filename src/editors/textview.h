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
#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "abstracttextmapper.h"
#include "syntax/textmarkrepo.h"
#include "editors/abstractedit.h"
#include "editors/logparser.h"
#include <QAbstractScrollArea>
#include <QPlainTextEdit>
#include <QStringBuilder>
#include <QScrollBar>

namespace gams {
namespace studio {

class TextViewEdit;
class LogParser;

class TextView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    enum TextKind { FileText, MemoryText };

    explicit TextView(TextKind kind, QWidget *parent = nullptr);
    ~TextView() override;

    bool loadFile(const QString &fileName, int codecMib, bool initAnchor);
    TextKind kind() const;
    void prepareRun();
    void endRun();
    qint64 size() const;
    int lineCount() const;
    int knownLines() const;
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    bool jumpTo(int lineNr, int charNr, int length = 0, bool focus = false);
    QPoint position() const;                // JM: changes on Debug / pos only in regular lines
    QPoint anchor() const;                  // JM: changes on Debug / no selection
    bool hasSelection() const;              // JM: changes on Debug / no selection
    void copySelection();
    QString selectedText() const;
    void selectAllText();
    AbstractEdit *edit();
    void setLineWrapMode(QPlainTextEdit::LineWrapMode mode);
    bool findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind);
    TextKind textKind() const;
    void setLogParser(LogParser *logParser);
    LogParser *logParser() const;
    void reset();
    void setDebugMode(bool debug);
    void invalidate();
    void jumpToEnd();
    int firstErrorLine();

signals:
    void addProcessData(const QByteArray &data);
    void blockCountChanged();
    void loadAmountChanged(int knownLineCount);
    void selectionChanged();
    void searchFindNextPressed();
    void searchFindPrevPressed();
    void hasHRef(const QString &href, bool &exist);
    void jumpToHRef(const QString &href);
    void createMarks(const LogParser::MarkData &marks);
    void appendLines(const QStringList &lines);

public slots:
    void updateExtraSelections();
    void updateView();

private slots:
    void outerScrollAction(int action);
    void horizontalScrollAction(int action);
    void editKeyPressEvent(QKeyEvent *event);
    void handleSelectionChange();
    void updatePosAndAnchor();
    void findClosestLstRef(const QTextCursor &cursor);
    void updateVScrollZone();

protected slots:
    void marksChanged(const QSet<int> dirtyLines = QSet<int>());
    void recalcVisibleLines();
    void topLineMoved();

protected:
    friend class FileMeta;
    void setMarks(const LineMarks *marks);
    const LineMarks* marks() const;
    bool eventFilter(QObject *watched, QEvent *event) override;
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

private:
    TextKind mTextKind;
    const int mDocChanging = 0;
    bool mInit = true;
    int mHScrollValue = 0;

    AbstractTextMapper *mMapper = nullptr;
    TextViewEdit *mEdit;
    bool *mStayAtTail = nullptr;
    bool mSliderStartedAtTail = false;
    int mSliderMouseStart = 0;

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
