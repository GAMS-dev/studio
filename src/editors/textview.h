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
#include <QAbstractScrollArea>
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
    void loadFile(const QString &fileName, QList<int> codecMibs);
    qint64 fileSize() const;
    int lineCount() const;
    int knownLines() const;
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    QPoint position() const;
    QPoint anchor() const;
//    int findLine(int lineNr);
    void copySelection();
    void selectAllText();

signals:
    void blockCountChanged(int newBlockCount);
    void loadAmountChanged();
    void selectionChanged();

private slots:
    void editScrollChanged();
    void peekMoreLines();
    void outerScrollAction(int action);
    void adjustOuterScrollAction();
    void editKeyPressEvent(QKeyEvent *event);
    void handleSelectionChange();
    void updatePosAndAnchor();

protected:
    friend class FileMeta;
    void setMarks(const LineMarks *marks);
    const LineMarks* marks() const;

    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

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
