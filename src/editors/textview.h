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

#include "codeedit.h"
#include "textmapper.h"
#include "studiosettings.h"
#include "locators/settingslocator.h"
#include <QStringBuilder>
#include <QScrollBar>

namespace gams {
namespace studio {

class TextViewEdit : public CodeEdit
{
    Q_OBJECT
public:
    TextViewEdit(TextMapper &mapper, QWidget *parent = nullptr)
        : CodeEdit(parent), mMapper(mapper), mSettings(SettingsLocator::settings()) {}
    bool showLineNr() const override { return false; }

protected:
    void keyPressEvent(QKeyEvent *event) override;
//    QString lineNrText(int blockNr) override {
//        double byteNr = mTopByte + document()->findBlockByNumber(blockNr-1).position();
//        double percent = byteNr * 100 / mOversizeMapper.size;
//        QString res = QString::number(percent, 'f', mDigits) % QString(mDigits-1, '0');
//        if (percent < 1.0 || res.startsWith("100")) return ('%' + res).left(mDigits+3);
//        if (percent < 10.0) return (' ' + res).left(mDigits+3);
//        return res.left(mDigits+3);
//    }

private:
    TextMapper &mMapper;
    StudioSettings *mSettings;
    qint64 mTopByte = 0;
    int mSubOffset = 0;
    int mDigits = 3;
};


class TextView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit TextView(QWidget *parent = nullptr);
    int lineCount() const;
    void loadFile(const QString &fileName, QList<int> codecMibs);
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    void getPosAndAnchor(QPoint &pos, QPoint &anchor) const;
    int findLine(int lineNr);

signals:
    void blockCountChanged(int newBlockCount);
    void loadAmount(int percent);
    void findLineAmount(int percent);

private slots:
    void editScrollChanged();
//    void editScrollResized(int min, int max);
    void peekMoreLines();
    void outerScrollAction(int action);
    void adjustOuterScrollAction();

protected:
    void scrollContentsBy(int dx, int dy) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void init();

private:
    void updateVScrollZone();
    void syncVScroll();
    void setVisibleTop(int lineNr);

private:
    int mTopLine = 0;
    int mTopVisibleLine = 0;
    int mVisibleLines = 0;
    bool mDocChanging = false;
    bool mInit = true;

private:
    TextMapper mMapper;
    TextViewEdit *mEdit;
    QTimer mPeekTimer;
    QTextCodec *mCodec = nullptr;
    int mTransferedAmount = 0;
    int mTransferedLineAmount = 0;
    int mLineToFind = -1;
    int mTopBufferLines = 100;
    QScrollBar::SliderAction mActiveScrollAction = QScrollBar::SliderNoAction;

};

} // namespace studio
} // namespace gams

#endif // TEXTVIEW_H
