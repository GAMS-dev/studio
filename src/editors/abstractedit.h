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
#ifndef ABSTRACTEDIT_H
#define ABSTRACTEDIT_H

#include <QPlainTextEdit>
#include "common.h"
#include "syntax/textmarkrepo.h"

namespace gams {
namespace studio {

class AbstractEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    enum EditorType { CodeEdit, ProcessLog, SystemLog };

public:
    virtual ~AbstractEdit() override;
    virtual EditorType type() = 0;
    virtual void setOverwriteMode(bool overwrite);
    virtual bool overwriteMode() const;
    void sendToggleBookmark();
    void sendJumpToNextBookmark();
    void sendJumpToPrevBookmark();

    void jumpTo(const QTextCursor &cursor);
    void jumpTo(int line, int column = 0);

    void updateGroupId();

signals:
    void requestLstTexts(NodeId groupId, const QList<TextMark*> &marks, QStringList &result);
    void toggleBookmark(FileId fileId, NodeId groupId, int lineNr, int posInLine);
    void jumpToNextBookmark(bool back, FileId refFileId, NodeId refGroupId, int refLineNr);
    void cloneBookmarkMenu(QMenu *menu);

protected slots:
    virtual void marksChanged(const QSet<int> dirtyLines = QSet<int>());

protected:
    friend class FileMeta;

    AbstractEdit(QWidget *parent);
    void showToolTip(const QList<TextMark *> marks);
    QMimeData* createMimeDataFromSelection() const override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    const QList<TextMark*> &marksAtMouse() const;
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
    virtual void setMarks(const LineMarks *marks);
    virtual const LineMarks* marks() const;
    virtual int effectiveBlockNr(const int &localBlockNr) const;

private:
    const LineMarks* mMarks = nullptr;
    QList<TextMark*> mMarksAtMouse;
    QPoint mClickPos;
    QPoint mTipPos;
};

}
}

#endif // ABSTRACTEDIT_H
