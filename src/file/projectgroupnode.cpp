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
#include "filemeta.h"
#include "filemetarepo.h"
#include "exception.h"
#include "gamsprocess.h"
#include "commonpaths.h"
#include "logger.h"
#include "syntax.h"
#include "file.h"
#include <QFileInfo>
#include <QDir>

namespace gams {
namespace studio {

ProjectGroupNode::ProjectGroupNode(QString name, QString location, NodeType type)
    : ProjectAbstractNode("G-"+name, type), mLocation(location)
{}

ProjectGroupNode::~ProjectGroupNode()
{
}

QIcon ProjectGroupNode::icon()
{
    return QIcon::fromTheme("folder", QIcon(":/img/folder-open"));
}

int ProjectGroupNode::childCount() const
{
    return mChildNodes.count();
}

ProjectAbstractNode*ProjectGroupNode::childNode(int index) const
{
    return mChildNodes.at(index);
}

int ProjectGroupNode::indexOf(ProjectAbstractNode* child)
{
    return mChildNodes.indexOf(child);
}

void ProjectGroupNode::insertChild(ProjectAbstractNode* child)
{
    if (!child || mChildNodes.contains(child)) return;
    mChildNodes.append(child);
    int i = mChildNodes.size()-1;
    while (i > 0 && child->name().compare(mChildNodes.at(i-1)->name(), Qt::CaseInsensitive) < 0)
        --i;
    if (i < mChildNodes.size()-1)
        mChildNodes.move(mChildNodes.size()-1, i);
// TODO(JM) set runableGms if missing

//    bool hit;
//    int pos = peekIndex(child->name(), &hit);
//    if (hit) pos++;
//    mChildList.insert(pos, child);
//    if (child->type() == ProjectAbstractNode::File) {
//        // TODO(JM) move file binding to FileMetaRepo
//        TextMarkRepo *markList = marks(child->location());
//        markList->bind(static_cast<ProjectFileNode*>(child));
//    }
//    if (!mAttachedFiles.contains(child->location())) {
//        mAttachedFiles << child->location();
//    }
//    if (child->testFlag(cfActive))
//        setFlag(cfActive);
}

void ProjectGroupNode::removeChild(ProjectAbstractNode* child)
{
    mChildNodes.removeOne(child);
}

QString ProjectGroupNode::location() const
{
    return mLocation;
}

void ProjectGroupNode::setLocation(const QString& location)
{
    mLocation = location;
}

QString ProjectGroupNode::tooltip()
{
    return QString(location());
}

QString ProjectGroupNode::lstErrorText(int line)
{
    return parentNode() ? parentNode()->lstErrorText(line) : QString();
}

ProjectAbstractNode *ProjectGroupNode::findNode(const QString &location, bool recurse) const
{
    for (ProjectAbstractNode* node: mChildNodes) {
        const ProjectFileNode* file = node->toFile();
        if (file && file->location() == location) return node;
        const ProjectGroupNode* group = node->toGroup();
        if (group) {
            if (group->location() == location) return node;
            if (recurse) {
                ProjectAbstractNode* sub = group->findNode(location, true);
                if (sub) return sub;
            }
        }
    }
    return nullptr;
}

ProjectFileNode *ProjectGroupNode::findFile(const FileMeta *fileMeta, bool recurse) const
{
    for (ProjectAbstractNode* node: mChildNodes) {
        ProjectFileNode* fileNode = node->toFile();
        if (fileNode && fileNode->file() == fileMeta) return fileNode;
        const ProjectGroupNode* group = node->toGroup();
        if (group && recurse) {
            ProjectFileNode* sub = group->findFile(fileMeta, true);
            if (sub) return sub;
        }
    }
    return nullptr;
}

ProjectFileNode *ProjectGroupNode::findOrCreateFileNode(const QString &location)
{
    ProjectAbstractNode* fn = findNode(location, false);
    if (fn) return fn->toFile();
    FileMeta* fm = projectRepo()->fileRepo()->findOrCreateFileMeta(location);
    return projectRepo()->findOrCreateFileNode(fm, this);
}

ProjectRunGroupNode *ProjectGroupNode::findRunGroup(const AbstractProcess *process) const
{
    for (ProjectAbstractNode* node: internalNodeList()) {
        ProjectRunGroupNode* runGroup = node->toRunGroup();
        if (runGroup && runGroup->isProcess(process))
            return runGroup;
        const ProjectGroupNode* group = node->toGroup();
        if (group) {
            runGroup = findRunGroup(process);
            if (runGroup) return runGroup;
        }
    }
    return nullptr;
}

ProjectRunGroupNode *ProjectGroupNode::findRunGroup(FileId runId) const
{
    for (ProjectAbstractNode* node: internalNodeList()) {
        ProjectRunGroupNode* runGroup = node->toRunGroup();
        if (runGroup && runGroup->runFileId() == runId)
            return runGroup;
        const ProjectGroupNode* group = node->toGroup();
        if (group) {
            runGroup = findRunGroup(runId);
            if (runGroup) return runGroup;
        }
    }
    return nullptr;
}

QVector<ProjectFileNode *> ProjectGroupNode::listFiles(bool recurse) const
{
    QVector<ProjectFileNode *> res;
    for (ProjectAbstractNode *node: mChildNodes) {
        ProjectFileNode *fileNode = node->toFile();
        if (fileNode)
            res << fileNode;
        else if (recurse) {
            ProjectGroupNode *sub = node->toGroup();
            if (sub) res << sub->listFiles(true);
        }
    }
    return res;
}

ProjectRunGroupNode::ProjectRunGroupNode(QString name, QString path, FileMeta* runFileMeta)
    : ProjectGroupNode(name, path, NodeType::runGroup)
    , mGamsProcess(runFileMeta ? new GamsProcess() : nullptr)
{
    if (mGamsProcess) {
        connect(mGamsProcess.get(), &GamsProcess::stateChanged, this, &ProjectRunGroupNode::onGamsProcessStateChanged);
    }
    if (path.isEmpty()) setLocation(QFileInfo(runFileMeta->location()).absoluteDir().path());
    if (runFileMeta && runFileMeta->kind() == FileKind::Gms) {
        setRunnableGms(runFileMeta);
    }
}

void ProjectRunGroupNode::updateRunState(const QProcess::ProcessState &state)
{
    Q_UNUSED(state)
    // TODO(JM) visualize if a state is running
}

GamsProcess *ProjectRunGroupNode::gamsProcess() const
{
    return mGamsProcess.get();
}

FileId ProjectRunGroupNode::runFileId() const
{
    return mGmsFile->id();
}


ProjectLogNode* ProjectRunGroupNode::logNode() const
{
    return mLogNode;
}

void ProjectRunGroupNode::setLogNode(ProjectLogNode* logNode)
{
    if (mLogNode)
        EXCEPT() << "Reset the logNode is not allowed";
    mLogNode = logNode;
}

ProjectLogNode *ProjectRunGroupNode::getOrCreateLogNode(FileMetaRepo *fileMetaRepo)
{
    if (!mLogNode) {
        QString logName = "[LOG]" + QString::number(id());
        FileMeta* fm = fileMetaRepo->findOrCreateFileMeta(logName);
        mLogNode = new ProjectLogNode(fm, this);
    }
    return mLogNode;

}

FileMeta* ProjectRunGroupNode::runnableGms() const
{
    return mGmsFile;
}

void ProjectRunGroupNode::setRunnableGms(FileMeta *gmsFile)
{
    if (!gmsFile) {
        mGmsFile = nullptr;
        mLstFileName = "";
        return;
    }
    if (gmsFile->kind() != FileKind::Gms)
        EXCEPT() << "Only files of FileKind::Gms can become runable";
    mGmsFile = gmsFile;
    QString location = gmsFile->location();
    QString lstName = QFileInfo(location).completeBaseName() + ".lst";
    setLstFileName(lstName);
    if (logNode()) logNode()->resetLst();
}

void ProjectRunGroupNode::setLstFileName(const QString &lstFileName)
{
    QFileInfo fi(lstFileName);
    if (fi.isRelative())
        mLstFileName = location() + "/" + lstFileName;
    else
        mLstFileName = lstFileName;
}

QString ProjectRunGroupNode::lstFileName() const
{
    return mLstFileName;
}

QString ProjectRunGroupNode::lstErrorText(int line)
{
    return mLstErrorTexts.value(line);
}

void ProjectRunGroupNode::setLstErrorText(int line, QString text)
{
    mLstErrorTexts.insert(line, text);
}

void ProjectRunGroupNode::clearLstErrorTexts()
{
    mLstErrorTexts.clear();
    // TODO(JM) remove marks for this groups NodeId
}

bool ProjectRunGroupNode::hasLstErrorText(int line)
{
    return (line < 0) ? mLstErrorTexts.size() > 0 : mLstErrorTexts.contains(line);
}

bool ProjectRunGroupNode::isProcess(const AbstractProcess *process) const
{
    return process && mGamsProcess.get() == process;
}

void ProjectRunGroupNode::jumpToFirstError(bool focus)
{
    if (!mLogNode) return;
    QList<TextMark*> marks = projectRepo()->textMarkRepo()->marks(mLogNode->file()->id(), -1, runFileId(), TextMark::error, 1);
    TextMark* textMark = marks.size() ? marks.first() : nullptr;
    if (textMark) {
        textMark->jumpToMark(focus);
        textMark->jumpToRefMark(focus);
    }
}

QProcess::ProcessState ProjectRunGroupNode::gamsProcessState() const
{
    return mGamsProcess->state();
}

QString ProjectRunGroupNode::tooltip()
{
    QString res(location());
    if (runnableGms()) res.append("\n\nMain GMS file: ").append(runnableGms()->name());
    if (!lstFileName().isEmpty()) res.append("\nLast output file: ").append(QFileInfo(lstFileName()).fileName());
    return res;
}

int ProjectGroupNode::peekIndex(const QString& name, bool *hit)
{
    if (hit) *hit = false;
    for (int i = 0; i < childCount(); ++i) {
        ProjectAbstractNode *child = childNode(i);
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


void ProjectRunGroupNode::onGamsProcessStateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState);
    updateRunState(newState);
    emit gamsProcessStateChanged(this);
}




ProjectRootNode::ProjectRootNode(ProjectRepo* repo)
    : ProjectGroupNode("Root", "", NodeType::root), mRepo(repo)
{
    if (!mRepo) EXCEPT() << "The ProjectRepo must not be null";
}

void ProjectRootNode::setParentNode(ProjectGroupNode *parent)
{
    Q_UNUSED(parent);
    EXCEPT() << "The root node has no parent";
}

ProjectRepo *ProjectRootNode::projectRepo() const
{
    return mRepo;
}

/*




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
        if (child->type() == NodeType::Group) {
            ProjectGroupNode *group = static_cast<ProjectGroupNode*>(child);
            ProjectAbstractNode *subChild = group->findNode(filePath);
            if (subChild) return subChild;
        }
    }
    return nullptr;
}

ProjectFileNode *ProjectGroupNode::findFile(FileId fileId)
{
    for (int i = 0; i < childCount(); i++) {
        ProjectAbstractNode *child = childEntry(i);
        if (QFileInfo(child->id()) == fileId)
            return child;
        if (child->type() == NodeType::Group) {
            ProjectGroupNode *group = static_cast<ProjectGroupNode*>(child);
            ProjectAbstractNode *subChild = group->findNode(fileId);
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

void ProjectGroupNode::updateRunState(const QProcess::ProcessState& state)
{
    Q_UNUSED(state)
    // TODO(JM) visualize if a state is running
}

TextMarkRepo *ProjectGroupNode::marks(const QString& fileName)
{
    if (!mMarksForFilenames.contains(fileName)) {
        TextMarkRepo* marks = new TextMarkRepo(this, fileName);
        // TOOD(JM) move functionality to FileMetaRepo
        connect(marks, &TextMarkRepo::getFileNode, this, &ProjectGroupNode::findOrCreateFileNode);
        mMarksForFilenames.insert(fileName, marks);
    }
    return mMarksForFilenames.value(fileName);
}

void ProjectGroupNode::removeMarks(QSet<TextMark::Type> tmTypes)
{
    QHash<QString, TextMarkRepo*>::iterator it;
    for (it = mMarksForFilenames.begin(); it != mMarksForFilenames.end(); ++it) {
        ProjectFileNode *file = findFile(it.key());
        if (file) {
            file->removeTextMarks(tmTypes);
        } else {
            // TODO(JM) move file binding to FileMetaRepo
            it.value()->removeTextMarks(tmTypes);
        }
    }
}

void ProjectGroupNode::removeMarks(QString fileName, QSet<TextMark::Type> tmTypes)
{
    // TODO(JM) move file binding to FileMetaRepo
    mMarksForFilenames.value(fileName)->removeTextMarks(tmTypes, true);
}

void ProjectGroupNode::dumpMarks()
{
    foreach (QString file, mMarksForFilenames.keys()) {
        QString res = file+":\n";
        TextMarkRepo* list = marks(file);
        foreach (TextMark* mark, list->marks()) {
            res.append(QString("  %1\n").arg(mark->dump()));
        }
        DEB() << res;
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

void ProjectGroupNode::saveGroup()
{
    foreach (ProjectAbstractNode* child, mChildList) {
        if (child->type() == NodeType::Group) {
            ProjectGroupNode *fgc = static_cast<ProjectGroupNode*>(child);
            fgc->saveGroup();
        } else if (child->type() == NodeType::File) {
            ProjectFileNode *fc = static_cast<ProjectFileNode*>(child);
            fc->save();
        }
    }
}

GamsProcess*ProjectGroupNode::gamsProcess()
{
    return mGamsProcess.get();
}

QProcess::ProcessState ProjectGroupNode::gamsProcessState() const
{
    return mGamsProcess->state();
}

*/



} // namespace studio
} // namespace gams
