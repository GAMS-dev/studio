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
#include <QDir>
#include "file.h"
#include "gamsprocess.h"
#include "lxiviewer.h"
#include "lxiparser.h"
#include "lxitreemodel.h"
#include "lxitreeitem.h"
#include "editors/codeedit.h"
#include "ui_lxiviewer.h"
#include "logger.h"
#include "file/projectgroupnode.h"

namespace gams {
namespace studio {
namespace lxiviewer {

LxiViewer::LxiViewer(CodeEdit *codeEditor, const QString &lstFile, QWidget *parent):
    QWidget(parent),
    ui(new Ui::LxiViewer),
    mCodeEdit(codeEditor),
    mLstFile(lstFile)
{
    ui->setupUi(this);

    ui->splitter->addWidget(mCodeEdit);

    QFileInfo info(mLstFile);
    mLxiFile = info.path() + "/" + info.baseName() + ".lxi";

    loadLxi();
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);

    connect(ui->lxiTreeView, &QTreeView::doubleClicked, this, &LxiViewer::jumpToLine);
    connect(mCodeEdit, &CodeEdit::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);
}

LxiViewer::~LxiViewer()
{
    LxiTreeModel* oldModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
    if (oldModel)
        delete oldModel;
    delete ui;
}

CodeEdit *LxiViewer::codeEdit() const
{
    return mCodeEdit;
}

void LxiViewer::loadLxiFile(ProjectGroupNode* group)
{
    if (group && QProcess::NotRunning == group->gamsProcessState())
        loadLxi();
}

void LxiViewer::loadLxi()
{
    if (QFileInfo(mLxiFile).exists() && QFileInfo(mLxiFile).size()>0) {
        ui->splitter->widget(0)->show();
        LxiTreeModel* model = LxiParser::parseFile(QDir::toNativeSeparators(mLxiFile));
        LxiTreeModel* oldModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
        ui->lxiTreeView->setModel(model);
        if (oldModel)
            delete oldModel;
    }
    else
        ui->splitter->widget(0)->hide();
}

void LxiViewer::loadLstFile(ProjectGroupNode* group)
{
    if (group && QProcess::NotRunning == group->gamsProcessState()) {
        ProjectFileNode *fileNode = group->findFile(group->gamsProcess()->inputFile());
        fileNode->load(fileNode->codecMib());
    }
}

void LxiViewer::jumpToTreeItem()
{
    if (ui->splitter->widget(0)->isHidden())
        return;

    int lineNr  = mCodeEdit->textCursor().block().blockNumber();
    LxiTreeModel* lxiTreeModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
    if (!lxiTreeModel) return;
    int itemIdx = 0;

    if (lineNr >= lxiTreeModel->lineNrs().first()) {
        itemIdx=1;
        while (lxiTreeModel->lineNrs().size()>itemIdx && lineNr >= lxiTreeModel->lineNrs()[itemIdx])
            itemIdx++;
        itemIdx--;
    }

    LxiTreeItem* treeItem = lxiTreeModel->treeItems()[itemIdx];
    if (!ui->lxiTreeView->isExpanded(treeItem->parentItem()->modelIndex()))
        ui->lxiTreeView->expand(treeItem->parentItem()->modelIndex());
    ui->lxiTreeView->selectionModel()->select(treeItem->modelIndex(), QItemSelectionModel::SelectCurrent);
    ui->lxiTreeView->scrollTo(treeItem->modelIndex());
}

void LxiViewer::jumpToLine(QModelIndex modelIndex)
{
    LxiTreeItem* selectedItem = static_cast<LxiTreeItem*>(modelIndex.internalPointer());
    int lineNr = selectedItem->lineNr();

    //jump to first child for virtual nodes
    if (lineNr == -1) {
        if (!ui->lxiTreeView->isExpanded(modelIndex)) {
            modelIndex = modelIndex.model()->index(0, 0, modelIndex);
            selectedItem = static_cast<LxiTreeItem*>(modelIndex.internalPointer());
            lineNr = selectedItem->lineNr();
        }
        else
            return;
    }

    QTextBlock tb = mCodeEdit->document()->findBlockByNumber(lineNr);
    while (tb.isValid() && tb.text().isEmpty()) {
        tb = tb.next();
    }
    lineNr  = tb.blockNumber();
    QTextCursor cursor = mCodeEdit->textCursor();
    cursor.setPosition(tb.position());

    disconnect(mCodeEdit, &CodeEdit::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);
    //mFileNode->jumpTo(cursor, true);
    connect(mCodeEdit, &CodeEdit::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
