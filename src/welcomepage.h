/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QWidget>
#include "mainwindow.h"

namespace Ui {
class WelcomePage;
}

namespace gams {
namespace studio {

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(HistoryData *history, QWidget *parent = 0);
    void historyChanged(HistoryData *history);
    ~WelcomePage();

private slots:
    void labelLinkActivated(const QString &link);

private:
    Ui::WelcomePage *ui;

signals:
    void linkActivated(const QString &link);

};

}
}

#endif // WELCOMEPAGE_H
