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
#include "projectgroupnode.h"
#include "projectfilenode.h"
#include "projectlognode.h"
#include "exception.h"
#include "gamsprocess.h"
#include "commonpaths.h"
#include "logger.h"
#include "syntax.h"

namespace gams {
namespace studio {

ProjectGroupNode::ProjectGroupNode(FileId id, QString name, QString location, QString fileName)
    : ProjectAbstractNode(id, name, location, ProjectAbstractNode::FileGroup),
      mGamsProcess(new GamsProcess)
{
    if (fileName == "") return;

    // only set runInfo if it's a .gms or .inc file, otherwise find gms file and set that
    QFileInfo runnableFile(location + "/" + fileName);
    QFileInfo alternateFile(runnableFile.absolutePath() + "/" + runnableFile.baseName() + ".gms");

    // fix for .lst-as-mainfile bug
    if (FileMetrics(runnableFile).fileType() == FileType::Gms)
        mGmsFileName = runnableFile.absoluteFilePath();
    else if (alternateFile.exists())
        mGmsFileName = alternateFile.absoluteFilePath();
    else
        mGmsFileName = "";

    //mGamsProcess->setContext(this);
    connect(mGamsProcess.get(), &GamsProcess::stateChanged, this, &ProjectGroupNode::onGamsProcessStateChanged);
}

ProjectGroupNode::~ProjectGroupNode()
{
}

void ProjectGroupNode::setFlag(ContextFlag flag, bool value)
{
    if (flag == ProjectAbstractNode::cfEditMod || flag == ProjectAbstractNode::cfFileMod)
        EXCEPT() << "Can't modify flag " << (flag == ProjectAbstractNode::cfEditMod ? "cfEditMod" : "cfFileMod");
    ProjectAbstractNode::setFlag(flag, value);

    // distribute missing flag to child entries
    if (flag == (ProjectAbstractNode::cfMissing & flag)) {
        for (ProjectAbstractNode *fc: mChildList) {
            fc->setFlag(flag);
        }
    }
}

void ProjectGroupNode::unsetFlag(ContextFlag flag)
{
    if (flag == ProjectAbstractNode::cfEditMod || flag == ProjectAbstractNode::cfFileMod)
        EXCEPT() << "Can't modify flag " << (flag == ProjectAbstractNode::cfEditMod ? "cfEditMod" : "cfFileMod");
    ProjectAbstractNode::setFlag(flag, false);
}

ProjectAbstractNode* ProjectGroupNode::findNode(QString filePath)
{
    QFileInfo fi(filePath);
    for (int i = 0; i < childCount(); i++) {
        ProjectAbstractNode *child = childEntry(i);
        if (QFileInfo(child->location()) == fi)
            return child;
        if (child->type() == ProjectAbstractNode::FileGroup) {
            ProjectGroupNode *group = static_cast<ProjectGroupNode*>(child);
            ProjectAbstractNode *subChild = group->findNode(filePath);
            if (subChild) return subChild;
        }
    }
    return nullptr;
}

ProjectFileNode*ProjectGroupNode::findFile(QString filePath)
{
    ProjectAbstractNode* fsc = findNode(filePath);
    return (fsc && (fsc->type() == ProjectAbstractNode::File || fsc->type() == ProjectAbstractNode::Log))
            ? static_cast<ProjectFileNode*>(fsc) : nullptr;
}

void ProjectGroupNode::setLocation(const QString& location)
{
    Q_UNUSED(location);
    EXCEPT() << "The location of a FileGroupNode can't be changed.";
}

int ProjectGroupNode::peekIndex(const QString& location, bool *hit)
{
    if (hit) *hit = false;
    for (int i = 0; i < childCount(); ++i) {
        ProjectAbstractNode *child = childEntry(i);
        QString other = child->location();
        int comp = location.compare(other, Qt::CaseInsensitive);
        if (comp < 0) return i;
        if (comp == 0) {
            if (hit) *hit = true;
            return i;
        }
    }
    return childCount();
}

void ProjectGroupNode::insertChild(ProjectAbstractNode* child)
{
    if (!child || mChildList.contains(child)) return;
    bool hit;
    int pos = peekIndex(child->name(), &hit);
    if (hit) pos++;
    mChildList.insert(pos, child);
    if (child->type() == ProjectAbstractNode::File) {
        TextMarkList *markList = marks(child->location());
        markList->bind(static_cast<ProjectFileNode*>(child));
    }
    if (!mAttachedFiles.contains(child->location())) {
        mAttachedFiles << child->location();
    }
    if (child->testFlag(cfActive))
        setFlag(cfActive);
}

void ProjectGroupNode::removeChild(ProjectAbstractNode* child)
{
    mChildList.removeOne(child);
    detachFile(child->location());
}

void ProjectGroupNode::checkFlags()
{
    bool active = false;
    for (ProjectAbstractNode *fsc: mChildList) {
        if (fsc->testFlag(cfActive)) {
            active = true;
            break;
        }
    }
    setFlag(cfActive, active);
}

void ProjectGroupNode::setLogNode(ProjectLogNode* logNode)
{
    if (mLogNode)
        EXCEPT() << "Reset the logNode is not allowed";
    mLogNode = logNode;
    if (mLogNode) {
        TextMarkList *markList = marks(mLogNode->location());
        markList->bind(mLogNode);
    }
}

void ProjectGroupNode::updateRunState(const QProcess::ProcessState& state)
{
    Q_UNUSED(state)
    // TODO(JM) visualize if a state is running
}

TextMarkList* ProjectGroupNode::marks(const QString& fileName)
{
    if (!mMarksForFilenames.contains(fileName)) {
        TextMarkList* marks = new TextMarkList(this, fileName);
        connect(marks, &TextMarkList::getFileNode, this, &ProjectGroupNode::findOrCreateFileNode);
        mMarksForFilenames.insert(fileName, marks);
    }
    return mMarksForFilenames.value(fileName);
}

void ProjectGroupNode::removeMarks(QSet<TextMark::Type> tmTypes)
{
    QHash<QString, TextMarkList*>::iterator it;
    for (it = mMarksForFilenames.begin(); it != mMarksForFilenames.end(); ++it) {
        ProjectFileNode *file = findFile(it.key());
        if (file) {
            file->removeTextMarks(tmTypes);
        } else {
            it.value()->removeTextMarks(tmTypes);
        }
    }
}

void ProjectGroupNode::removeMarks(QString fileName, QSet<TextMark::Type> tmTypes)
{
    mMarksForFilenames.value(fileName)->removeTextMarks(tmTypes, true);
}

QString ProjectGroupNode::lastLstFile() const
{
    return mLastLstFile;
}

void ProjectGroupNode::setLastLstFile(const QString &lastLstFile)
{
    mLastLstFile = lastLstFile;
}

void ProjectGroupNode::setLstFileName(const QString &lstFileName)
{
    QFileInfo fi(lstFileName);
    if (fi.isRelative())
        mLstFileName = location() + "/" + lstFileName;
    else
        mLstFileName = lstFileName;
}

void ProjectGroupNode::dumpMarks()
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

QString ProjectGroupNode::tooltip()
{
    QString tooltip(location());
    tooltip.append("\n\nMain GMS file: ").append(QFileInfo(runnableGms()).fileName());
//    tooltip.append("\nLast output file: ").append(QFileInfo(lstFileName()).fileName());
    return tooltip;
}

void ProjectGroupNode::addRunParametersHistory(QString runParameters)
{
    if (!runParameters.simplified().isEmpty()) {
       QStringList list = mRunParametersHistory.filter(runParameters.simplified());
       if (list.size() > 0) {
           mRunParametersHistory.removeOne(runParameters.simplified());
       }
    } else {
        for (int i=0; i< mRunParametersHistory.size(); ++i) {
            QString str = mRunParametersHistory.at(i);
            if (str.simplified().isEmpty()) {
                mRunParametersHistory.removeAt(i);
                break;
            }
        }
    }
    mRunParametersHistory.append(runParameters.simplified());
}

QStringList ProjectGroupNode::getRunParametersHistory()
{
    return mRunParametersHistory;
}

void ProjectGroupNode::attachFile(const QString &filepath)
{
    if(filepath == "") return;
    QFileInfo fi(filepath);
    if(!mAttachedFiles.contains(fi)) {
        mAttachedFiles << fi;
        ProjectAbstractNode* fsc = findNode(filepath);
        if (!fsc && fi.exists()) {
            updateChildNodes();
        }

        if (runnableGms().isEmpty() && FileType::from(fi.suffix()) == FileType::Gms) {
            ProjectFileNode *gms = nullptr;
            findOrCreateFileNode(filepath, gms, this);
            setRunnableGms(gms);
        }
    }
}

void ProjectGroupNode::detachFile(const QString& filepath)
{
    QFileInfo fi(filepath);
    if (mAttachedFiles.contains(fi)) {
        ProjectAbstractNode* fsc = findNode(filepath);
        ProjectFileNode *fc = (fsc && fsc->type()==ProjectAbstractNode::File) ? static_cast<ProjectFileNode*>(fsc) : nullptr;
        if (!fc || fc->editors().isEmpty()) {
            mAttachedFiles.removeOne(fi);
        }
    }
}

typedef QPair<int, ProjectAbstractNode*> IndexedNode;

void ProjectGroupNode::updateChildNodes()
{
    QFileInfoList addList = mAttachedFiles;
    QList<IndexedNode> vanishedEntries;
    for (int i = 0; i < childCount(); ++i) {
        ProjectAbstractNode *entry = childEntry(i);
        if (entry->type() == ProjectAbstractNode::Log)
            continue;
        QFileInfo fi(entry->location());
        int pos = addList.indexOf(fi);
        if (pos >= 0) {
            // keep existing entries and remove them from addList
            addList.removeAt(pos);
            entry->unsetFlag(ProjectAbstractNode::cfMissing);
        } else {
            // prepare indicees in reverse order (highest index first)
            vanishedEntries.insert(0, IndexedNode(i, entry));
        }
    }
    // check for vanished files and directories
    for (IndexedNode childIndex: vanishedEntries) {
        ProjectAbstractNode* entry = childIndex.second;
        if (entry->testFlag(ProjectAbstractNode::cfActive)) {
            // mark active files as missing (directories recursively)
            entry->setFlag(ProjectAbstractNode::cfMissing);
            qDebug() << "Missing node: " << entry->name();
        } else {
            // inactive files can be removed (directories recursively)
            emit removeNode(entry);
        }
    }
    // add newly appeared files and directories
    for (QFileInfo fi: addList) {
        if (fi.exists())
            emit requestNode(fi.fileName(), CommonPaths::absolutFilePath(fi.filePath()), this);
    }
}

void ProjectGroupNode::jumpToFirstError(bool focus)
{
    if (!mLogNode) return;
    TextMark* textMark = mLogNode->firstErrorMark();
    if (textMark) {
        if (!textMark->textCursor().isNull()) {
            textMark->jumpToMark(focus);
            textMark->jumpToRefMark(focus);
        }
        textMark = nullptr;
    }
}

QString ProjectGroupNode::lstErrorText(int line)
{
    return mLstErrorTexts.value(line);
}

void ProjectGroupNode::setLstErrorText(int line, QString text)
{
    mLstErrorTexts.insert(line, text);
}

void ProjectGroupNode::clearLstErrorTexts()
{
    mLstErrorTexts.clear();
//    dumpMarks();
    removeMarks(QSet<TextMark::Type>() << TextMark::error << TextMark::link << TextMark::none);
//    ProjectAbstractNode *fsc = findFile(lstFileName());
//    if (fsc && fsc->type() == ProjectAbstractNode::File) {
//        ProjectFileNode *fc = static_cast<ProjectFileNode*>(fsc);
//        fc->clearMarksEnhanced();
//    }
}

bool ProjectGroupNode::hasLstErrorText(int line)
{
    return (line < 0) ? mLstErrorTexts.size() > 0 : mLstErrorTexts.contains(line);
}

void ProjectGroupNode::saveGroup()
{
    foreach (ProjectAbstractNode* child, mChildList) {
        if (child->type() == ContextType::FileGroup) {
            ProjectGroupNode *fgc = static_cast<ProjectGroupNode*>(child);
            fgc->saveGroup();
        } else if (child->type() == ContextType::File) {
            ProjectFileNode *fc = static_cast<ProjectFileNode*>(child);
            fc->save();
        }
    }
}

QString ProjectGroupNode::runnableGms()
{
    // TODO(JM) for projects the project file has to be parsed for the main runableGms
    return mGmsFileName;
}

void ProjectGroupNode::setRunnableGms(ProjectFileNode *gmsFileNode)
{
    QString location = gmsFileNode->location();

    mGmsFileName = location;

    // TODO rogo: move setting of LST to GamsArgManager
    setLstFileName(QFileInfo(location).baseName() + ".lst");
    if (logNode()) logNode()->resetLst();
}

void ProjectGroupNode::removeRunnableGms()
{
    mGmsFileName = "";
    mLstFileName = "";
}

QString ProjectGroupNode::lstFileName()
{
    return mLstFileName;
}

ProjectLogNode*ProjectGroupNode::logNode() const
{
    return mLogNode;
}

GamsProcess*ProjectGroupNode::gamsProcess()
{
    return mGamsProcess.get();
}

QProcess::ProcessState ProjectGroupNode::gamsProcessState() const
{
    return mGamsProcess ? mGamsProcess->state() : QProcess::NotRunning;
}

int ProjectGroupNode::childCount() const
{
    return mChildList.count();
}

int ProjectGroupNode::indexOf(ProjectAbstractNode* child)
{
    return mChildList.indexOf(child);
}

ProjectAbstractNode*ProjectGroupNode::childEntry(int index) const
{
    return mChildList.at(index);
}

QIcon ProjectGroupNode::icon()
{
    return QIcon::fromTheme("folder", QIcon(":/img/folder-open"));
}

void ProjectGroupNode::onGamsProcessStateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState);
    updateRunState(newState);
    emit gamsProcessStateChanged(this);
}

} // namespace studio
} // namespace gams
