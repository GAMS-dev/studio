#include "fileeventhandler.h"
#include "mainwindow.h"
#include "viewhelper.h"

#include <QMessageBox>
#include <QPushButton>

namespace gams {
namespace studio {

FileEventHandler::FileEventHandler(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , mMainWindow(mainWindow)
    , mMessageBox(new QMessageBox(mMainWindow))
{
    connect(mMessageBox.data(), &QMessageBox::finished,
            this, &FileEventHandler::messageBoxFinished);
}

void FileEventHandler::process(Type type, const QVector<FileEventData> &events)
{
    mMainWindow->appendSystemLogInfo("File Events to process: " + QString::number(events.count()));
    if (mMessageBox->isVisible()) {
        for (auto event: events) {
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
    for (auto event: events) {
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
    mMainWindow->appendSystemLogInfo("File Events after filter(): " + QString::number(mCurrentEvents.count()));
    return !mCurrentEvents.isEmpty();
}

void FileEventHandler::process()
{
    if (mCurrentEvents.isEmpty()) return;
    mCurrentFile = mMainWindow->fileRepo()->fileMeta(mCurrentEvents.first().fileId);
    if (!mCurrentFile) return;

    switch (mCurrentType) {
        case Change:
            mMainWindow->appendSystemLogInfo("Process Change");
            openMessageBox(QDir::toNativeSeparators(mCurrentFile->location()),
                           false, mCurrentFile->isModified(), mCurrentEvents.size());
            break;
        case Deletion:
            mMainWindow->appendSystemLogInfo("Process Deletion");
            if (mCurrentFile->exists(true))
                return;
            openMessageBox(QDir::toNativeSeparators(mCurrentFile->location()),
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

    if (result == 0) { // reload
        mOpenMessageBox = true;
        reloadFirstChangedFile();
    } else if (result == 2) { // reload all
        mOpenMessageBox = false;
        reloadAllChangedFiles();
    } else if (result == 1) { // keep
        mOpenMessageBox = true;
        keepFirstChangedFile();
    } else if (result == 3) { // keep all
        mOpenMessageBox = false;
        keepAllChangedFiles();
    }
}

void FileEventHandler::processDeletion(int result)
{
    auto queued = mQueuedEvents.take(Deletion);
    mCurrentEvents.append(queued.values().toVector());

    if (result == 0) { // close
        mOpenMessageBox = true;
        closeFirstDeletedFile();
    } else if (result == 2) { // close all
        mOpenMessageBox = false;
        closeAllDeletedFiles();
    } else if (result == 1) { // keep
        mOpenMessageBox = true;
        keepFirstDeletedFile();
    } else if (result == 3) { // keep all
        mOpenMessageBox = false;
        keepAllDeletedFiles();
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

void FileEventHandler::reloadAllChangedFiles()
{
    for (int i=mCurrentEvents.size(); i>0; --i)
        reloadFirstChangedFile();
}

void FileEventHandler::reloadFirstChangedFile()
{
    if (mCurrentEvents.isEmpty())
        return;
    auto event = mCurrentEvents.takeFirst();
    reloadFile(mMainWindow->fileRepo()->fileMeta(event.fileId));
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
    if (file->kind() == FileKind::Opt) {
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
    if (file->kind() == FileKind::Opt) {
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
        QVector<ProjectFileNode*> nodes = mMainWindow->projectRepo()->fileNodes(file->id());
        for (ProjectFileNode* node: nodes) {
            ProjectGroupNode *group = node->parentNode();
            mMainWindow->projectRepo()->closeNode(node);
            if (group->childCount() == 0)
                mMainWindow->closeGroup(group);
        }
        mMainWindow->clearHistory(file);
        mMainWindow->historyChanged();
        return;
    }

    if (file->exists(true)) return;

    mMainWindow->closeFileEditors(file->id());
    mMainWindow->clearHistory(file);
    mMainWindow->historyChanged();
}

void FileEventHandler::openMessageBox(QString filePath, bool deleted, bool modified, int count)
{
    mMessageBox->setWindowTitle(QString("File %1").arg(deleted ? "vanished" : "changed"));
    QString text(filePath + (deleted ? "%1 doesn't exist anymore."
                                     : (count>1 ? "%1 have been modified externally."
                                                : "%1 has been modified externally.")));
    text = text.arg(count<2? "" : QString(" and %1 other file%2").arg(count-1).arg(count<3? "" : "s"));
    text += "\nDo you want to %1?";
    if (deleted) text = text.arg("keep the file in editor");
    else if (modified) text = text.arg("reload the file or keep your changes");
    else text = text.arg("reload the file");
    mMessageBox->setText(text);

    // The button roles define their position. To keep them in order they all get the same value
    for (auto* button: mMessageBox->buttons())
        mMessageBox->removeButton(button);
    mMessageBox->setDefaultButton(mMessageBox->addButton(deleted ? "Close" : "Reload", QMessageBox::AcceptRole));
    mMessageBox->setEscapeButton(mMessageBox->addButton("Keep", QMessageBox::AcceptRole));
    if (count > 1) {
        mMessageBox->addButton(mMessageBox->buttonText(0) + " all", QMessageBox::AcceptRole);
        mMessageBox->addButton(mMessageBox->buttonText(1) + " all", QMessageBox::AcceptRole);
    }
    // close: 0, keep: 1, close all: 2, keep all: 3
    mMessageBox->open();
}

}
}
