/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
 */
#include "recentdata.h"
#include "mainwindow.h"
#include "settings.h"
#include "viewhelper.h"
#include "exception.h"

namespace gams {
namespace studio {

RecentData::RecentData()
{
}

void RecentData::init(MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
    mEditFileId = -1;
    mPath = Settings::settings()->toString(skDefaultWorkspace);
}

void RecentData::setEditor(QWidget *edit)
{
    if (!mMainWindow) EXCEPT() << "Warning: RecentData isn't initialized";

    if (QWidget *lastEdit = editor()) {
        if (option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(lastEdit)) {
            MainWindow::disconnect(soEdit, &option::SolverOptionWidget::itemCountChanged, mMainWindow, &MainWindow::updateStatusLineCount );
        } else {
            if (AbstractEdit* aEdit = ViewHelper::toAbstractEdit(lastEdit)) {
                MainWindow::disconnect(aEdit, &AbstractEdit::cursorPositionChanged, mMainWindow, &MainWindow::updateStatusPos);
                MainWindow::disconnect(aEdit, &AbstractEdit::selectionChanged, mMainWindow, &MainWindow::updateStatusPos);
                MainWindow::disconnect(aEdit, &AbstractEdit::blockCountChanged, mMainWindow, &MainWindow::updateStatusLineCount);
                MainWindow::disconnect(aEdit->document(), &QTextDocument::contentsChange, mMainWindow, &MainWindow::currentDocumentChanged);
            }
            if (TextView* tv = ViewHelper::toTextView(lastEdit)) {
                MainWindow::disconnect(tv, &TextView::selectionChanged, mMainWindow, &MainWindow::updateStatusPos);
                MainWindow::disconnect(tv, &TextView::blockCountChanged, mMainWindow, &MainWindow::updateStatusLineCount);
                MainWindow::disconnect(tv, &TextView::loadAmountChanged, mMainWindow, &MainWindow::updateStatusLoadAmount);
                mMainWindow->resetLoadAmount();
            }
        }
    }

    mEditList.removeAll(edit);
    mEditList << edit;

    if (PExFileNode* node = mMainWindow->projectRepo()->findFileNode(edit)) {
        mEditFileId = node->file()->id();
        mPath = QFileInfo(node->location()).path();
    } else {
        mEditFileId = ViewHelper::fileId(edit);
        if (!mEditFileId.isValid())
            edit = nullptr;
        mPath = Settings::settings()->toString(skDefaultWorkspace);
    }

    if (option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(edit)) {
        MainWindow::connect(soEdit, &option::SolverOptionWidget::itemCountChanged, mMainWindow, &MainWindow::updateStatusLineCount );
    } else {
        if (AbstractEdit* aEdit = ViewHelper::toAbstractEdit(edit)) {
            MainWindow::connect(aEdit, &AbstractEdit::cursorPositionChanged, mMainWindow, &MainWindow::updateStatusPos);
            MainWindow::connect(aEdit, &AbstractEdit::selectionChanged, mMainWindow, &MainWindow::updateStatusPos);
            MainWindow::connect(aEdit, &AbstractEdit::blockCountChanged, mMainWindow, &MainWindow::updateStatusLineCount);
            MainWindow::connect(aEdit->document(), &QTextDocument::contentsChange, mMainWindow, &MainWindow::currentDocumentChanged);
        } else if (option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(edit)) {
            MainWindow::connect(soEdit, &option::SolverOptionWidget::itemCountChanged, mMainWindow, &MainWindow::updateStatusLineCount );
        }
        if (TextView* tv = ViewHelper::toTextView(edit)) {
            MainWindow::connect(tv, &TextView::selectionChanged, mMainWindow, &MainWindow::updateStatusPos, Qt::UniqueConnection);
            MainWindow::connect(tv, &TextView::blockCountChanged, mMainWindow, &MainWindow::updateStatusLineCount, Qt::UniqueConnection);
            MainWindow::connect(tv, &TextView::loadAmountChanged, mMainWindow, &MainWindow::updateStatusLoadAmount, Qt::UniqueConnection);
        }
    }
    mMainWindow->updateStatusFile();
    mMainWindow->updateStatusLineCount();
    mMainWindow->updateStatusPos();
    mMainWindow->updateStatusMode();
    mMainWindow->updateStatusLoadAmount();
}

void RecentData::removeEditor(QWidget *edit)
{
    bool lastRemoved = !mEditList.isEmpty() && mEditList.last() == edit;
    mEditList.removeAll(edit);
    if (lastRemoved)
        setEditor(mEditList.size() > 0 ? mEditList.last() : nullptr);
}

QWidget *RecentData::editor() const
{
    return mEditList.isEmpty() ? nullptr : mEditList.last();
}

PExProjectNode *RecentData::project() const
{
    return (editor() ? mMainWindow->projectRepo()->asProject(ViewHelper::groupId(editor())) : nullptr);
}

QWidget *RecentData::persistentEditor() const
{
    for (int i = mEditList.size()-1; i >= 0; --i) {
        if (FileMeta *fm = mMainWindow->fileRepo()->fileMeta(mEditList.at(i))) {
            if (fm->kind() != FileKind::PrO) return mEditList.at(i);
        }
    }
    return nullptr;
}


} // namespace studio
} // namespace gams
