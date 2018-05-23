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
#include "searchwidget.h"
#include "ui_resultsview.h"
#include "exception.h"
#include "searchresultlist.h"

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

    if (QFileInfo(item.locFile()).exists())
        mMain->openFile(item.locFile());

    ProjectAbstractNode *fsc = mMain->projectRepo()->findNode(item.locFile());
    if (!fsc) EXCEPT() << "File not found: " << item.locFile();

    ProjectFileNode *jmpFc = nullptr;
    if (fsc->type() == ProjectAbstractNode::File)
        jmpFc = static_cast<ProjectFileNode*>(fsc);

    if (!jmpFc) EXCEPT() << "Not a file:" << item.locFile();

    // open and highlight
    mMain->searchWidget()->findInFile(jmpFc, true);

    // jump to line
    QTextCursor tc(jmpFc->document());
    if (item.locCol() <= 0) {
        tc.setPosition(jmpFc->document()->findBlockByNumber(item.locLineNr() - 1).position());
    } else {
        tc.setPosition(jmpFc->document()->findBlockByNumber(item.locLineNr() - 1).position()
                       + item.locCol());

    }
    jmpFc->jumpTo(tc, false);
    jmpFc->editors().first()->setFocus();
}

}
}

