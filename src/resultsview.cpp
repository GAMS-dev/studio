/* This file is part of the GAMS Studio project.
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
#include "resultsview.h"
#include "search/searchdialog.h"
#include "ui_resultsview.h"
#include "exception.h"
#include "search/searchresultlist.h"
#include "search/result.h"

namespace gams {
namespace studio {

ResultsView::ResultsView(SearchResultList &resultList, MainWindow *parent) :
    QWidget(parent), ui(new Ui::ResultsView), mMain(parent), mResultList(resultList)
{
    ui->setupUi(this);
    ui->tableView->setModel(&mResultList);
    searchTermLength = resultList.searchTerm().length();
}

ResultsView::~ResultsView()
{
    delete ui;
}

void ResultsView::resizeColumnsToContent()
{
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void ResultsView::on_tableView_doubleClicked(const QModelIndex &index)
{
    int selectedRow = index.row();
    Result item = mResultList.resultList().at(selectedRow);

    // open so we have a document of the file
    if (QFileInfo(item.filepath()).exists())
        mMain->openFilePath(item.filepath());

    ProjectFileNode *node = mMain->projectRepo()->findFile(item.filepath());
    if (!node) EXCEPT() << "File not found: " << item.filepath();

    // jump to line
    QTextCursor tc(node->document());
    if (item.colNr() <= 0)
        tc.setPosition(node->document()->findBlockByNumber(item.lineNr() - 1).position());
    else
        tc.setPosition(node->document()->findBlockByNumber(item.lineNr() - 1).position()
                       + item.colNr());

    node->file()->jumpTo(node->runGroupId(), true, item.lineNr()-1, item.colNr());
}

}
}

