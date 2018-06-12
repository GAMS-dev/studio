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
#include "editors/codeeditor.h"
#include "ui_lxiviewer.h"
#include "logger.h"

namespace gams {
namespace studio {
namespace lxiviewer {

LxiViewer::LxiViewer(CodeEditor *codeEditor, FileMeta *fileMeta, ProjectRunGroupNode* runGroup, QWidget *parent):
    QWidget(parent),
    ui(new Ui::LxiViewer),
    mCodeEditor(codeEditor),
    mFileMeta(fileMeta),
    mRunGroup(runGroup)
{
    ui->setupUi(this);

    mLstFile = mFileMeta->location();

    ui->splitter->addWidget(mCodeEditor);

    QFileInfo info(mLstFile);
    mLxiFile = info.path() + "/" + info.baseName() + ".lxi";

    loadLxiFile();
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);

    connect(ui->lxiTreeView, &QTreeView::doubleClicked, this, &LxiViewer::jumpToLine);
    connect(mCodeEditor, &CodeEditor::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);

    if (runGroup) {
        connect(runGroup, &ProjectRunGroupNode::gamsProcessStateChanged, this, &LxiViewer::loadLxiFile);
        connect(runGroup, &ProjectRunGroupNode::gamsProcessStateChanged, this, &LxiViewer::loadLstFile);
    }



    //connect(mCodeEditor, &CodeEditor::textChanged, this, &LxiViewer::loadLxiFile);
}

LxiViewer::~LxiViewer()
{
    LxiTreeModel* oldModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
    if (oldModel)
        delete oldModel;
    delete ui;
}

CodeEditor *LxiViewer::codeEditor() const
{
    return mCodeEditor;
}

void LxiViewer::loadLxiFile()
{
    if (QProcess::NotRunning == mRunGroup->gamsProcessState()) {
        if (QFileInfo(mLxiFile).exists()) {
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
}

void LxiViewer::loadLstFile()
{
    if (QProcess::NotRunning == mRunGroup->gamsProcessState()) {
        mFileMeta->triggerLoad(QList<int>() << mFileMeta->codecMib());
    }

}

void LxiViewer::jumpToTreeItem()
{
    if (ui->splitter->widget(0)->isHidden())
        return;

    int lineNr  = mCodeEditor->textCursor().block().blockNumber();
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
        if (ui->lxiTreeView->isExpanded(modelIndex))
            return;
        modelIndex = modelIndex.model()->index(0, 0, modelIndex);
        selectedItem = static_cast<LxiTreeItem*>(modelIndex.internalPointer());
        lineNr = selectedItem->lineNr();
    }

    QTextBlock tb = mCodeEditor->document()->findBlockByNumber(lineNr);
    while (tb.isValid() && tb.text().isEmpty()) {
        tb = tb.next();
    }
    lineNr  = tb.blockNumber();
    disconnect(mCodeEditor, &CodeEditor::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);
    mFileMeta->jumpTo(mRunGroup->runFileId(), true, lineNr);
    connect(mCodeEditor, &CodeEditor::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
