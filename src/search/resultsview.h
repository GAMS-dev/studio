/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include <QTextCursor>
#include <QWidget>
#include "searchresultmodel.h"


namespace gams {
namespace studio {

class MainWindow;

namespace search {

namespace Ui {
class ResultsView;
}

class Result;

class ResultsView : public QWidget
{
    Q_OBJECT

public:
    explicit ResultsView(SearchResultModel* searchResultList, MainWindow *parent = nullptr);
    ~ResultsView();
    void resizeColumnsToContent();

    void selectItem(int index);
    int selectedItem();

    void setOutdated();
    bool isOutdated();

    void jumpToResult(int selectedRow, bool focus = true);

signals:
    void updateMatchLabel(int row, int max);

private slots:
    void on_tableView_clicked(const QModelIndex &index);
    void on_tableView_doubleClicked(const QModelIndex &index);

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::ResultsView *ui;
    MainWindow *mMain;
    SearchResultModel* mResultList;
    bool mOutdated = false;

private:
    int selectNextItem(bool backwards = false);
};

}
}
}

#endif // RESULTSVIEW_H
