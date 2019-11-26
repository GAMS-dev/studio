/* This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "searchdialog.h"
#include "ui_resultsview.h"
#include "exception.h"
#include "searchresultlist.h"
#include "result.h"
#include "common.h"

namespace gams {
namespace studio {
namespace search {

ResultsView::ResultsView(SearchResultList* resultList, MainWindow *parent) :
    QWidget(parent), ui(new Ui::ResultsView), mMain(parent), mResultList(resultList->searchRegex())
{
    ui->setupUi(this);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setMinimumSectionSize(1);
    ui->tableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    ui->tableView->setTextElideMode(Qt::ElideLeft);

    for (Result r : resultList->resultsAsList()) {
        mResultList.addResult(r.lineNr(), r.colNr(), r.length(), r.filepath(), r.context());
    }
    ui->tableView->setModel(&mResultList);

    QPalette palette;
    palette.setColor(QPalette::Highlight, ui->tableView->palette().highlight().color());
    palette.setColor(QPalette::HighlightedText, ui->tableView->palette().highlightedText().color());
    ui->tableView->setPalette(palette);
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

void ResultsView::jumpToResult(int selectedRow, bool focus)
{
    Result r = mResultList.at(selectedRow);

    // open so we have a document of the file
    if (QFileInfo(r.filepath()).exists())
        mMain->openFilePath(r.filepath());

    ProjectFileNode *node = mMain->projectRepo()->findFile(r.filepath());
    if (!node) EXCEPT() << "File not found: " << r.filepath();

    // jump to line
    node->file()->jumpTo(node->runGroupId(), true, r.lineNr()-1, qMax(r.colNr(), 0), r.length());
    emit updateMatchLabel(selectedRow+1, mResultList.size());
    if (!focus) setFocus();
}

void ResultsView::on_tableView_doubleClicked(const QModelIndex &index)
{
    int selectedRow = index.row();
    jumpToResult(selectedRow);
}

void ResultsView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        on_tableView_doubleClicked(ui->tableView->selectionModel()->selectedRows(0).first());
        e->accept();
    } else if ((e->modifiers() & Qt::ShiftModifier) && (e->key() == Qt::Key_F3)) {
        jumpToResult(selectNextItem(true), false);
    } else if (e->key() == Qt::Key_F3) {
        jumpToResult(selectNextItem(), false);
    }
    QWidget::keyPressEvent(e);
}

int ResultsView::selectNextItem(bool backwards)
{
    int selected = selectedItem();
    int iterator = backwards ? -1 : 1;
    if (selected == -1) selected = backwards ? mResultList.size() : 0;

    int newIndex = selected + iterator;
    if (newIndex < 0)
        newIndex = mResultList.size()-1;
    else if (newIndex > mResultList.size()-1)
        newIndex = 0;

    selectItem(newIndex);
    return newIndex;
}

void ResultsView::selectItem(int index)
{
    ui->tableView->selectRow(index);
}

int ResultsView::selectedItem()
{
    if (!ui->tableView->selectionModel()->hasSelection())
        return -1;
    return ui->tableView->selectionModel()->selectedRows().first().row();
}

void ResultsView::setOutdated()
{
    mOutdated = true;
}

bool ResultsView::isOutdated()
{
    return mOutdated;
}

}
}
}

