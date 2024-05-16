/* This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "searchresultmodel.h"
#include "result.h"
#include "common.h"
#include "keys.h"
#include "mainwindow.h"
#include "searchresultviewitemdelegate.h"

namespace gams {
namespace studio {
namespace search {

ResultsView::ResultsView(SearchResultModel* results, MainWindow *parent) :
    QWidget(parent), ui(new Ui::ResultsView), mMain(parent), mResultList(results)
{
    ui->setupUi(this);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setMinimumSectionSize(1);
    ui->tableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    ui->tableView->setModel(mResultList);

    QPalette palette = qApp->palette();
    palette.setColor(QPalette::Highlight, ui->tableView->palette().highlight().color());
    palette.setColor(QPalette::HighlightedText, ui->tableView->palette().highlightedText().color());
    ui->tableView->setPalette(palette);

    ui->tableView->setItemDelegateForColumn(2, new SearchResultViewItemDelegate(this));
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
    Result r = mResultList->at(selectedRow);

    mMain->searchDialog()->jumpToResult(r);

    emit updateMatchLabel(selectedRow+1, mResultList->size());
    selectItem(selectedRow);
    if (!focus) setFocus(); // focus stays in results view
}

void ResultsView::on_tableView_clicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    setFocus();
}

void ResultsView::on_tableView_doubleClicked(const QModelIndex &index)
{
    int selectedRow = index.row();
    jumpToResult(selectedRow);
}

void ResultsView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        if (!ui->tableView->selectionModel()->hasSelection())
            ui->tableView->selectRow(0);
        on_tableView_doubleClicked(ui->tableView->selectionModel()->selectedRows(0).first());
        e->accept();
    } else if (e == Hotkey::SearchFindPrev) {
        mMain->searchDialog()->on_searchPrev();
        e->accept();
    } else if (e == Hotkey::SearchFindNext) {
        mMain->searchDialog()->on_searchNext();
        e->accept();
    } else if (e->key() == Qt::Key_Up) {
        jumpToResult(selectNextItem(true), false);
        e->accept();
    } else if (e->key() == Qt::Key_Down) {
        jumpToResult(selectNextItem(), false);
        e->accept();
    }
    QWidget::keyPressEvent(e);
}

int ResultsView::selectNextItem(bool backwards)
{
    int selected = selectedItem();
    int iterator = backwards ? -1 : 1;
    if (selected == -1) selected = backwards ? mResultList->size() : 0;

    int newIndex = selected + iterator;
    if (newIndex < 0)
        newIndex = mResultList->size()-1;
    else if (newIndex > mResultList->size()-1)
        newIndex = 0;

    ui->tableView->selectRow(newIndex);
    return newIndex;
}

void ResultsView::selectItem(int index)
{
    if (index < 0) ui->tableView->clearSelection();
    else if (!mOutdated) ui->tableView->selectRow(index);
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
    ui->tableView->clearSelection();
}

bool ResultsView::isOutdated()
{
    return mOutdated;
}

}
}
}

