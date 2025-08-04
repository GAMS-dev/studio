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
#include "resultsview.h"
#include "keys.h"
#include "searchdialog.h"
#include "ui_resultsview.h"
#include "searchresultmodel.h"
#include "result.h"
#include "resultitem.h"
#include "mainwindow.h"

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
    mDelegate = new SearchResultViewItemDelegate(this);
    ui->resultView->setItemDelegateForColumn(0, mDelegate);
    connect(ui->resultView, &ResultTreeView::doubleClick,
            this, &ResultsView::handleDoubleClick);
    connect(ui->resultView, &QTreeView::clicked, this, [this]{
        setFocus();
    });
    resizeColumnsToContent();
}

ResultsView::~ResultsView()
{
    delete ui;
}

bool ResultsView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheel = static_cast<QWheelEvent*>(event);
        if (wheel->modifiers() == Qt::ControlModifier) {
            wheel->angleDelta().y() > 0 ? zoomIn() : zoomOut();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ResultsView::resizeColumnsToContent()
{
    ui->resultView->resizeColumns();
}

void ResultsView::jumpToResult(bool focus)
{
    auto index = ui->resultView->currentIndex();
    auto item = static_cast<ResultItem*>(index.internalPointer());
    Result r = item->type() != ResultItem::Entry ? item->child(0)->data() : item->data();
    mMain->searchDialog()->jumpToResult(r);
    emit updateMatchLabel(item->data().logicalIndex()+1, mResultModel->resultCount());
    if (!focus) setFocus(); // focus stays in results view
}

void ResultsView::expandAll()
{
    ui->resultView->expandAll();
    resizeColumnsToContent();
}

void ResultsView::handleDoubleClick()
{
    auto index = ui->resultView->currentIndex();
    if (!index.isValid())
        return;
    auto item = static_cast<ResultItem*>(index.internalPointer());
    if (item->hasChilds()) {
        auto child = ui->resultView->indexBelow(index);
        if (child.isValid())
            selectResult(child);
    } else {
        selectResult(index);
    }
    resizeColumnsToContent();
}

void ResultsView::selectResult(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    auto item = static_cast<ResultItem*>(index.internalPointer());
    jumpToResult(item->data().logicalIndex());
}

int ResultsView::prevSibling(ResultItem *parent)
{
    int sibling = parent->row();
    if (parent->row() == 0 && parent->parent()->childs().size() > 1) {
        sibling = parent->parent()->childs().size()-1;
    } else {
        sibling = sibling-1;
    }
    return sibling;
}

int ResultsView::nextSibling()
{
    return 0;
}

void ResultsView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        auto index = ui->resultView->currentIndex();
        selectResult(index);
        e->accept();
    } else if (e == Hotkey::SearchFindPrev) {
        mMain->searchDialog()->on_searchPrev();
        e->accept();
    } else if (e == Hotkey::SearchFindNext) {
        mMain->searchDialog()->on_searchNext();
        e->accept();
    } else if (e->key() == Qt::Key_Up) {
        auto index = ui->resultView->currentIndex();
        if (index.isValid()) {
            auto item = static_cast<ResultItem*>(index.internalPointer());
            if (item->hasChilds()) {
                ui->resultView->expand(index);
                auto next = ui->resultView->model()->index(0, 0, index);
                ui->resultView->setCurrentIndex(next);
                jumpToResult(false);
            } else {
                int row = 0;
                auto parent = item->parent();
                if (!item->row())
                    row = parent->childs().size()-1;
                else
                    row = item->row()-1;
                auto next = ui->resultView->model()->sibling(row, 0, index);
                ui->resultView->setCurrentIndex(next);
                jumpToResult(false);
            }
            e->accept();
        }
    } else if (e->key() == Qt::Key_Down) {
        auto index = ui->resultView->currentIndex();
        if (index.isValid()) {
            auto item = static_cast<ResultItem*>(index.internalPointer());
            if (item->hasChilds()) {
                ui->resultView->expand(index);
                auto next = ui->resultView->model()->index(0, 0, index);
                ui->resultView->setCurrentIndex(next);
                jumpToResult(false);
            } else {
                int row = 0;
                auto parent = item->parent();
                if (item->row() == parent->childs().size()-1 && parent->childs().size() > 0)
                    row = 0;
                else
                    row = item->row()+1;
                auto next = ui->resultView->model()->sibling(row, 0, index);
                ui->resultView->setCurrentIndex(next);
                jumpToResult(false);
            }
            e->accept();
        }
    }
    QWidget::keyPressEvent(e);
}

void ResultsView::zoomIn()
{
    ui->resultView->zoomIn(ZOOM_FACTOR);
    mDelegate->setFont(ui->resultView->font());
}

void ResultsView::zoomOut()
{
    ui->resultView->zoomOut(ZOOM_FACTOR);
    mDelegate->setFont(ui->resultView->font());
}

void ResultsView::resetZoom()
{
    ui->resultView->resetZoom();
    mDelegate->setFont(ui->resultView->font());
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
