/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "filegroupcontext.h"
#include "filecontext.h"
#include "logcontext.h"
#include "exception.h"
#include "gamsprocess.h"
#include "logger.h"

namespace gams {
namespace studio {

FileGroupContext::~FileGroupContext()
{
    if (mChildList.size())
        DEB() << "Group must be empty before deletion";
}

void FileGroupContext::setFlag(ContextFlag flag, bool value)
{
    if (flag == FileSystemContext::cfEditMod || flag == FileSystemContext::cfFileMod)
        EXCEPT() << "Can't modify flag " << (flag == FileSystemContext::cfEditMod ? "cfEditMod" : "cfFileMod");
    FileSystemContext::setFlag(flag, value);

    // distribute missing flag to child entries
    if (flag == FileSystemContext::cfMissing && flag) {
        for (FileSystemContext *fc: mChildList) {
            fc->setFlag(flag);
        }
    }
}

void FileGroupContext::unsetFlag(ContextFlag flag)
{
    if (flag == FileSystemContext::cfEditMod || flag == FileSystemContext::cfFileMod)
        EXCEPT() << "Can't modify flag " << (flag == FileSystemContext::cfEditMod ? "cfEditMod" : "cfFileMod");
    FileSystemContext::setFlag(flag, false);
}

FileSystemContext* FileGroupContext::findFile(QString filePath)
{
    QFileInfo fi(filePath);
    for (int i = 0; i < childCount(); i++) {
        FileSystemContext *child = childEntry(i);
        if (QFileInfo(child->location()) == fi)
            return child;
        if (child->type() == FileSystemContext::FileGroup) {
            FileGroupContext *group = static_cast<FileGroupContext*>(child);
            FileSystemContext *subChild = group->findFile(filePath);
            if (subChild) return subChild;
        }
    }
    return nullptr;
}

void FileGroupContext::setLocation(const QString& location)
{
    Q_UNUSED(location);
    EXCEPT() << "The location of a FileGroupContext can't be changed.";
}

int FileGroupContext::peekIndex(const QString& name, bool *hit)
{
    if (hit) *hit = false;
    for (int i = 0; i < childCount(); ++i) {
        FileSystemContext *child = childEntry(i);
        QString other = child->name();
        int comp = name.compare(other, Qt::CaseInsensitive);
        if (comp < 0) return i;
        if (comp == 0) {
            if (hit) *hit = true;
            return i;
        }
    }
    return childCount();
}

void FileGroupContext::insertChild(FileSystemContext* child)
{
    if (!child || mChildList.contains(child)) return;
    bool hit;
    int pos = peekIndex(child->name(), &hit);
    if (hit) pos++;
    mChildList.insert(pos, child);
    if (!mAttachedFiles.contains(child->location()))
        mAttachedFiles << child->location();
    if (child->testFlag(cfActive))
        setFlag(cfActive);
}

void FileGroupContext::removeChild(FileSystemContext* child)
{
    mChildList.removeOne(child);
    detachFile(child->location());
}

void FileGroupContext::checkFlags()
{
    bool active = false;
    for (FileSystemContext *fsc: mChildList) {
        if (fsc->testFlag(cfActive)) {
            active = true;
            break;
        }
    }
    setFlag(cfActive, active);
}

void FileGroupContext::setLogContext(LogContext* logContext)
{
    if (mLogContext)
        EXCEPT() << "Reset the log-context is not allowed";
    mLogContext = logContext;
}

void FileGroupContext::updateRunState(const QProcess::ProcessState& state)
{
    Q_UNUSED(state)
    // TODO(JM) visualize if a state is running
}

TextMarkList* FileGroupContext::marks(const QString& fileName)
{
    if (!mMarksForFilenames.contains(fileName))
        mMarksForFilenames.insert(fileName, new TextMarkList());
    return mMarksForFilenames.value(fileName);
}

void FileGroupContext::removeMarks(QSet<TextMark::Type> tmTypes)
{
    QHash<QString, TextMarkList*>::iterator it;
    for (it = mMarksForFilenames.begin(); it != mMarksForFilenames.end(); ++it)
        it.value()->removeTextMarks(tmTypes);
}

void FileGroupContext::removeMarks(QString fileName, QSet<TextMark::Type> tmTypes)
{
    mMarksForFilenames.value(fileName)->removeTextMarks(tmTypes);
}

void FileGroupContext::dumpMarks()
{
    foreach (QString file, mMarksForFilenames.keys()) {
        QString res = file+":";
        TextMarkList* list = marks(file);
        foreach (TextMark* mark, list->marks()) {
            res.append(QString(" [%1,%2]").arg(mark->line()).arg(mark->column()));
        }
        DEB() << res;
    }
}


void FileGroupContext::attachFile(const QString &filepath)
{
    if(filepath == "") return;
    QFileInfo fi(filepath);
    if(!mAttachedFiles.contains(fi)) {
        mAttachedFiles << fi;
        FileSystemContext* fsc = findFile(filepath);
        if (!fsc && fi.exists()) {
            // TODO(JM) create individual node?
            updateChildNodes();
        }
    }
}

void FileGroupContext::detachFile(const QString& filepath)
{
    QFileInfo fi(filepath);
    if (mAttachedFiles.contains(fi)) {
        FileSystemContext* fsc = findFile(filepath);
        FileContext *fc = (fsc && fsc->type()==FileSystemContext::File) ? static_cast<FileContext*>(fsc) : nullptr;
        if (!fc || fc->editors().isEmpty()) {
            mAttachedFiles.removeOne(fi);
        }
    }
}

typedef QPair<int, FileSystemContext*> IndexedFSContext;

void FileGroupContext::updateChildNodes()
{
    QFileInfoList addList = mAttachedFiles;
    QList<IndexedFSContext> vanishedEntries;
    for (int i = 0; i < childCount(); ++i) {
        FileSystemContext *entry = childEntry(i);
        if (entry->location().isEmpty())
            continue;
        QFileInfo fi(entry->location());
        int pos = addList.indexOf(fi);
        if (pos >= 0) {
            // keep existing entries and remove them from addList
            addList.removeAt(pos);
            entry->unsetFlag(FileSystemContext::cfMissing);
        } else {
            // prepare indicees in reverse order (highest index first)
            vanishedEntries.insert(0, IndexedFSContext(i, entry));
        }
    }
    // check for vanished files and directories
    for (IndexedFSContext childIndex: vanishedEntries) {
        FileSystemContext* entry = childIndex.second;
        if (entry->testFlag(FileSystemContext::cfActive)) {
            // mark active files as missing (directories recursively)
            entry->setFlag(FileSystemContext::cfMissing);
            qDebug() << "Missing node: " << entry->name();
        } else {
            // inactive files can be removed (directories recursively)
            emit removeNode(entry);
        }
    }
    // add newly appeared files and directories
    for (QFileInfo fi: addList) {
        if (fi.exists())
            emit requestNode(fi.fileName(), fi.canonicalFilePath(), this);
    }
}

void FileGroupContext::jumpToFirstError(bool focus)
{
    if (!mLogContext) return;
    TextMark* textMark = mLogContext->firstErrorMark();
    if (textMark) {
        if (!textMark->textCursor().isNull()) {
            textMark->jumpToMark(focus);
            textMark->jumpToRefMark(focus);
        }
        textMark = nullptr;
    }
}

QString FileGroupContext::lstErrorText(int line)
{
    return mLstErrorTexts.value(line);
}

void FileGroupContext::setLstErrorText(int line, QString text)
{
    mLstErrorTexts.insert(line, text);
}

void FileGroupContext::clearLstErrorTexts()
{
    mLstErrorTexts.clear();
    removeMarks(QSet<TextMark::Type>() << TextMark::error << TextMark::link << TextMark::none);
//    FileSystemContext *fsc = findFile(lstFileName());
//    if (fsc && fsc->type() == FileSystemContext::File) {
//        FileContext *fc = static_cast<FileContext*>(fsc);
//        fc->clearMarksEnhanced();
//    }
}

bool FileGroupContext::hasLstErrorText(int line)
{
    return (line < 0) ? mLstErrorTexts.size() > 0 : mLstErrorTexts.contains(line);
}

QString FileGroupContext::runableGms()
{
    // TODO(JM) for projects the project file has to be parsed for the main runableGms
    return QDir(location()).filePath(mRunInfo);
}

QString FileGroupContext::lstFileName()
{
    if (mLstFileName.isNull()) {
        mLstFileName = QDir(location()).filePath(name())+".lst";
    }
    return mLstFileName;
}

LogContext*FileGroupContext::logContext()
{
    return mLogContext;
}

GamsProcess*FileGroupContext::newGamsProcess()
{
    if (mGamsProcess)
        EXCEPT() << "Cannot create process. This group already has an active process.";
    mGamsProcess = new GamsProcess();
    mGamsProcess->setContext(this);
    connect(mGamsProcess, &GamsProcess::destroyed, this, &FileGroupContext::processDeleted);
    connect(mGamsProcess, &GamsProcess::stateChanged, this, &FileGroupContext::onGamsProcessStateChanged);
    return mGamsProcess;
}

GamsProcess*FileGroupContext::gamsProcess()
{
    return mGamsProcess;
}

QProcess::ProcessState FileGroupContext::gamsProcessState() const
{
    return mGamsProcess ? mGamsProcess->state() : QProcess::NotRunning;
}

int FileGroupContext::childCount() const
{
    return mChildList.count();
}

int FileGroupContext::indexOf(FileSystemContext* child)
{
    return mChildList.indexOf(child);
}

FileSystemContext*FileGroupContext::childEntry(int index) const
{
    return mChildList.at(index);
}

QIcon FileGroupContext::icon()
{
    return QIcon::fromTheme("folder", QIcon(":/img/folder-open"));
}

void FileGroupContext::onGamsProcessStateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState);
    updateRunState(newState);
    emit gamsProcessStateChanged(this);
}

void FileGroupContext::processDeleted()
{
    mGamsProcess = nullptr;
    updateRunState(QProcess::NotRunning);
    emit gamsProcessStateChanged(this);
}

FileGroupContext::FileGroupContext(FileId id, QString name, QString location, QString runInfo)
    : FileSystemContext(id, name, location, FileSystemContext::FileGroup)
{
    if (runInfo == "") return;

    // only set runInfo if it's a .gms file, otherwise find gms file and set that
    QFileInfo runnableFile(location + "/" + runInfo);
    QFileInfo alternateFile(runnableFile.absolutePath() + "/" + runnableFile.baseName() + ".gms");

    // fix for .lst-as-basefile bug
    if (runnableFile.suffix() == "gms") {
        mRunInfo = runInfo;
    } else if (alternateFile.exists()) {
        mRunInfo = alternateFile.fileName();
    }
}

} // namespace studio
} // namespace gams
