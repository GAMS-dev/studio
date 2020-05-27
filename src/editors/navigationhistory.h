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
#ifndef NAVIGATIONHISTORY_H
#define NAVIGATIONHISTORY_H

#include "abstractedit.h"

#include <QObject>
#include <QStack>

namespace gams {
namespace studio {

struct CursorHistoryItem {
    QWidget* tab;
    int pos;
};

class NavigationHistory : public QObject
{
    Q_OBJECT
public:
    NavigationHistory(QObject *parent = nullptr);
    void setActiveTab(QWidget* newTab);

    CursorHistoryItem goBack();
    CursorHistoryItem goForward();
    bool itemValid(CursorHistoryItem item);

    void stopRecord();
    void reenableRecord();

public slots:
    void receiveCursorPosChange();

private:
    int MAX_SIZE = 1000;
    CursorHistoryItem mInvalidItem;
    QStack<CursorHistoryItem> mHistory;

    int mStackPosition = -1;
    QWidget* mCurrentTab = nullptr;
    AbstractEdit* mCurrentEditor = nullptr;
    bool mStopRecord = false;

    void insertCursorItem(QWidget* widget, int pos);

signals:


};

} // namespace studio
} // namespace gams

#endif // NAVIGATIONHISTORY_H
