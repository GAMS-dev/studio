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
#ifndef ABSTRACTEDIT_H
#define ABSTRACTEDIT_H

#include <QPlainTextEdit>
#include "common.h"
#include "syntax/textmarkrepo.h"
#include "search/result.h"
#include <QTimer>

namespace gams {
namespace studio {

class FileMeta;

struct PositionPair {
    PositionPair(int _pos = -1, int _match = -1, bool _valid = false)
        : pos(_pos), match(_match), valid(_valid) {}
    bool isNull() { return pos < 0; }
    bool operator==(const PositionPair &other) { return pos==other.pos && match==other.match && valid==other.valid; }
    bool operator!=(const PositionPair &other) { return !operator==(other); }
    int pos;
    int match;
    bool valid;
};
typedef PositionPair LinePair;

enum TextLinkType { linkNone, linkHide, linkMiss, linkMark, linkDirect };

class AbstractEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    enum EditorType { CodeEditor, ProcessLog, SystemLog, LstView };

public:
    virtual ~AbstractEdit() override;
    virtual EditorType type() const = 0;
    virtual void setOverwriteMode(bool overwrite);
    virtual bool overwriteMode() const;
    void sendToggleBookmark();
    void sendJumpToNextBookmark();
    void sendJumpToPrevBookmark();
    virtual void jumpTo(int line, int column = 0);

    void updateGroupId();
    virtual void disconnectTimers();

    bool hasSearchSelection();
    void clearSearchSelection();
    void findInSelection(QList<search::Result> &results);
    inline FileId fileId() {
        bool ok;
        FileId file = property("fileId").toInt(&ok);
        return ok ? file : FileId();
    }
    void replaceNext(QRegularExpression regex, QString replacementText, bool selectionScope);
    void updateSearchSelection();
    int replaceAll(FileMeta *fm, QRegularExpression regex, QString replaceTerm, QFlags<QTextDocument::FindFlag> options, bool selectionScope);

signals:
    void requestLstTexts(gams::studio::NodeId groupId, const QVector<int> &lstLines, QStringList &result);
    void toggleBookmark(gams::studio::FileId fileId, int lineNr, int posInLine);
    void jumpToNextBookmark(bool back, gams::studio::FileId refFileId, int refLineNr);
    void cloneBookmarkMenu(QMenu *menu);

public slots:
    virtual void updateExtraSelections();
    virtual void unfold(QTextBlock block);
    void updateTabSize(int size = 0);


protected slots:
    virtual void marksChanged(const QSet<int> dirtyLines = QSet<int>());

protected:
    friend class FileMeta;

    AbstractEdit(QWidget *parent);
    virtual QString getToolTipText(const QPoint &pos);
    void updateToolTip(const QPoint &pos, bool direct = false);
    bool isToolTipValid(QString text, const QPoint &pos);
    QMimeData* createMimeDataFromSelection() const override;
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    const QList<TextMark *> marksAtMouse() const;

    inline NodeId groupId() const {
        bool ok;
        NodeId group = property("groupId").toInt(&ok);
        return ok ? group : NodeId();
    }
    virtual void setMarks(const LineMarks *marks);
    virtual const LineMarks* marks() const;
    virtual int absoluteBlockNr(const int &localBlockNr) const;
    virtual int localBlockNr(const int &absoluteBlockNr) const;
    virtual int topVisibleLine();
    virtual void extraSelCurrentLine(QList<QTextEdit::ExtraSelection>& selections);
    virtual void extraSelMarks(QList<QTextEdit::ExtraSelection> &selections);
    virtual void extraSelLineMarks(QList<QTextEdit::ExtraSelection>& selections) { Q_UNUSED(selections) }
    virtual void extraSelSearchSelection(QList<QTextEdit::ExtraSelection>& selections);
    virtual void updateCursorShape(bool greedy);
    virtual QPoint toolTipPos(const QPoint &mousePos);
    virtual QVector<int> toolTipLstNumbers(const QPoint &pos);
    virtual LinePair findFoldBlock(int line, bool onlyThisLine = false) const;
    virtual bool ensureUnfolded(int line);
    virtual TextLinkType checkLinks(const QPoint &mousePos, bool greedy, QString *fName = nullptr);
    virtual void jumpToCurrentLink(const QPoint &mousePos);
    QPoint linkClickPos() const;
    void setLinkClickPos(const QPoint &linkClickPos);
    QTextCursor cursorForPositionCut(const QPoint &pos) const;

    QTextCursor searchSelection;

private:
    const LineMarks* mMarks = nullptr;
    QPoint mClickPos;
    QPoint mTipPos;
    QTimer mSelUpdater;
    QTimer mToolTipUpdater;

private slots:
    void internalExtraSelUpdate();
    void internalToolTipUpdate();
};

}
}

#endif // ABSTRACTEDIT_H
