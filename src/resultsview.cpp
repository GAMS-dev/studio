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

ResultsView::ResultsView(SearchResultList* resultList, MainWindow *parent) :
    QWidget(parent), ui(new Ui::ResultsView), mMain(parent)
{
    ui->setupUi(this);
    resultList->resultHash();
    // create copy of hash to decouple cached and displayed results

    mResultList = resultList;
    ui->tableView->setModel(mResultList);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*1.4));
    ui->tableView->setTextElideMode(Qt::ElideLeft);
}

ResultsView::~ResultsView()
{
    delete ui;
}

void ResultsView::resizeColumnsToContent()
{
    ui->tableView->setColumnWidth(0, this->width()/3);
    ui->tableView->resizeColumnToContents(1);
}

void ResultsView::on_tableView_doubleClicked(const QModelIndex &index)
{
    int selectedRow = index.row();
    Result r = mResultList->at(selectedRow);

    // open so we have a document of the file
    if (QFileInfo(r.filepath()).exists())
        mMain->openFilePath(r.filepath());

    ProjectFileNode *node = mMain->projectRepo()->findFile(r.filepath());
    if (!node) EXCEPT() << "File not found: " << r.filepath();

    // jump to line
    node->file()->jumpTo(node->runGroupId(), true, r.lineNr()-1, qMax(r.colNr(), 0), r.length());
}

SearchResultList* ResultsView::searchResultList() const
{
    return mResultList;
}

///
/// \brief ResultsView::selectNextItem selects next item in resultsview and jumps to it in file
/// \param file file where the cursor is
/// \param tc cursor to get position for jumping to next match
/// \param backwards WiP not yet implemented
/// \return returns selected row +1 to make it "human readable"
///
int ResultsView::selectNextItem(QString file, QTextCursor tc, bool backwards)
{
    int tcLine = tc.blockNumber()+1;
    int tcCol = tc.positionInBlock();

    // for large docs where we have no cursor just select next item in list
    if (tc.isNull()) {
        int newRow = ui->tableView->selectionModel()->selectedRows(0).first().row() + 1;
        ui->tableView->selectRow(newRow);
        on_tableView_doubleClicked(ui->tableView->selectionModel()->selectedRows(0).first());
        return newRow+1; // and skip all the rest
    }

    QList<Result> resultList = mResultList->resultsAsList();
    int start = 0; // first index in current file
    for (int s = 0; s < resultList.size(); s++) {
        if (resultList.at(s).filepath() == file) {
            start = s;
            break;
        }
    }

    for (int i = start; i < resultList.size(); i++) {
        if (file != resultList.at(i).filepath()) { // reset cursor if in next file
            tcLine = 0;
            tcCol = 0;
        }

        // if match is in one of the following lines
        if (tcLine <= resultList.at(i).lineNr()) {

            // check if is in same line but behind the cursor
            if (tcLine != resultList.at(i).lineNr() || tcCol <= resultList.at(i).colNr()) {
                ui->tableView->selectRow(i);
                on_tableView_doubleClicked(ui->tableView->selectionModel()->selectedRows(0).first());
                return i+1;
            }
        }
    }
    // start over when arriving here
    ui->tableView->selectRow(0);
    on_tableView_doubleClicked(ui->tableView->selectionModel()->selectedRows(0).first());
    return 1;
}

}
}

