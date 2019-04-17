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
#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H

#include <QTableWidget>
#include <QWidget>
#include "search/searchresultlist.h"

namespace Ui {
class ResultsView;
}

namespace gams {
namespace studio {

class Result;
class MainWindow;

class ResultsView : public QWidget
{
    Q_OBJECT

public:
    explicit ResultsView(SearchResultList* resultList, MainWindow *parent = nullptr);
    ~ResultsView();
    void resizeColumnsToContent();
    SearchResultList* resultList() const;

private slots:
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::ResultsView *ui;
    MainWindow *mMain;
    SearchResultList* mResultList;
};

}
}

#endif // RESULTSVIEW_H
