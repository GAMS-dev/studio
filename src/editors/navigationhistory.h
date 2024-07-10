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
#ifndef NAVIGATIONHISTORY_H
#define NAVIGATIONHISTORY_H

#include "abstractedit.h"

#include <QObject>
#include <QStack>

namespace gams {
namespace studio {

enum PinKind { pkNone, pkHorizontal, pkVertical };

struct CursorHistoryItem {
    QWidget* edit = nullptr;
    int lineNr = -1;
    int col = -1;
    PinKind pinKind = pkNone;
    QString filePath = "";
};

class NavigationHistory : public QObject
{
    Q_OBJECT
public:
    NavigationHistory(QObject *parent = nullptr);

    CursorHistoryItem goBack();
    CursorHistoryItem goForward();

    void stopRecord();
    void startRecord();
    bool isRecording();

    bool canGoForward();
    bool canGoBackward();

    bool itemValid(const CursorHistoryItem &item);
    void setCurrentEdit(QWidget* edit, PinKind pinKind);
    QWidget *currentEdit() const;

signals:
    void historyChanged();

public slots:
    void receiveCursorPosChange();

private:
    const int MAX_SIZE = 1000;
    QStack<CursorHistoryItem> mHistory;

    int mStackPosition = -1;
    QWidget* mCurrentEdit = nullptr;
    PinKind mPinKind = pkNone;
    bool mHasCursor = false;
    bool mStopRecord = true;

    void insertCursorItem(int line = -1, int pos = -1);
};

} // namespace studio
} // namespace gams

#endif // NAVIGATIONHISTORY_H
