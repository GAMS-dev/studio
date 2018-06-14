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
#include "commandlinehistory.h"

namespace gams {
namespace studio {

CommandLineHistory::CommandLineHistory(QObject* parent, int initialHistorySize)
    : QObject(parent), mHistorySize(initialHistorySize)
{
}

CommandLineHistory::CommandLineHistory(QMap<QString, QStringList> map)
{
    setAllHistory(map);
}

CommandLineHistory::~CommandLineHistory()
{
    mHistory.clear();
}

void CommandLineHistory::setHistory(QString context, QStringList history)
{
    mHistory[context] = history;
}

void CommandLineHistory::addIntoCurrentContextHistory(QString option)
{
//  also allow empty option to be added into history ?
//    if (option.simplified().isEmpty())
//        return;

    if (mHistory.contains(mCurrentContext)) {
        if (!option.simplified().isEmpty()) {
           QStringList list = mHistory[mCurrentContext].filter(option.simplified());
           if (list.size() > 0) {
               mHistory[mCurrentContext].removeOne(option.simplified());
           }

           if (mHistory[mCurrentContext].size() >= mHistorySize) {
               mHistory[mCurrentContext].removeFirst();
           }
           mHistory[mCurrentContext].append(option.simplified());
        } else {
            for (int i=0; i< mHistory[mCurrentContext].size(); ++i) {
                QString str = mHistory[mCurrentContext].at(i);
                if (str.simplified().isEmpty()) {
                    mHistory[mCurrentContext].removeAt(i);
                    break;
                }
            }
            mHistory[mCurrentContext].append("");
        }
    }
}

void CommandLineHistory::setAllHistory(QMap<QString, QStringList> opts)
{
    mHistory = opts;
}

QMap<QString, QStringList> CommandLineHistory::allHistory() const
{
    return mHistory;
}

int CommandLineHistory::getHistorySize() const
{
    return mHistorySize;
}

void CommandLineHistory::setHistorySize(int historySize)
{
    mHistorySize = historySize;
}

QStringList CommandLineHistory::getHistoryFor(QString context)
{
    setContext(context);
    return mHistory[mCurrentContext];
}

void CommandLineHistory::removeFromHistory(const QString &key)
{
    mHistory.remove(key);
}

void CommandLineHistory::setContext(QString context)
{
    mCurrentContext = context;
    if (!mHistory.contains(mCurrentContext)) {
        QStringList strList;
        mHistory[mCurrentContext] = strList;
    }
}

} // namespace studio
} // namespace gams
