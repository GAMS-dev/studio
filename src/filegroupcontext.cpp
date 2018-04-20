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
#include "filegroupcontext.h"
#include "filecontext.h"
#include "logcontext.h"
#include "exception.h"
#include "gamsprocess.h"
#include "gamspaths.h"
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

FileSystemContext* FileGroupContext::findContext(QString filePath)
{
    QFileInfo fi(filePath);
    for (int i = 0; i < childCount(); i++) {
        FileSystemContext *child = childEntry(i);
        if (QFileInfo(child->location()) == fi)
            return child;
        if (child->type() == FileSystemContext::FileGroup) {
            FileGroupContext *group = static_cast<FileGroupContext*>(child);
            FileSystemContext *subChild = group->findContext(filePath);
            if (subChild) return subChild;
        }
    }
    return nullptr;
}

FileContext*FileGroupContext::findFile(QString filePath)
{
    FileSystemContext* fsc = findContext(filePath);
    return (fsc && (fsc->type() == FileSystemContext::File || fsc->type() == FileSystemContext::Log))
            ? static_cast<FileContext*>(fsc) : nullptr;
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
    if (child->type() == FileSystemContext::File) {
        TextMarkList *markList = marks(child->location());
        markList->bind(static_cast<FileContext*>(child));
    }
    if (!mAttachedFiles.contains(child->location())) {
        mAttachedFiles << child->location();
    }
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
    if (mLogContext) {
        TextMarkList *markList = marks(mLogContext->location());
        markList->bind(mLogContext);
    }
}

void FileGroupContext::updateRunState(const QProcess::ProcessState& state)
{
    Q_UNUSED(state)
    // TODO(JM) visualize if a state is running
}

TextMarkList* FileGroupContext::marks(const QString& fileName)
{
    if (!mMarksForFilenames.contains(fileName)) {
        TextMarkList* marks = new TextMarkList(this, fileName);
        connect(marks, &TextMarkList::getFileContext, this, &FileGroupContext::findOrCreateFileContext);
        mMarksForFilenames.insert(fileName, marks);
    }
    return mMarksForFilenames.value(fileName);
}

void FileGroupContext::removeMarks(QSet<TextMark::Type> tmTypes)
{
    QHash<QString, TextMarkList*>::iterator it;
    for (it = mMarksForFilenames.begin(); it != mMarksForFilenames.end(); ++it) {
        FileContext *file = findFile(it.key());
        if (file) {
            file->removeTextMarks(tmTypes);
        } else {
            it.value()->removeTextMarks(tmTypes);
        }
    }
}

void FileGroupContext::removeMarks(QString fileName, QSet<TextMark::Type> tmTypes)
{
    mMarksForFilenames.value(fileName)->removeTextMarks(tmTypes, true);
}

void FileGroupContext::setLstFileName(const QString &lstFileName)
{
    QFileInfo fi(lstFileName);
    if (fi.isRelative())
        mLstFileName = location() + "/" + lstFileName;
    else
        mLstFileName = lstFileName;
}

void FileGroupContext::dumpMarks()
{
    foreach (QString file, mMarksForFilenames.keys()) {
        QString res = file+":\n";
        TextMarkList* list = marks(file);
        foreach (TextMark* mark, list->marks()) {
            res.append(QString("  %1\n").arg(mark->dump()));
        }
        DEB() << res;
    }
}

QString FileGroupContext::tooltip()
{
    QString tooltip(location());
    tooltip.append("\n\nMain GMS file: ").append(QFileInfo(runnableGms()).fileName());
    tooltip.append("\nLast output file: ").append(QFileInfo(lstFileName()).fileName());
    return tooltip;
}

void FileGroupContext::attachFile(const QString &filepath)
{
    if(filepath == "") return;
    QFileInfo fi(filepath);
    if(!mAttachedFiles.contains(fi)) {
        mAttachedFiles << fi;
        FileSystemContext* fsc = findContext(filepath);
        if (!fsc && fi.exists()) {
            updateChildNodes();
        }
    }
}

void FileGroupContext::detachFile(const QString& filepath)
{
    QFileInfo fi(filepath);
    if (mAttachedFiles.contains(fi)) {
        FileSystemContext* fsc = findContext(filepath);
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
        if (entry->type() == FileSystemContext::Log)
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
            emit requestNode(fi.fileName(), GAMSPaths::filePath(fi.filePath()), this);
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
//    dumpMarks();
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

void FileGroupContext::saveGroup()
{
    foreach (FileSystemContext* child, mChildList) {
        if (child->type() == ContextType::FileGroup) {
            FileGroupContext *fgc = static_cast<FileGroupContext*>(child);
            fgc->saveGroup();
        } else if (child->type() == ContextType::File) {
            FileContext *fc = static_cast<FileContext*>(child);
            fc->save();
        }
    }
}

QString FileGroupContext::runnableGms()
{
    // TODO(JM) for projects the project file has to be parsed for the main runableGms
    return QDir(location()).filePath(mGmsFileName);
}

void FileGroupContext::setRunnableGms(FileContext *gmsFileContext)
{
    mGmsFileName = gmsFileContext->location();
}

QString FileGroupContext::lstFileName()
{
    if (mLstFileName.isNull()) {
        mLstFileName = QDir(location()).filePath(name())+".lst";
    }
    return mLstFileName;
}

LogContext*FileGroupContext::logContext() const
{
    return mLogContext;
}

GamsProcess*FileGroupContext::gamsProcess()
{
    return mGamsProcess.get();
}

QProcess::ProcessState FileGroupContext::gamsProcessState() const
{
    return mGamsProcess->state();
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

FileGroupContext::FileGroupContext(FileId id, QString name, QString location, QString runInfo)
    : FileSystemContext(id, name, location, FileSystemContext::FileGroup),
      mGamsProcess(new GamsProcess)
{
    if (runInfo == "") return;

    // only set runInfo if it's a .gms file, otherwise find gms file and set that
    QFileInfo runnableFile(location + "/" + runInfo);
    QFileInfo alternateFile(runnableFile.absolutePath() + "/" + runnableFile.baseName() + ".gms");

    // fix for .lst-as-basefile bug
    if (runnableFile.suffix() == "gms") {
        mGmsFileName = runnableFile.canonicalFilePath();
    } else if (alternateFile.exists()) {
        mGmsFileName = alternateFile.fileName();
    } else {
        mGmsFileName = runnableFile.canonicalFilePath();
    }

    //mGamsProcess->setContext(this);
    connect(mGamsProcess.get(), &GamsProcess::stateChanged, this, &FileGroupContext::onGamsProcessStateChanged);
}

} // namespace studio
} // namespace gams
