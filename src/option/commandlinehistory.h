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
#ifndef COMMANDLINEHISTORY_H
#define COMMANDLINEHISTORY_H

#include <QtWidgets>

namespace gams {
namespace studio {

class CommandLineHistory : public QObject
{
    Q_OBJECT

public:
    static const int MAX_HISTORY_SIZE = 20;

    CommandLineHistory(QObject* parent, int initialHistorySize = MAX_HISTORY_SIZE);
    CommandLineHistory(QMap<QString, QStringList> map);
    ~CommandLineHistory();

    QStringList getHistoryFor(QString context);
    void setHistory(QString context, QStringList history);

    QMap<QString, QStringList> allHistory() const;
    void setAllHistory(QMap<QString, QStringList> opts);

    int getHistorySize() const;
    void setHistorySize(int historySize);
    QMap<QString, QStringList> allOptions();

public slots:
    void addIntoCurrentContextHistory(QString option);

private:
    QMap<QString, QStringList> mHistory;
    QString mCurrentContext;
    int mHistorySize;

    void setContext(QString context);
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEHISTORY_H
