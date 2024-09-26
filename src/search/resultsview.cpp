/**
 * GAMS Studio
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
#include "resultitem.h"
#include "mainwindow.h"
#include "searchresultviewitemdelegate.h"

namespace gams {
namespace studio {
namespace search {

ResultsView::ResultsView(SearchResultModel* results, MainWindow *parent)
    : QWidget(parent)
    , ui(new Ui::ResultsView)
    , mMain(parent)
    , mResultModel(results)
{
    ui->setupUi(this);

    ui->resultView->setModel(mResultModel);

    QPalette palette = qApp->palette();
    palette.setColor(QPalette::Highlight, ui->resultView->palette().highlight().color());
    palette.setColor(QPalette::HighlightedText, ui->resultView->palette().highlightedText().color());
    ui->resultView->setPalette(palette);

    // TODO AF: https://git.gams.com/devel/studio/-/issues/2619
    //ui->resultView->setItemDelegateForColumn(0, new SearchResultViewItemDelegate(this));
}

ResultsView::~ResultsView()
{
    delete ui;
}

void ResultsView::resizeColumnsToContent()
{
    ui->resultView->resizeColumns();
}

void ResultsView::jumpToResult(int selectedRow, bool focus)
{
    auto index = ui->resultView->currentIndex();
    auto item = static_cast<ResultItem*>(index.internalPointer());
    Result r;
    if (item->type() != ResultItem::Entry) {
        r = item->child(0)->data();
    } else {
        r = item->data();
    }

    mMain->searchDialog()->jumpToResult(r);

    emit updateMatchLabel(item->data().logicalIndex()+1, mResultModel->resultCount());
    selectItem(selectedRow);
    if (!focus) setFocus(); // focus stays in results view
}

void ResultsView::expandAll()
{
    ui->resultView->expandAll();
}

void ResultsView::on_resultView_clicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    setFocus();
}

void ResultsView::on_resultView_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    auto item = static_cast<ResultItem*>(index.internalPointer());
    jumpToResult(item->data().logicalIndex());
}

void ResultsView::zoomIn()
{
    ui->resultView->zoomIn(ZOOM_FACTOR);
}

void ResultsView::zoomOut()
{
    ui->resultView->zoomOut(ZOOM_FACTOR);
}

void ResultsView::resetZoom()
{
    ui->resultView->resetZoom();
}

void ResultsView::selectItem(int row)
{
    if (row < 0) {
        ui->resultView->clearSelection();
    } else if (!mOutdated) {
        auto item = mResultModel->item(row);
        if (item) {
            auto fileItem = item->parent();
            auto fileIdx = ui->resultView->model()->index(fileItem->realIndex(), 0);
            ui->resultView->expand(fileIdx);
            auto itemIdx = ui->resultView->model()->index(item->realIndex(), 0, fileIdx);
            ui->resultView->setCurrentIndex(itemIdx);
        }
    }
}

int ResultsView::selectedItem()
{
    if (!ui->resultView->selectionModel()->hasSelection())
        return -1;
    return ui->resultView->selectionModel()->selectedRows().first().row();
}

void ResultsView::setOutdated()
{
    mOutdated = true;
    ui->resultView->clearSelection();
}

bool ResultsView::isOutdated()
{
    return mOutdated;
}

}
}
}
