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
#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H

#include <QTableWidget>
#include <QWidget>
#include "mainwindow.h"
#include "searchresultlist.h"

namespace Ui {
class ResultsView;
}

namespace gams {
namespace studio {

class Result;
class ResultsView : public QWidget
{
    Q_OBJECT

public:
    explicit ResultsView(SearchResultList resultList, MainWindow *parent = 0);
    ~ResultsView();
    void resizeColumnsToContent();

private slots:
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

private:
    Ui::ResultsView *ui;
    MainWindow *mMain;
    SearchResultList mResultList;
};

}
}

#endif // RESULTSVIEW_H
