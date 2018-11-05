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

namespace gams {
namespace studio {

class TextViewEdit : public CodeEdit
{
    Q_OBJECT
public:
    TextViewEdit(QWidget *parent = nullptr) : CodeEdit(parent), mSettings(SettingsLocator::settings()) {}
    void setupFileSize(const qint64 &size) { mOversizeMapper.setSize(size); }
    int lineNumberAreaWidth() override {
        int space = 0;
        if(mSettings->showLineNr())
            space = 3 + (fontMetrics().width(QLatin1Char('9')) * (mDigits+3));

        space += markCount() ? iconSize() : 0;
        return space;
    }

protected:
    QString lineNrText(int blockNr) override {
        double byteNr = mTopByte + document()->findBlockByNumber(blockNr-1).position();
        double percent = byteNr * 100 / mOversizeMapper.size;
//        int pMain = int(percent);
//        QString res = QString::number(pMain) % '.' %
//                QString::number(percent-pMain, 'f', mDigits).remove(0,2) % QString(mDigits, '0');
//        return (pMain < 10) ? (' ' + res).left(mDigits+3) : res.left(mDigits+3);
        QString res = QString::number(percent, 'f', mDigits) % QString(mDigits-1, '0');
        if (percent < 1.0 || res.startsWith("100")) return ('%' + res).left(mDigits+3);
        if (percent < 10.0) return (' ' + res).left(mDigits+3);
        return res.left(mDigits+3);
    }

private:
    StudioSettings *mSettings;
    OversizeMapper mOversizeMapper;
    qint64 mTopByte = 0;
    int mSubOffset = 0;
    int mDigits = 3;
};


class TextView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit TextView(QWidget *parent = nullptr);
    int lineCount();
    void loadFile(const QString &fileName, QList<int> codecMibs);
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);

signals:

private slots:
    void editScrollChanged();
    void editScrollResized(int min, int max);

protected:
    void scrollContentsBy(int dx, int dy) override;

private:
    void syncVScroll(int editValue = -1, int mainValue = -1);

private:
    int mTopLine = 0;

private:
    TextViewEdit *mEdit;
    TextMapper *mData = nullptr;
    QTextCodec *mCodec = nullptr;
    bool mLoading = false;
};

} // namespace studio
} // namespace gams

#endif // TEXTVIEW_H
