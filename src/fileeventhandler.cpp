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
#include "fileeventhandler.h"
#include "mainwindow.h"
#include "viewhelper.h"
#include "file/filechangedialog.h"

#include <QPushButton>
#include <QCheckBox>
#include <QTimer>

namespace gams {
namespace studio {

FileEventHandler::FileEventHandler(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , mMainWindow(mainWindow)
    , mDialog(new FileChangeDialog(mainWindow))
{
    connect(mDialog, &FileChangeDialog::ready, this, &FileEventHandler::messageBoxFinished);
}

void FileEventHandler::process(Type type, const QVector<FileEventData> &events)
{
    if (mDialog->isVisible()) {
        for (const auto &event: events) {
            if (mCurrentType == type && event.fileId == mCurrentFile->id())
                continue;
            mQueuedEvents[type][event.fileId] = event;
        }
        return;
    }

    mCurrentType = type;
    if (filter(events))
        process();
}

bool FileEventHandler::filter(const QVector<FileEventData> &events)
{
    if (events.isEmpty())
        return false;
    mCurrentEvents.clear();
    for (const auto &event: events) {
        auto *file = mMainWindow->fileRepo()->fileMeta(event.fileId);
        if (file->isOpen() && file->isReadOnly()) {
            switch (mCurrentType) {
                case Change:
                    reloadFile(file);
                    break;
                case Deletion:
                    removeFile(file);
                    break;
                default:
                    break;
            }
            continue;
        }
        mCurrentEvents.push_back(event);
    }
    return !mCurrentEvents.isEmpty();
}

void FileEventHandler::process()
{
    if (mCurrentEvents.isEmpty()) return;
    mCurrentFile = mMainWindow->fileRepo()->fileMeta(mCurrentEvents.first().fileId);
    if (!mCurrentFile) return;

    switch (mCurrentType) {
        case Change:
            mDialog->show(QDir::toNativeSeparators(mCurrentFile->location()),
                          false, mCurrentFile->isModified(), mCurrentEvents.size());
            break;
        case Deletion:
            if (mCurrentFile->exists(true))
                return;
            mDialog->show(QDir::toNativeSeparators(mCurrentFile->location()),
                          true, mCurrentFile->isModified(), mCurrentEvents.size());
            break;
        default:
            break;
    }
}

void FileEventHandler::processChange(int result)
{
    auto queued = mQueuedEvents.take(Change);
    mCurrentEvents.append(queued.values().toVector());

    mOpenMessageBox = !FileChangeDialog::isForAll(result);
    if (FileChangeDialog::enumResult(result) == FileChangeDialog::Result::rKeep) {
        if (FileChangeDialog::isForAll(result))
            keepAllChangedFiles();
        else keepFirstChangedFile();
    } else { // rReload, rReloadAlways
        if (FileChangeDialog::isForAll(result))
            reloadAllChangedFiles(FileChangeDialog::isAutoReload(result));
        else reloadFirstChangedFile(FileChangeDialog::isAutoReload(result));
    }
}

void FileEventHandler::processDeletion(int result)
{
    auto queued = mQueuedEvents.take(Deletion);
    mCurrentEvents.append(queued.values().toVector());

    mOpenMessageBox = !FileChangeDialog::isForAll(result);
    if (FileChangeDialog::enumResult(result) == FileChangeDialog::Result::rKeep) {
        if (FileChangeDialog::isForAll(result))
            keepAllDeletedFiles();
        else keepFirstDeletedFile();
    } else { // rClose
        if (FileChangeDialog::isForAll(result))
            closeAllDeletedFiles();
        else closeFirstDeletedFile();
    }
}

void FileEventHandler::messageBoxFinished(int result)
{
    switch (mCurrentType) {
        case Change:
            processChange(result);
            break;
        case Deletion:
            processDeletion(result);
            break;
        default:
            break;
    }

    if (mOpenMessageBox)
        process();
    if (!mQueuedEvents.isEmpty()) {
        auto type = mQueuedEvents.firstKey();
        auto queued = mQueuedEvents.take(type);
        process(type, queued.values().toVector());
    }
}

void FileEventHandler::closeAllDeletedFiles()
{
    for (int i=mCurrentEvents.size(); i>0; --i)
        closeFirstDeletedFile();
}

void FileEventHandler::closeFirstDeletedFile()
{
    if (mCurrentEvents.isEmpty())
        return;

    auto data = mCurrentEvents.takeFirst();
    FileMeta *file = mMainWindow->fileRepo()->fileMeta(data.fileId);
    removeFile(file);
}

void FileEventHandler::keepAllDeletedFiles()
{
    for (int i=mCurrentEvents.size(); i>0; --i)
        keepFirstDeletedFile();
}

void FileEventHandler::keepFirstDeletedFile()
{
    if (mCurrentEvents.isEmpty())
        return;

    auto data = mCurrentEvents.takeFirst();
    FileMeta *file = mMainWindow->fileRepo()->fileMeta(data.fileId);
    if (!file) return;
    if (file->exists(true)) return;

    file->setModified();
    mMainWindow->fileRepo()->unwatch(file);
}

void FileEventHandler::reloadAllChangedFiles(bool always)
{
    for (int i=mCurrentEvents.size(); i>0; --i)
        reloadFirstChangedFile(always);
}

void FileEventHandler::reloadFirstChangedFile(bool always)
{
    if (mCurrentEvents.isEmpty())
        return;
    auto event = mCurrentEvents.takeFirst();
    FileMeta *fm = mMainWindow->fileRepo()->fileMeta(event.fileId);
    if (always) fm->setAutoReload();
    reloadFile(fm);
}

void FileEventHandler::keepAllChangedFiles()
{
    for (int i=mCurrentEvents.size(); i>0; --i)
        keepFirstChangedFile();
}

void FileEventHandler::keepFirstChangedFile()
{
    if (mCurrentEvents.isEmpty())
        return;

    auto data = mCurrentEvents.takeFirst();
    FileMeta *file = mMainWindow->fileRepo()->fileMeta(data.fileId);
    if (!file) return;
    if (!file->isOpen()) return;
    if (file->kind() == FileKind::Log) return;
    if (file->kind() == FileKind::Gdx) {
        for (QWidget *e : file->editors()) {
            auto* gdxViewer = ViewHelper::toGdxViewer(e);
            if (gdxViewer) gdxViewer->setHasChanged(true);
        }
        return;
    }
    if (file->kind() == FileKind::Opt || file->kind() == FileKind::Pf) {
        for (QWidget *e : file->editors()) {
            auto* sow = ViewHelper::toSolverOptionEdit(e);
            if (sow) sow->setFileChangedExtern(true);
        }
    }

    file->setModified();
    mMainWindow->fileRepo()->unwatch(file);
}

void FileEventHandler::reloadFile(FileMeta *file)
{
    if (!file) return;
    if (!file->isOpen()) return;
    if (file->kind() == FileKind::Log) return;
    if (file->kind() == FileKind::Gdx) {
        for (QWidget *e : file->editors()) {
            auto* gdxViewer = ViewHelper::toGdxViewer(e);
            if (gdxViewer) gdxViewer->setHasChanged(true);
        }
        return;
    }
    if (file->kind() == FileKind::Opt || file->kind() == FileKind::Pf) {
        for (QWidget *e : file->editors()) {
            auto *sow = ViewHelper::toSolverOptionEdit(e);
            if (sow) sow->setFileChangedExtern(true);
        }
    }

    file->reloadDelayed();
    file->resetTempReloadState();
}

void FileEventHandler::removeFile(FileMeta *file)
{
    if (!file) return;
    if (file->exists(true)) return;
    mMainWindow->textMarkRepo()->removeMarks(file->id(), QSet<TextMark::Type>() << TextMark::all);
    if (!file->isOpen()) {
        QVector<PExFileNode*> nodes = mMainWindow->projectRepo()->fileNodes(file->id());
        for (PExFileNode* node: std::as_const(nodes))
            mMainWindow->projectRepo()->closeNode(node);
        mMainWindow->removeFromHistory(file->location());
        return;
    }

    if (file->exists(true)) return;

    mMainWindow->closeFileEditors(file->id());
    mMainWindow->removeFromHistory(file->location());
}

}
}
