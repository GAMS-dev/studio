/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef TEXTVIEWEDIT_H
#define TEXTVIEWEDIT_H

#include "codeedit.h"
#include "chunktextmapper.h"
#include <QWidget>

namespace gams {
namespace studio {

class TextViewEdit : public CodeEdit
{
    Q_OBJECT
public:
    TextViewEdit(AbstractTextMapper &mapper, QWidget *parent = nullptr);
    bool showLineNr() const override { return false; }
    bool showFolding() const override { return false; }
    void protectWordUnderCursor(bool protect);
    bool hasSelection() const override;
    void disconnectTimers() override;
    EditorType type() const override;
    int lineCount();
    void setLineMarked(const QVector<bool> &newLineMarked);

signals:
    void keyPressed(QKeyEvent *event);
    void updatePosAndAnchor();
    void recalcVisibleLines();
    void topLineMoved();
    void findClosestLstRef(const QTextCursor &cursor);
    void selectWord(int localLine, int charFrom, int charTo);

public slots:
    void copySelection() override;
    void selectAllText() override;
    void clearSelection();
    void scrollStep();

protected:
    friend class TextView;

    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void recalcWordUnderCursor() override;
    int absoluteBlockNr(const int &localBlockNr) const override;
    int localBlockNr(const int &absoluteBlockNr) const override;
    void extraSelCurrentLine(QList<QTextEdit::ExtraSelection> &selections) override;
    void extraSelLineMarks(QList<QTextEdit::ExtraSelection>& selections) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    TextLinkType checkLinks(const QPoint &mousePos, bool greedy, QString *fName = nullptr) override;
//    bool viewportEvent(QEvent *event) override;
    QVector<int> toolTipLstNumbers(const QPoint &mousePos) override;
    void paintEvent(QPaintEvent *e) override;
    QString getToolTipText(const QPoint &pos) override;

private:
    int topVisibleLine() override;
    int scrollMs(int delta);

private:
    AbstractTextMapper &mMapper;
    QVector<bool> mLineMarked;
    Settings *mSettings;
    qint64 mTopByte = 0;
    bool mClickStart = false;
    QTimer mScrollTimer;
    int mScrollDelta = 0;
    int mSubOffset = 0;
    int mDigits = 3;
    bool mKeepWordUnderCursor = false;
};

} // namespace studio
} // namespace gams

#endif // TEXTVIEWEDIT_H
