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
#include <QDir>
#include "file.h"
#include "process.h"
#include "lxiviewer.h"
#include "lxiparser.h"
#include "lxitreemodel.h"
#include "lxitreeitem.h"
#include "editors/textview.h"
#include "exception.h"
#include "ui_lxiviewer.h"
#include "file/pexgroupnode.h"

namespace gams {
namespace studio {
namespace lxiviewer {

LxiViewer::LxiViewer(TextView *textView, const QString &lstFile, QWidget *parent):
    QWidget(parent),
    ui(new Ui::LxiViewer),
    mTextView(textView),
    mLstFile(lstFile)
{
    ui->setupUi(this);

    ui->splitter->addWidget(mTextView);
    setTabOrder(ui->lxiTreeView, mTextView);

    QFileInfo info(lstFile);
    mLxiFile = info.path() + "/" + info.completeBaseName() + ".lxi";

    loadLxi();
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
    setFocusProxy(ui->lxiTreeView);

    connect(ui->lxiTreeView, &QTreeView::clicked, this, &LxiViewer::jumpToLine);
    connect(ui->lxiTreeView, &QTreeView::doubleClicked, this, &LxiViewer::jumpToLine);
    connect(mTextView, &TextView::selectionChanged, this, &LxiViewer::jumpToTreeItem);
    connect(mTextView, &TextView::scrolled, this, [this](QWidget *, int dx, int dy) {
        emit scrolled(this, dx, dy);
    });
    ui->lxiTreeView->installEventFilter(this);
    mTextView->edit()->installEventFilter(this);
}

LxiViewer::~LxiViewer()
{
    LxiTreeModel* oldModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
    if (oldModel)
        delete oldModel;
    delete ui;
}

TextView *LxiViewer::textView() const
{
    return mTextView;
}

void LxiViewer::print(QPagedPaintDevice *printer)
{
    if(!mTextView) return;
    QString text;
    QFile file(mLstFile);
    if (file.open(QFile::ReadOnly | QFile::Text)){
        text = file.readAll();
        file.close();
    }
    QTextDocument document(text);
    document.print(printer);
}

bool LxiViewer::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress && watched == ui->lxiTreeView && ui->lxiTreeView->currentIndex().isValid()) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            jumpToLine(ui->lxiTreeView->currentIndex());
            if (ui->lxiTreeView->model()->hasChildren(ui->lxiTreeView->currentIndex()))
                ui->lxiTreeView->expand(ui->lxiTreeView->currentIndex());
        }
    } else if (event->type() == QEvent::FontChange && watched == mTextView->edit()) {
        setFont(mTextView->edit()->font());

    } else if (event->type() == QEvent::Wheel && watched == ui->lxiTreeView) {
        QWheelEvent *wheelEv = static_cast<QWheelEvent*>(event);
        if (wheelEv->modifiers().testFlag(Qt::ControlModifier)) {
            qApp->sendEvent(mTextView->edit()->viewport(), wheelEv);
            return true;
        }
    }

    return false;
}

void LxiViewer::resetView()
{
    QList<int> sizes;
    sizes << ui->splitter->width() / 4;
    sizes << ui->splitter->width() - sizes.at(0);
    ui->splitter->setSizes(sizes);
}

void LxiViewer::loadLxi()
{
    if (QFileInfo::exists(mLxiFile) && QFileInfo(mLxiFile).size() > 0) {
        ui->splitter->widget(0)->show();
        LxiTreeModel* model = LxiParser::parseFile(mLxiFile);
        LxiTreeModel* oldModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
        ui->lxiTreeView->setModel(model);
        QModelIndex idx = model->index(0,0);
        if (idx.isValid()) {
            ui->lxiTreeView->setCurrentIndex(idx);
            markLine(idx);
            if (oldModel)
                delete oldModel;
        } else {
            ui->splitter->widget(0)->hide();
        }
    }
    else
        ui->splitter->widget(0)->hide();
}

void LxiViewer::jumpToTreeItem()
{
    if (ui->splitter->widget(0)->isHidden())
        return;

    int lineNr  = mTextView->position().y();
    if (lineNr < 0) return; // negative lineNr is estimated

    LxiTreeModel* lxiTreeModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
    if (!lxiTreeModel) return;
    int itemIdx = 0;

    if (lxiTreeModel->lineNrs().size() > 1 && lineNr >= lxiTreeModel->lineNrs().at(1)-1) {
        itemIdx=1;
        while (lxiTreeModel->lineNrs().size() > itemIdx && lineNr >= lxiTreeModel->lineNrs().at(itemIdx)-1)
            itemIdx++;
        itemIdx--;
    }

    LxiTreeItem* treeItem = lxiTreeModel->treeItems().at(itemIdx);
    if (!ui->lxiTreeView->isExpanded(treeItem->parentItem()->modelIndex()))
        ui->lxiTreeView->expand(treeItem->parentItem()->modelIndex());
    ui->lxiTreeView->selectionModel()->select(treeItem->modelIndex(), QItemSelectionModel::SelectCurrent);
    ui->lxiTreeView->scrollTo(treeItem->modelIndex());
    mTextView->setLineMarker(treeItem->lineNr()-1);
}

void LxiViewer::jumpToLine(const QModelIndex &modelIndex)
{
    LxiTreeItem* selectedItem = static_cast<LxiTreeItem*>(modelIndex.internalPointer());
    int lineNr = selectedItem->lineNr();

    //jump to first child for virtual nodes
    if (lineNr == -1) {
        if (!ui->lxiTreeView->isExpanded(modelIndex)) {
            QModelIndex mi = modelIndex.model()->index(0, 0, modelIndex);
            selectedItem = static_cast<LxiTreeItem*>(mi.internalPointer());
            lineNr = selectedItem->lineNr();
        }
        else
            return;
    }
    disconnect(mTextView, &TextView::selectionChanged, this, &LxiViewer::jumpToTreeItem);
    mTextView->jumpTo(lineNr-1, 0);
    connect(mTextView, &TextView::selectionChanged, this, &LxiViewer::jumpToTreeItem);
    mTextView->setLineMarker(lineNr-1);
}

void LxiViewer::markLine(const QModelIndex &modelIndex)
{
    LxiTreeItem* selectedItem = static_cast<LxiTreeItem*>(modelIndex.internalPointer());
    int lineNr = selectedItem->lineNr();
    // get first child for virtual nodes
    if (lineNr == -1) {
        if (!ui->lxiTreeView->isExpanded(modelIndex)) {
            QModelIndex mi = modelIndex.model()->index(0, 0, modelIndex);
            selectedItem = static_cast<LxiTreeItem*>(mi.internalPointer());
            lineNr = selectedItem->lineNr();
        }
        else
            return;
    }
    mTextView->setLineMarker(lineNr-1);
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
