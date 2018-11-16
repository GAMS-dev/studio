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
#include "projectrepo.h"
#include "filemeta.h"
#include "filemetarepo.h"
#include "exception.h"
#include "editors/systemlogedit.h"
#include "gamsprocess.h"
#include "commonpaths.h"
#include "logger.h"
#include "syntax.h"
#include "option/option.h"
#include "locators/sysloglocator.h"
#include <QFileInfo>
#include <QDir>

namespace gams {
namespace studio {

ProjectGroupNode::ProjectGroupNode(QString name, QString location, NodeType type)
    : ProjectAbstractNode(name, type), mLocation(location)
{}

ProjectGroupNode::~ProjectGroupNode()
{
    if (mChildNodes.size())
        DEB() << "Group must be empty before deletion";
}

QIcon ProjectGroupNode::icon()
{
    return QIcon::fromTheme("folder", QIcon(":/img/folder-open"));
}

int ProjectGroupNode::childCount() const
{
    return mChildNodes.count();
}

bool ProjectGroupNode::isEmpty()
{
    return (mChildNodes.count() == 0);
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
    emit changed(id());
}

QString ProjectGroupNode::tooltip()
{
    QString res = location();
    if (debugMode()) {
        res.append("\nNodeId: "+QString::number(id()));
        res.append("\nParent-NodeId: " + (parentNode() ? QString::number(parentNode()->id()) : "?"));
    }
    return QString(res);
}

QString ProjectGroupNode::lstErrorText(int line)
{
    return parentNode() ? parentNode()->lstErrorText(line) : QString();
}

ProjectFileNode *ProjectGroupNode::findFile(const QString &location, bool recurse) const
{
    QFileInfo fi(location);
    for (ProjectAbstractNode* node: mChildNodes) {
        ProjectFileNode* file = node->toFile();
        if (file && FileMetaRepo::equals(QFileInfo(file->location()), fi)) return file;
        if (recurse) {
            const ProjectGroupNode* group = node->toGroup();
            ProjectFileNode* sub = group ? group->findFile(location, true) : nullptr;
            if (sub) return sub;
        }
    }
    return nullptr;
}

ProjectFileNode *ProjectGroupNode::findFile(const FileMeta *fileMeta, bool recurse) const
{
    if (!fileMeta) return nullptr;
    if (fileMeta->kind() == FileKind::Log) return nullptr;
    for (ProjectAbstractNode* node: mChildNodes) {
        ProjectFileNode* fileNode = node->toFile();
        if (fileNode && fileNode->file() == fileMeta) return fileNode;
        if (recurse) {
            const ProjectGroupNode* group = node->toGroup();
            ProjectFileNode* sub = group ? group->findFile(fileMeta, true) : nullptr;
            if (sub) return sub;
        }
    }
    return nullptr;
}

ProjectFileNode *ProjectGroupNode::findOrCreateFileNode(const QString &location)
{
    ProjectFileNode* node = findFile(location, false);
    if (node) return node;
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
        if (runGroup && runGroup->runnableGms()->id() == runId)
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
    , mGamsProcess(new GamsProcess())
{
    connect(mGamsProcess.get(), &GamsProcess::stateChanged, this, &ProjectRunGroupNode::onGamsProcessStateChanged);
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

bool ProjectRunGroupNode::hasLogNode() const
{
    return mLogNode;
}

void ProjectRunGroupNode::setLogNode(ProjectLogNode* logNode)
{
    if (mLogNode)
        EXCEPT() << "Reset the logNode is not allowed";
    mLogNode = logNode;
}

ProjectLogNode *ProjectRunGroupNode::logNode()
{
    if (!mLogNode) {
        QString suffix = FileType::from(FileKind::Log).defaultSuffix();
        QFileInfo fi = !specialFile(FileKind::Gms).isEmpty()
                       ? specialFile(FileKind::Gms) : QFileInfo(location()+"/"+name()+"."+suffix);
        QString logName = fi.path()+"/"+fi.completeBaseName()+"."+suffix;
        FileMeta* fm = fileRepo()->findOrCreateFileMeta(logName, &FileType::from(FileKind::Log));
        mLogNode = new ProjectLogNode(fm, this);
    }
    return mLogNode;
}

FileMeta* ProjectRunGroupNode::runnableGms() const
{
    return fileRepo()->fileMeta(specialFile(FileKind::Gms));
}

void ProjectRunGroupNode::setRunnableGms(FileMeta *gmsFile)
{
    ProjectFileNode *gmsFileNode;
    if (!gmsFile) {
        // find alternative runable file
        for (ProjectAbstractNode *node: internalNodeList()) {
            gmsFileNode = node->toFile();
            if (gmsFileNode->file()->kind() == FileKind::Gms) {
                gmsFile = gmsFileNode->file();
                break;
            }
        }
        if (!gmsFile) return;
    } else {
        gmsFileNode = findFile(gmsFile);
    }
    if (gmsFile && gmsFile->kind() != FileKind::Gms) {
        DEB() << "Only files of FileKind::Gms can become runable";
        return;
    }
    if (!gmsFile) {
        setSpecialFile(FileKind::Gms, "");
        setSpecialFile(FileKind::Lst, "");
        return;
    }
    setLocation(QFileInfo(gmsFile->location()).absoluteDir().path());
    QString gmsPath = gmsFile->location();
    QString lstPath = QFileInfo(gmsPath).completeBaseName() + ".lst";
    setSpecialFile(FileKind::Gms, gmsPath);
    setSpecialFile(FileKind::Lst, lstPath);
    if (hasLogNode()) logNode()->resetLst();
}

QString ProjectRunGroupNode::lstErrorText(int line)
{
    return mLstErrorTexts.value(line);
}

void ProjectRunGroupNode::setLstErrorText(int line, QString text)
{
    if (text.isEmpty())
        DEB() << "Empty LST-text ignored for line " << line;
    else
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

void ProjectRunGroupNode::addRunParametersHistory(QString runParameters)
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

QStringList ProjectRunGroupNode::getRunParametersHistory() const
{
    return mRunParametersHistory;
}

QStringList ProjectRunGroupNode::analyzeParameters(const QString &gmsLocation, QList<OptionItem> itemList)
{
    // set studio default parameters
    QMap<QString, QString> defaultGamsArgs;
    defaultGamsArgs.insert("lo", "3");
    defaultGamsArgs.insert("ide", "1");
    defaultGamsArgs.insert("er", "99");
    defaultGamsArgs.insert("errmsg", "1");
    defaultGamsArgs.insert("pagesize", "0");
    defaultGamsArgs.insert("LstTitleLeftAligned", "1");
    QMap<QString, QString> gamsArgs(defaultGamsArgs);

    // find directory changes first
    QString path = "";
    for (OptionItem item : itemList) {
        if (QString::compare(item.key, "curdir", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "wdir", Qt::CaseInsensitive) == 0) {
            path = item.value;
            if (! (path.endsWith("/") || path.endsWith("\\")) )
                path += "/";

            gamsArgs[item.key] = item.value;
        }
    }

    QFileInfo fi(gmsLocation);
    // set default lst name to revert deleted o parameter values
    clearSpecialFiles();
    setSpecialFile(FileKind::Lst, path + fi.baseName() + ".lst");

    // iterate options
    for (OptionItem item : itemList) {
        // output (o) found
        if (QString::compare(item.key, "o", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "output", Qt::CaseInsensitive) == 0) {

            setSpecialFile(FileKind::Lst, path + item.value);
        } else if (QString::compare(item.key, "gdx", Qt::CaseInsensitive) == 0) {

            QString name = item.value;
            if (name == "default") name = fi.baseName() + ".gdx";
            setSpecialFile(FileKind::Gdx, path + name);
        } else if (QString::compare(item.key, "rf", Qt::CaseInsensitive) == 0) {

            QString name = item.value;
            if (name == "default") name = fi.baseName() + ".ref";
            setSpecialFile(FileKind::Ref, path + name);
        }

        if (defaultGamsArgs.contains(item.key)) {
            SysLogLocator::systemLog()->appendLog("You are overwriting at least one GAMS Studio default argument. "
                                 "Some of these are necessary to ensure a smooth experience. "
                                 "Use at your own risk!", LogMsgType::Warning);
        }
        gamsArgs[item.key] = item.value;
    }

    // prepare return value
#if defined(__unix__) || defined(__APPLE__)
    QStringList output { QDir::toNativeSeparators(gmsLocation) };
#else
    QStringList output { "\""+QDir::toNativeSeparators(gmsLocation)+"\"" };
#endif
    for(QString k : gamsArgs.keys()) {
        output.append(k + "=" + gamsArgs.value(k));
    }
    QString msg = "Running GAMS:";
    msg.append(output.join(" "));

    SysLogLocator::systemLog()->appendLog(msg, LogMsgType::Info);
    return output;
}

bool ProjectRunGroupNode::isProcess(const AbstractProcess *process) const
{
    return process && mGamsProcess.get() == process;
}

void ProjectRunGroupNode::jumpToFirstError(bool focus)
{
    if (!mLogNode) return;
    QList<TextMark*> marks = textMarkRepo()->marks(mLogNode->file()->id(), -1, id(), TextMark::error, 1);
    TextMark* textMark = marks.size() ? marks.first() : nullptr;
    if (textMark) {
        textMark->jumpToMark(focus);
        textMark->jumpToRefMark(focus);
    }
}

void ProjectRunGroupNode::lstTexts(const QList<TextMark *> &marks, QStringList &result)
{
    for (TextMark* mark: marks) {
        int lstLine = mark->value();
        if (lstLine < 0 && mark->refMark()) lstLine = mark->refMark()->value();
        QString newTip = lstErrorText(lstLine);
        if (!result.contains(newTip))
            result << newTip;
    }
}

QString ProjectRunGroupNode::specialFile(const FileKind &kind) const
{
    return mSpecialFiles.value(kind);
}

bool ProjectRunGroupNode::hasSpecialFile(const FileKind &kind) const
{
    return mSpecialFiles.contains(kind);
}

void ProjectRunGroupNode::addNodesForSpecialFiles()
{
    for (QString loc : mSpecialFiles.values())
        findOrCreateFileNode(loc);
}

void ProjectRunGroupNode::setSpecialFile(const FileKind &kind, const QString &path)
{
    if (path.isEmpty()) {
        mSpecialFiles.remove(kind);
        return;
    }
    QString fullPath = path;
    if (QFileInfo(path).isRelative())
        fullPath = QFileInfo(location()).canonicalFilePath() + "/" + path;

    if (QFileInfo(fullPath).suffix().isEmpty()) {
        switch (kind) {
        case FileKind::Gdx:
            fullPath += ".gdx";
            break;
        case FileKind::Lst:
            // no! gams does not add lst extension. unlike .ref or .gdx
            break;
        case FileKind::Ref:
            fullPath += ".ref";
            break;
        default:
            qDebug() << "WARNING: unhandled file type!" << fullPath << "is missing extension.";
        }
    }

    mSpecialFiles.insert(kind, fullPath);
}

void ProjectRunGroupNode::clearSpecialFiles()
{
    QString gms = mSpecialFiles.value(FileKind::Gms);
    mSpecialFiles.clear();
    mSpecialFiles.insert(FileKind::Gms, gms);
}

QProcess::ProcessState ProjectRunGroupNode::gamsProcessState() const
{
    return mGamsProcess->state();
}

QString ProjectRunGroupNode::tooltip()
{
    QString res(location());
    if (runnableGms()) res.append("\n\nMain GMS file: ").append(runnableGms()->name());
    if (!specialFile(FileKind::Lst).isEmpty())
        res.append("\nLast output file: ").append(QFileInfo(specialFile(FileKind::Lst)).fileName());
    if (debugMode()) {
        res.append("\nNodeId: "+QString::number(id()));
        res.append("\nParent-NodeId: " + (parentNode() ? QString::number(parentNode()->id()) : "?"));
    }
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

FileMetaRepo *ProjectRootNode::fileRepo() const
{
    return mRepo ? mRepo->fileRepo() : nullptr;
}

TextMarkRepo *ProjectRootNode::textMarkRepo() const
{
    return mRepo ? mRepo->textMarkRepo() : nullptr;
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
            it.value()->removeTextMarks(tmTypes);
        }
    }
}

void ProjectGroupNode::removeMarks(QString fileName, QSet<TextMark::Type> tmTypes)
{
    mMarksForFilenames.value(fileName)->removeTextMarks(tmTypes, true);
}

void ProjectGroupNode::dumpMarks()
{
    for (QString file: mMarksForFilenames.keys()) {
        QString res = file+":\n";
        TextMarkRepo* list = marks(file);
        for (TextMark* mark: list->marks()) {
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
    for (ProjectAbstractNode* child: mChildList) {
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
