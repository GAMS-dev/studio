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
#include <QDir>
#include <QApplication>
#include <QStringConverter>

#include "projectrepo.h"
#include "exception.h"
#include "syntax.h"
#include "logger.h"
#include "commonpaths.h"
#include "filemetarepo.h"
#include "process/abstractprocess.h"
#include "projecttreeview.h"
#include "viewhelper.h"
#include "settings.h"
#include "encoding.h"
#include "editors/sysloglocator.h"
#include "file/textfilesaver.h"

namespace gams {
namespace studio {

const QString ProjectRepo::CIgnoreSuffix(".lst.lxi.log.");
const QString CGamsSystemProjectName("-GAMS-System-");

ProjectRepo::ProjectRepo(QObject* parent)
    : QObject(parent), mNextId(0), mTreeModel(new ProjectTreeModel(this, new PExRootNode(this)))
{
    addToIndex(mTreeModel->rootNode());
    mProxyModel = new ProjectProxyModel(this);
    mProxyModel->setSourceModel(mTreeModel);
    mRunAnimateTimer.setInterval(250);
    runAnimateIcon(QIcon::Normal, 100);
    connect(&mRunAnimateTimer, &QTimer::timeout, this, &ProjectRepo::stepRunAnimation);
}

ProjectRepo::~ProjectRepo()
{
    mRunAnimateTimer.stop();
    mRunIcons.clear();
    FileType::clear();
    delete mProxyModel;
    delete mTreeModel;
}

void ProjectRepo::init(ProjectTreeView *treeView, FileMetaRepo *fileRepo, TextMarkRepo *textMarkRepo)
{
    Q_ASSERT_X(!mFileRepo && !mTextMarkRepo, "ProjectRepo initialization", "The ProjectRepo already has been initialized");
    Q_ASSERT_X(treeView, "ProjectRepo initialization", "The ProjectTreeView must not be null");
    Q_ASSERT_X(fileRepo, "ProjectRepo initialization", "The FileMetaRepo must not be null");
    Q_ASSERT_X(textMarkRepo, "ProjectRepo initialization", "The TextMarkRepo must not be null");
    mTreeView = treeView;
    mFileRepo = fileRepo;
    mTextMarkRepo = textMarkRepo;
    connect(mTreeModel, &ProjectTreeModel::childrenChanged, this, &ProjectRepo::childrenChanged);
    connect(mTreeModel, &ProjectTreeModel::parentAssigned, this, &ProjectRepo::parentAssigned);
    connect(mTreeModel, &ProjectTreeModel::projectListChanged, this, &ProjectRepo::projectListChanged);
    connect(mTreeModel, &ProjectTreeModel::getConfigPaths, this, &ProjectRepo::getConfigPaths);
}

PExProjectNode *ProjectRepo::findProject(const QString &projectFile) const
{
    PExRootNode *root = mTreeModel->rootNode();
    if (!root) return nullptr;
    return root->findProject(projectFile);
}

PExProjectNode *ProjectRepo::findProject(const NodeId& nodeId) const
{
    PExAbstractNode *node = mNodes.value(nodeId);
    if (!node) return nullptr;
    return node->assignedProject();
}

PExProjectNode *ProjectRepo::findProject(const AbstractProcess *process, PExGroupNode *group) const
{
    if (!group) group = mTreeModel->rootNode();
    return group->findProject(process);
}

PExFileNode *ProjectRepo::findFile(const QString& filePath, PExGroupNode *fileGroup) const
{
    FileMeta* fm = mFileRepo->fileMeta(filePath);
    return findFile(fm, fileGroup);
}

PExFileNode *ProjectRepo::findFile(FileMeta *fileMeta, PExGroupNode *fileGroup) const
{
    PExGroupNode *group = fileGroup ? fileGroup : mTreeModel->rootNode();
    return group->findFile(fileMeta);
}

PExAbstractNode *ProjectRepo::node(const NodeId &id) const
{
    return mNodes.value(id, nullptr);
}

PExAbstractNode *ProjectRepo::node(const QModelIndex& index) const
{
    return mProxyModel->node(index);
}

PExGroupNode *ProjectRepo::asGroup(const NodeId &id) const
{
    PExAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toGroup());
}

PExGroupNode *ProjectRepo::asGroup(const QModelIndex& index) const
{
    PExAbstractNode* node = mProxyModel->node(index);
    return node ? node->toGroup() : nullptr;
}

PExProjectNode *ProjectRepo::asProject(const NodeId &id) const
{
    PExAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toProject());
}

PExProjectNode *ProjectRepo::asProject(const QModelIndex &index) const
{
    PExAbstractNode* node = mProxyModel->node(index);
    return node ? node->toProject() : nullptr;
}

PExFileNode *ProjectRepo::asFileNode(const NodeId &id) const
{
    PExAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toFile());
}

PExFileNode *ProjectRepo::asFileNode(const QModelIndex& index) const
{
    PExAbstractNode* node = mProxyModel->node(index);
    return node ? node->toFile() : nullptr;
}

PExFileNode *ProjectRepo::findFileNode(QWidget *editWidget) const
{
    FileMeta *fileMeta = mFileRepo->fileMeta(editWidget);
    if (!fileMeta) return nullptr;
    NodeId groupId = fileMeta->projectId();
    PExAbstractNode *node = groupId.isValid() ? mNodes.value(groupId) : nullptr;
    PExGroupNode *group = node ? node->toGroup() : nullptr;
    if (!group) return nullptr;
    return group->findFile(fileMeta);
}

PExProjectNode *ProjectRepo::findProject(QWidget *edit) const
{
    FileMeta *fileMeta = mFileRepo->fileMeta(edit);
    if (!fileMeta) return nullptr;
    NodeId projId = fileMeta->projectId();
    if (!projId.isValid()) return nullptr;
    PExAbstractNode *node = mNodes.value(projId);
    return node ? node->toProject() : nullptr;
}

PExProjectNode *ProjectRepo::findProjectForPEdit(QWidget *projectEdit) const
{
    FileMeta *fileMeta = mFileRepo->fileMeta(projectEdit);
    if (!fileMeta || fileMeta->kind() != FileKind::Gsp) return nullptr;
    NodeId groupId = fileMeta->projectId();
    if (!groupId.isValid()) return nullptr;
    PExAbstractNode *node = mNodes.value(groupId);
    return node ? node->toProject() : nullptr;
}

PExAbstractNode *ProjectRepo::next(PExAbstractNode *node)
{
    if (!node || node->toRoot()) return nullptr;
    // for non-empty groups the next node is the first child
    if (node->toGroup() && node->toGroup()->childCount())
        return node->toGroup()->childNode(0);
    // for last-children
    PExGroupNode *group = node->parentNode();
    while (group->indexOf(node) == group->childCount()-1) {
        if (group->toRoot()) return group->toRoot()->childNode(0);
        node = group;
        group = node->parentNode();
    }
    return group->childNode(group->indexOf(node)+1);
}

PExAbstractNode *ProjectRepo::previous(PExAbstractNode *node)
{
    if (!node || node->toRoot()) return nullptr;
    int i = node->parentNode()->indexOf(node);
    if (i > 0) {
        node = node->parentNode()->childNode(i-1);
    } else if (node->parentNode()->toRoot()) {
        node = node->parentNode()->childNode(node->parentNode()->childCount()-1);
    } else {
        return node->parentNode();
    }
    PExGroupNode *group = node->toGroup();
    while (group && group->childCount()) {
        node = group->childNode(group->childCount()-1);
        group = node->toGroup();
        if (!group) return node;
    }
    return node;
}

PExProjectNode *ProjectRepo::gamsSystemProject()
{
    PExProjectNode *res = findProject(CGamsSystemProjectName);
    return res && res->type() == PExProjectNode::tGams ? res : nullptr;
}

ProjectProxyModel *ProjectRepo::proxyModel() const
{
    return mProxyModel;
}

FileMetaRepo *ProjectRepo::fileRepo() const
{
    Q_ASSERT_X(mFileRepo, "ProjectRepo", "FileMetaRepo not initialized");
    return mFileRepo;
}

TextMarkRepo *ProjectRepo::textMarkRepo() const
{
    Q_ASSERT_X(mTextMarkRepo, "ProjectRepo", "TextMarkRepo not initialized");
    return mTextMarkRepo;
}

bool ProjectRepo::checkRead(const QVariantMap &map, int &count, int &ignored, QStringList &missed, const QString &basePath)
{
    count = 0;
    ignored = 0;
    if (basePath.isEmpty()) {
        addWarning("Missing base path. Can't open project " + map.value("name").toString());
        return false;
    }
    QDir baseDir(basePath);
    if (!baseDir.exists() || !baseDir.isAbsolute()) {
        addWarning("Base path '" +basePath + "' not valid. Can't open project " + map.value("name").toString());
        return false;
    }

    missed.clear();
    QString runFile = map.value("file").toString();
    QString runPath = runFile.isEmpty() ? "" : QDir::cleanPath(baseDir.absoluteFilePath(runFile));

    QVariantList children = map.value("nodes").toList();
    if (!children.isEmpty() && !basePath.isEmpty()) {
        for (int i = 0; i < children.size(); ++i) {
            QVariantMap child = children.at(i).toMap();
            QString fileName = QDir::cleanPath(baseDir.absoluteFilePath(child.value("file").toString()));
            if (fileName.compare(runPath, FileType::fsCaseSense()) == 0)
                runPath = QString();
            QFileInfo file(fileName);
            if (!file.exists()) {
                if (CIgnoreSuffix.contains('.'+file.suffix()+'.')) ++ignored;
                else missed << fileName;
            }
            ++count;
        }
    }
    if (!runPath.isEmpty()) {
        missed << runPath + " (main file missing in file list)";
        ++count;
    }
    return missed.isEmpty();
}

bool ProjectRepo::readList(const QVariantList &projectsList)
{
    bool res = true;
    for (int i = 0; i < projectsList.size(); ++i) {
        QVariantMap projectMap = projectsList.at(i).toMap();
        if (!read(projectMap)) res = false;
    }
    return res;
}

bool ProjectRepo::read(const QVariantMap &projectMap, QString gspFile)
{
    bool res = true;
    PExProjectNode::Type type = PExProjectNode::tCommon;
    if (projectMap.contains("projectType"))
        type = PExProjectNode::Type(projectMap.value("projectType").toInt());
    else {
        // identify projectType for older settings
        QString name = projectMap.value("name").toString();
        if (name.startsWith("-Search: ")) type = PExProjectNode::tSearch;
        if (name.compare(CGamsSystemProjectName) == 0) type = PExProjectNode::tGams;
    }
    bool projectChangedMarker = false;
    QString projectPath;
    QVariantMap projectData = projectMap;

    // if there is a valid project file, load it instead of the settings part
    if (gspFile.isEmpty() && projectMap.contains("project"))
        gspFile = projectMap.value("project").toString();

    if (type == PExProjectNode::tCommon && !gspFile.isEmpty()) {
        projectPath = QFileInfo(gspFile).absolutePath();
        if (QFile::exists(gspFile)) {
            QVariantMap data = parseProjectFile(gspFile);
            projectChangedMarker = data.isEmpty();
            if (!data.isEmpty()) {
                projectData = data;
            }
        } else {
            QString message;
            int count = 0;
            int ignored = 0;
            QStringList missed;
            checkRead(projectMap, count, ignored, missed, projectPath);
            if (count == ignored + missed.count()) {
                message = "Couldn't restore missing project " + gspFile;
                SysLogLocator::systemLog()->append(message);
                return false;
            } else {
                message = "Restoring missing project:\n" + gspFile;
                SysLogLocator::systemLog()->append(message, LogMsgType::Info);
            }
            projectChangedMarker = true;
        }
    }

    // read name and path from projectData, and fill missing data
    QString name = projectData.value("name").toString();
    QString baseDir = projectData.value("path").toString();
    if (gspFile.isEmpty()) {
        if (baseDir.isEmpty())
            baseDir = CommonPaths::defaultWorkingDir();
        projectPath = baseDir;
        gspFile = projectPath + '/' + name + ".gsp";
    }
    if (baseDir.isEmpty()) {
        if (projectPath.isEmpty())
            projectPath = CommonPaths::defaultWorkingDir();
        baseDir = projectPath;
    }
    QDir projectDir(projectPath);
    if (QDir(baseDir).isRelative())
        baseDir = QDir::cleanPath(projectDir.absoluteFilePath(baseDir));

    QString workDir = QDir::cleanPath(projectDir.absoluteFilePath(projectData.value("workDir").toString()));
    if (workDir.isEmpty()) workDir = projectPath;

    QString runFile = QDir::cleanPath(projectDir.absoluteFilePath(projectData.value("file").toString()));

    QVariantList subChildren = projectData.value("nodes").toList();
    if (!name.isEmpty() || !projectPath.isEmpty()) {
        if (PExProjectNode* project = createProject(gspFile, baseDir, runFile, onExist_Project, workDir, type)) {
            if (projectData.contains("pf")) {
                QString pfFile = projectData.value("pf").toString();
                if (!pfFile.isEmpty())
                    pfFile = projectDir.absoluteFilePath(pfFile);
                project->setParameterFile(pfFile);
            }
            if (!readProjectFiles(project, subChildren, projectPath))
                res = false;
            bool expand = projectData.contains("expand") ? projectData.value("expand").toBool() : true;
            mTreeView->setExpanded(mProxyModel->asIndex(project), expand);
            if (projectChangedMarker)
                project->setNeedSave();
            if (projectData.contains("engineJobToken"))
                project->setEngineJobToken(projectData.value("engineJobToken").toString(), false);
            project->setRunFileParameterHistory(FileId(), projectData.value("options").toStringList());
            if (!project->childCount()) {
                closeGroup(project);
            }
        }
    }
    return res;
}

bool ProjectRepo::readProjectFiles(PExProjectNode *project, const QVariantList &children, const QString &baseDir)
{
    bool res = true;
    if (!project)
        EXCEPT() << "Missing project node, can't add file nodes";
    QDir localBaseDir(baseDir);
    for (int i = 0; i < children.size(); ++i) {
        QVariantMap child = children.at(i).toMap();
        QString name = child.value("name").toString();
        QString file = QDir::cleanPath(localBaseDir.absoluteFilePath(child.value("file").toString()));
        if (!name.isEmpty() || !file.isEmpty()) {
            QString suf = child["type"].toString();
            if (suf == "gms") suf = QFileInfo(name).fileName();
            FileType *ft = &FileType::from(suf);
            if (QFileInfo::exists(file)) {
                PExFileNode * node = findOrCreateFileNode(file, project, ft, name);
                QString encoding = child.contains("encoding") ?
                                       child.value("encoding").toString()
                                       : child.contains("codecMib") ?
                                           Encoding::name(child.value("codecMib").toInt())
                                           : Settings::settings()->toString(skDefaultEncoding);
                node->file()->setEncoding(encoding);
                QStringList optList;
                if (child.contains("options"))
                    optList = child.value("options").toStringList();
                project->setRunFileParameterHistory(node->file()->id(), optList);
            } else if (!CIgnoreSuffix.contains('.'+QFileInfo(file).suffix()+'.')) {
                emit addWarning("File not found: " + file);
                res = false;
            }
        }
    }
    project->setNeedSave(false);
    return res;
}

void ProjectRepo::write(QVariantList &projects) const
{
    for (int i = 0; i < mTreeModel->rootNode()->childCount(); ++i) {
        PExProjectNode *project = mTreeModel->rootNode()->childNode(i)->toProject();
        if (!project) continue;
        if (project->type() == PExProjectNode::tCommon && project->needSave()) {
            // store to file with relative paths
            QVariantMap proData = getProjectMap(project, true);
            save(project, proData);
        } else project->setNeedSave(false);
        // store to Settings with absolute paths
        if (project->type() != PExProjectNode::tSearch) {
            QVariantMap data;
            data = getProjectMap(project, false);
            data.insert("project", project->fileName());
            projects.append(data);
        }
    }
}

void ProjectRepo::save(PExProjectNode *project, const QVariantMap &data) const
{
    QString fileName = project->fileName();
    TextFileSaver file;
    if (file.open(fileName)) {
        file.write(QJsonDocument(QJsonObject::fromVariantMap(data)).toJson());
        if (file.close())
            project->setNeedSave(false);
    } else {
        SysLogLocator::systemLog()->append("Couldn't write project to " + fileName, LogMsgType::Error);
    }
}

QVariantMap ProjectRepo::getProjectMap(PExProjectNode *project, bool relativePaths) const
{
    if (!project) return QVariantMap();
    QVariantMap projectObject;
    bool expand = true;
    QDir dir(QFileInfo(project->fileName()).absolutePath());
    if (project->runnableGms()) {
        QString filePath = project->runnableGms()->location();
        projectObject.insert("file", relativePaths ? dir.relativeFilePath(filePath) : filePath);
    }
    if (project->hasParameterFile()) {
        QString pfPath;
        if (FileMeta * meta = project->parameterFile()) {
            QString filePath = meta->location();
            pfPath = relativePaths ? dir.relativeFilePath(filePath) : filePath;
        }
        projectObject.insert("pf", pfPath);
    }
    projectObject.insert("projectType", int(project->type()));
    projectObject.insert("path", relativePaths ? dir.relativeFilePath(project->location()) : project->location() );
    projectObject.insert("workDir", relativePaths ? dir.relativeFilePath(project->workDir()) : project->workDir() );
    projectObject.insert("name", project->name());
    projectObject.insert("options", project->getRunParametersHistory());
    if (!project->engineJobToken().isEmpty())
        projectObject.insert("engineJobToken", project->engineJobToken());
    QModelIndex mi = mProxyModel->asIndex(project);
    bool ok = true;
    expand = isExpanded(project->id(), &ok);
    if (!ok)
        expand = mTreeView->isExpanded(mi);
    if (!expand) projectObject.insert("expand", false);
    QVariantList subArray;
    writeProjectFiles(project, subArray, relativePaths);
    projectObject.insert("nodes", subArray);
    return projectObject;
}

void ProjectRepo::writeProjectFiles(const PExProjectNode* project, QVariantList& childList, bool relativePaths) const
{
    QDir dir(QFileInfo(project->fileName()).absolutePath());
    for (PExFileNode *file : project->listFiles()) {
        QVariantMap nodeObject;
        nodeObject.insert("file", relativePaths ? dir.relativeFilePath(file->location()) : file->location());
        nodeObject.insert("name", file->name());
        nodeObject.insert("type", file->file()->kindAsStr());
        nodeObject.insert("encoding", file->file()->encoding());

        // TODO(JM) Keep for two GAMS releases (until GAMS 52)
        nodeObject.insert("codecMib", Encoding::nameToOldMib(file->file()->encoding()));

        QStringList options = project->runFileParameterHistory(file->file()->id());
        if (options.size() == 1 && options.at(0).isEmpty())
            options.clear();
        if (!options.isEmpty())
            nodeObject.insert("options", options);
        childList.append(nodeObject);
    }
}

void ProjectRepo::addToProject(PExProjectNode *project, PExFileNode *file)
{
    PExGroupNode *oldParent = nullptr;
    if (mNodes.contains(file->id()))
        oldParent = file->parentNode()->toGroup();
    else addToIndex(file);

    // create missing group node for folders
    QList<PExGroupNode*> unsortedGroup;

    PExGroupNode *newParent = project;
    unsortedGroup << project;
    if (project->type() <= PExProjectNode::tCommon) {
        QDir prjPath(project->location());
        QString relPath = prjPath.relativeFilePath(file->location());
        bool isAbs = QDir(relPath).isAbsolute();
        QStringList folders;
        folders = relPath.split('/');
        folders.removeLast();
        for (const QString &folderName : std::as_const(folders)) {
            newParent = findOrCreateFolder(folderName, newParent, isAbs);
            unsortedGroup << newParent;
            isAbs = false;
        }
    }
    // add to (new) destination
    mTreeModel->insertChild(newParent->childCount(), newParent, file);
    for (PExGroupNode *group : unsortedGroup)
        sortChildNodes(group);
    purgeGroup(oldParent);
}

QString ProjectRepo::uniqueNameExt(PExGroupNode *parentNode, const QString &name, PExAbstractNode *node)
{
    // Project name should be unique, append number in case
    if (!parentNode) return name;
    QString res;
    int nr = 0;
    bool conflict = true;
    while (conflict) {
        conflict = false;
        for (PExAbstractNode * n : parentNode->childNodes()) {
            if (n != node && n->name(NameModifier::withNameExt) == name + res) {
                ++nr;
                res = QString::number(nr);
                conflict = true;
                break;
            }
        }
    }
    return res;
}

void ProjectRepo::uniqueProjectFile(PExGroupNode *parentNode, QString &filePath)
{
    // Project name must be unique in a path, append number in case
    if (!parentNode) return;
    int nr = 0;
    QFileInfo fi(filePath);
    QString res;
    bool conflict = true;
    while (conflict) {
        res = fi.path() + '/' + fi.completeBaseName() + (nr>0 ? QString::number(nr) : "") + ".gsp";
        conflict = false;
        for (PExAbstractNode * n : parentNode->childNodes()) {
            PExProjectNode *project = n->toProject();
            if (project && project->fileName().compare(res) == 0) {
                ++nr;
                conflict = true;
                break;
            }
        }
    }
    filePath = res;
}

PExProjectNode* ProjectRepo::createProject(QString name, const QString &path, const QString &runFileName, ProjectExistFlag mode,
                                           const QString &workDir, PExProjectNode::Type type)
{
    PExGroupNode *root = mTreeModel->rootNode();
    if (!root) FATAL() << "Can't get tree-model root-node";

    if (name.compare("-GAMS-System-") == 0)
        type = PExProjectNode::tGams;
    else if (type == PExProjectNode::tGams) {
        name = CGamsSystemProjectName;
    } else if (type <= PExProjectNode::tCommon) {
        if (!name.endsWith(".gsp", FileType::fsCaseSense())) {
            QFileInfo fi(name);
            name = path + '/' + fi.completeBaseName() + ".gsp";
        }
        if (!name.contains('/')) {
            name = path + '/' + name;
        }
    }

    PExProjectNode* project = findProject(name);
    if (project) {
        if (mode == onExist_Project) return project;
        if (mode == onExist_Null) return nullptr;
    }

    if (type <= PExProjectNode::tCommon)
        uniqueProjectFile(mTreeModel->rootNode(), name);

    FileMeta* runFile = runFileName.isEmpty() || type > PExProjectNode::tCommon ? nullptr
                                                                                : mFileRepo->findOrCreateFileMeta(runFileName);
    project = new PExProjectNode(name, path, runFile, workDir, type);
    if (type <= PExProjectNode::tCommon) {
        connect(project, &PExProjectNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChange);
        connect(project, &PExProjectNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChanged);
        connect(project, &PExProjectNode::getParameterValue, this, &ProjectRepo::getParameterValue);
        connect(project, &PExProjectNode::baseDirChanged, this, &ProjectRepo::reassignFiles);
        connect(project, &PExProjectNode::runnableChanged, this, &ProjectRepo::runnableChanged);
        connect(project, &PExProjectNode::updateProfilerAction, this, &ProjectRepo::updateProfilerAction);
        connect(project, &PExProjectNode::openInPinView, this, &ProjectRepo::openInPinView);
        connect(project, &PExProjectNode::openFileNode, this, [this](PExFileNode *node) {
            emit openFile(node->file(), true, node->assignedProject());
        });
        connect(project, &PExProjectNode::switchToTab, this, &ProjectRepo::switchToTab);
    }
    addToIndex(project);
    mTreeModel->insertChild(root->childCount(), root, project);
    connect(project, &PExGroupNode::changed, this, &ProjectRepo::nodeChanged);
    emit changed();
    mTreeView->setExpanded(mProxyModel->asIndex(project), true);
    sortChildNodes(root);
    return project;
}

MultiCopyCheck ProjectRepo::getCopyPaths(PExProjectNode *project, const QString &filePath, QStringList &srcFiles,
                                          QStringList &dstFiles, QStringList &missFiles, QStringList &collideFiles)
{
    QDir srcDir = QFileInfo(project->fileName()).path();
    QDir dstDir = QFileInfo(filePath).path();
    const QVector<PExFileNode*> nodes = project->listFiles();
    QStringList srcAll;
    srcAll << project->fileName();
    for (const PExFileNode *node : nodes)
        srcAll << node->location();
    bool skipFirst = project->type() == PExProjectNode::tSmall;

    for (const QString &source : std::as_const(srcAll)) {
        if (!skipFirst && !QFile::exists(source)) {
            missFiles << source;
        } else {
            QString relPath = srcDir.relativeFilePath(source);
            QString dest = dstDir.absoluteFilePath(relPath);
            if (QFile::exists(dest)) {
                collideFiles << dest;
            }
            srcFiles << source;
            dstFiles << dest;
        }
        skipFirst = false;
    }
    MultiCopyCheck res = mcsOk;
    if (missFiles.count() == nodes.count()) res = mcsMissAll;
    else {
        if (missFiles.count()) res = mcsMiss;
        if (collideFiles.count()) res = (res==mcsMiss ? mcsMissCollide : mcsCollide);
    }
    return res;
}

void ProjectRepo::moveProject(PExProjectNode *project, const QString &filePath, bool fullCopy)
{
    QString oldFile = project->fileName();
    project->setFileName(filePath);
    bool removeOld = filePath.compare(oldFile, FileType::fsCaseSense()) != 0;
    bool keepSmall = project->type() == PExProjectNode::tSmall && !removeOld;
    project->setHasGspFile(true);
    project->setNeedSave();
    QVariantMap proData = getProjectMap(project, true);
    save(project, proData);
    if (fullCopy) {
        if (keepSmall) project->setHasGspFile(false);
        project->setFileName(oldFile);
    } else {
        if (removeOld) {
            QFile file(oldFile);
            file.remove();
        }
        sortChildNodes(project->parentNode());
        emit changed();
    }
}

PExGroupNode *ProjectRepo::findOrCreateFolder(const QString &folderName, PExGroupNode *parentNode, bool isAbs)
{
    if (!parentNode) FATAL() << "Parent-node missing";
    if (parentNode == mTreeModel->rootNode()) FATAL() << "Folder-node must not exist on top level";

    for (int i = 0; i < parentNode->childCount(); ++i) {
        PExAbstractNode *node = parentNode->childNode(i);
        if (node->name().compare(folderName, FileType::fsCaseSense()) == 0) {
            PExGroupNode* folder = node->toGroup();
            if (!folder)
                EXCEPT() << "Folder node '" << folderName << "' already exists as file node";
            return node->toGroup();
        }
    }
    PExGroupNode* folder = new PExGroupNode(folderName, isAbs ? folderName
                                                              : QDir::cleanPath(parentNode->location()+'/'+folderName));
    addToIndex(folder);
    mTreeModel->insertChild(parentNode->childCount(), parentNode, folder);
    connect(folder, &PExGroupNode::changed, this, &ProjectRepo::nodeChanged);
    emit changed();
    sortChildNodes(parentNode);
    return folder;
}

void ProjectRepo::closeGroup(PExGroupNode* group)
{
    if (group->childCount()) EXCEPT() << "Can't close project that isn't empty";
    if (mNodes.contains(group->id())) {
        mTreeModel->removeChild(group);
        removeFromIndex(group);
        group->deleteLater();
    }
}

void ProjectRepo::closeNode(PExFileNode *node)
{
    PExGroupNode *group = node->parentNode();
    PExProjectNode *project = node->assignedProject();
    FileMeta *fm = node->file();
    QList<PExFileNode*> otherNodes = fileNodes(fm->id());
    otherNodes.removeAll(node);

    if (node->file()->isOpen() && otherNodes.isEmpty()) {
        DEB() << "Close error: Node has open editors";
        return;
    }

    // Remove reference (if this is a lst file referenced in a log)
    if (project && project->hasLogNode() && project->logNode()->lstNode() == node)
        project->logNode()->resetLst();

    // close actual file and remove repo node
    if (mNodes.contains(node->id())) {
        mTreeModel->removeChild(node);
        removeFromIndex(node);
    }

    // if this file is marked as runnable remove reference
    if (project && project->runnableGms() == node->file()) {
        project->setRunnableGms();
        for (int i = 0; i < project->childCount(); i++) {
            // choose next as main gms file
            PExFileNode *nextRunable = project->childNode(i)->toFile();
            if (nextRunable && nextRunable->location().endsWith(".gms", Qt::CaseInsensitive)) {
                project->setRunnableGms(nextRunable->file());
                break;
            }
        }
    }
    node->deleteLater();
    if (otherNodes.isEmpty())
        fm->deleteLater();
    else
        fm->setProjectId(otherNodes.first()->projectId());
    purgeGroup(group);
}

void ProjectRepo::purgeGroup(PExGroupNode *group)
{
    if (!group || group->toRoot()) return;
    PExGroupNode *parGroup = group->parentNode();
    if (group->isEmpty() && !group->toProject()) {
        closeGroup(group);
        if (parGroup) purgeGroup(parGroup);
    }
}

void ProjectRepo::sortChildNodes(PExGroupNode *group)
{
    mTreeModel->sortChildNodes(group);
}

void ProjectRepo::focusProject(PExProjectNode *project)
{
    storeExpansionState(mProxyModel->rootModelIndex());
    mProxyModel->focusProject(project);
    restoreExpansionState(mProxyModel->rootModelIndex());
}

PExProjectNode *ProjectRepo::focussedProject() const
{
    return mProxyModel->focussedProject();
}

void ProjectRepo::storeExpansionState(QModelIndex parent)
{
    for (int row = 0; row < mProxyModel->rowCount(parent); ++row) {
        QModelIndex mi = mProxyModel->index(row, 0, parent);
        if (mProxyModel->hasChildren(mi)) {
            NodeId id = mProxyModel->nodeId(mi);
            if (id.isValid()) {
                mIsExpanded.insert(id, mTreeView->isExpanded(mi));
                storeExpansionState(mi);
            }
        }
    }
}

void ProjectRepo::restoreExpansionState(QModelIndex parent)
{
    for (int row = 0; row < mProxyModel->rowCount(parent); ++row) {
        QModelIndex mi = mProxyModel->index(row, 0, parent);
        if (mProxyModel->hasChildren(mi)) {
            NodeId id = mProxyModel->nodeId(mi);
            if (id.isValid() && mIsExpanded.contains(id)) {
                if (mIsExpanded.value(id))
                    mTreeView->expand(mi);
                else
                    mTreeView->collapse(mi);
                restoreExpansionState(mi);
            }
        }
    }
}

bool ProjectRepo::isExpanded(NodeId id, bool *ok) const
{
    if (id.isValid() && mIsExpanded.contains(id)) {
        if (ok) *ok = true;
        return mIsExpanded.value(id);
    }
    if (ok) *ok = false;
    return false;
}

PExFileNode *ProjectRepo::findOrCreateFileNode(QString location, PExProjectNode *project, FileType *knownType,
                                               const QString &explicitName)
{
    if (location.isEmpty())
        return nullptr;
    if (location.contains('\\'))
        location = QDir::fromNativeSeparators(location);

    if (!knownType || knownType->kind() == FileKind::None)
        knownType = parseGdxHeader(location) ? &FileType::from(FileKind::Gdx) : nullptr;

    FileMeta* fileMeta = mFileRepo->findOrCreateFileMeta(location, knownType);
    return findOrCreateFileNode(fileMeta, project, explicitName);
}

PExFileNode* ProjectRepo::findOrCreateFileNode(FileMeta* fileMeta, PExProjectNode* project, const QString &explicitName)
{
    if (!fileMeta) {
        DEB() << "The file meta must not be null";
        return nullptr;
    }
    if (!project) {
        QFileInfo fi(fileMeta->location());
        QString groupName = explicitName.isNull() ? fi.completeBaseName() : explicitName;

        if (PExFileNode *pfn = findFile(fileMeta))
            project = pfn->assignedProject();
        else
            project = createProject(groupName, fi.absolutePath(), fi.filePath(), onExist_Project);

        if (!project) {
            DEB() << "The project must not be null";
            return nullptr;
        }
    }
    PExFileNode* file = findFile(fileMeta, project);
    if (!file) {
        mProxyModel->deselectAll();
        if (fileMeta->kind() == FileKind::Log)
            return project->logNode();
        file = new PExFileNode(fileMeta);
        if (!explicitName.isNull())
            file->setName(explicitName);
        addToProject(project, file);
        fileMeta->setProjectId(project->id());
    }
    connect(project, &PExGroupNode::changed, this, &ProjectRepo::nodeChanged);
    if (!project->runnableGms() && fileMeta->kind() == FileKind::Gms)
        project->setRunnableGms(fileMeta);
    return file;
}

PExLogNode*ProjectRepo::logNode(PExAbstractNode* node)
{
    if (!node) return nullptr;
    // Find the project
    PExProjectNode* project = node->assignedProject();
    if (!project) return nullptr;
    PExLogNode* log = project->logNode();
    if (!log) {
        DEB() << "Error while creating LOG node.";
        return nullptr;
    }
    return log;
}

void ProjectRepo::saveNodeAs(PExFileNode *node, const QString &target)
{

    FileMeta* sourceFM = node->file();
    QString sourceFilename = sourceFM->location();
    QList<PExFileNode*> sourceNodes = fileNodes(sourceFM->id());
    sourceNodes.removeAll(node);

    FileMeta *destFM = fileRepo()->fileMeta(target);
    QList<PExFileNode*> destNodes;
    if (destFM) {
        if (destFM->isOpen()) {
            SysLogLocator::systemLog()->append("Editor must not be open for " + target);
            return;
        }
        destNodes << fileNodes(destFM->id());
    }

    if (sourceFM->save(target)) {
        // transfer to destination
        node->setName(sourceFM->name());
        for (PExFileNode *node : destNodes)
            node->replaceFile(sourceFM);

        // create new FileMeta for source if there are remaining nodes
        if (sourceNodes.size()) {
            sourceFM = fileRepo()->findOrCreateFileMeta(sourceFilename);

            for (PExFileNode *node : sourceNodes) {
                node->replaceFile(sourceFM);
            }
        }
        PExProjectNode *project = node->assignedProject();
        if (project->type() == PExProjectNode::tGams) {
            bool add = false;
            for (const QString &path : CommonPaths::gamsStandardPaths(CommonPaths::StandardConfigPath)) {
                if (target.compare(path + "/gamsconfig.yaml", FileType::fsCaseSense()) == 0) {
                    add = true;
                    break;
                }
            }
            if (add)
                addToProject(project, node);
            else {
                QFileInfo fi(target);
                PExProjectNode *newPro = createProject(fi.completeBaseName(), fi.path(), "", onExist_Project);
                if (newPro)
                    addToProject(newPro, node);
            }
        } else {
            addToProject(project, node);
        }

        project->setNeedSave();

        // macOS didn't focus on the new node
        mProxyModel->setCurrent(mProxyModel->asIndex(node));
    }
}

QVector<PExFileNode*> ProjectRepo::fileNodes(const FileId &fileId, const NodeId &groupId) const
{
    QVector<PExFileNode*> res;
    QHashIterator<NodeId, PExAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        PExFileNode* fileNode = i.value()->toFile();
        if (fileNode && fileNode->file()->id() == fileId) {
            if (!groupId.isValid() || fileNode->projectId() == groupId) {
                res << fileNode;
            }
        }
    }
    return res;
}

const QList<PExProjectNode *> ProjectRepo::projects(const FileId &fileId) const
{
    QList<PExProjectNode *> res;
    QHashIterator<NodeId, PExAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        if (fileId.isValid()) {
            PExFileNode* fileNode = i.value()->toFile();
            if (fileNode && fileNode->file()->id() == fileId) {
                PExProjectNode *project = fileNode->assignedProject();
                if (project && !res.contains(project)) {
                    res << project;
                }
            }
        } else {
            PExProjectNode* project = i.value()->toProject();
            if (project) {
                res << project;
            }
        }
    }
    return res;
}

const QList<PExProjectNode *> ProjectRepo::projects() const
{
    QList<PExProjectNode *> res;
    for (PExAbstractNode *node : mTreeModel->rootNode()->childNodes()) {
        if (PExProjectNode *project = node->toProject())
            res << project;
    }
    return res;
}

const QVector<AbstractProcess *> ProjectRepo::listProcesses()
{
    QVector<AbstractProcess*> res;
    QHashIterator<NodeId, PExAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        PExProjectNode* project = i.value()->toProject();
        if (project && project->process()) {
            res << project->process();
        }
    }
    return res;
}

void ProjectRepo::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    mProxyModel->selectionChanged(selected, deselected);
    QVector<QModelIndex> groups;
    for (QModelIndex ind: mProxyModel->popAddProjects()) {
        if (!mTreeView->isExpanded(ind))
            groups << ind;
    }
    QItemSelectionModel *selModel = mTreeView->selectionModel();
    for (QModelIndex ind: mProxyModel->popDeclined()) {
        selModel->select(ind, QItemSelectionModel::Deselect);
    }
    for (QModelIndex group: std::as_const(groups)) {
        if (!mTreeView->isExpanded(group)) {
            mTreeView->setExpanded(group, true);
            for (int row = 0; row < mProxyModel->rowCount(group); ++row) {
                QModelIndex ind = mProxyModel->index(row, 0, group);
                selModel->select(ind, QItemSelectionModel::Select);
            }
        }
    }
}

void ProjectRepo::errorTexts(const NodeId &groupId, const QVector<int> &lstLines, QStringList &result)
{
    PExProjectNode *project = asProject(groupId);
    if (project)
        project->errorTexts(lstLines, result);
}

void ProjectRepo::stepRunAnimation()
{
    mRunAnimateIndex = ((mRunAnimateIndex+1) % mRunIconCount);
    for (PExProjectNode* project: std::as_const(mRunnigGroups)) {
        QModelIndex ind = mProxyModel->asIndex(project);
        if (ind.isValid())
            emit mProxyModel->dataChanged(ind, ind);
    }
}

void ProjectRepo::dropFiles(QModelIndex idx, QStringList files, QList<NodeId> knownIds, Qt::DropAction act
                            , QList<QModelIndex> &newSelection)
{
    while (files.count() && files.first().isEmpty())
        files.removeFirst();

    QList<NodeId> addIds;
    for (const NodeId &id : knownIds) {
        PExGroupNode *group = asGroup(id);
        if (group && group->type() == NodeType::group) {
            QVector<PExFileNode*> groupFiles = group->listFiles();
            for (PExFileNode* file: std::as_const(groupFiles)) {
                files << file->location();
                addIds << file->id();
            }
        }
    }
    knownIds.append(addIds);

    PExProjectNode *project = nullptr;
    if (idx.isValid()) {
        PExAbstractNode *aNode = node(idx);
        project = aNode->assignedProject();
    } else {
        QString validFile;
        for (const QString &filePath: std::as_const(files)) {
            if (!filePath.endsWith(".gsp", Qt::CaseInsensitive)) {
                validFile = filePath;
                break;
            }
        }
        if (!validFile.isEmpty()) {
            QFileInfo file(validFile);
            QString name;
            QString basePath;
            if (file.isFile())
                basePath = file.absolutePath();
            else if (file.isDir())
                basePath = file.filePath();
            name = file.completeBaseName();
            project = createProject(name, basePath, files.first(), onExist_AddNr);
        }
    }

    int projectCount = 0;
    int otherFileCount = 0;
    for (const QString &item: std::as_const(files)) {
        QFileInfo f(item);
        QDir d(item);
        if (f.isFile()) {
            if (item.endsWith(".gsp", Qt::CaseInsensitive)) {
                ++projectCount;
                continue;
            }
            ++otherFileCount;
        } else if (d.exists()) {
            ++otherFileCount;
        }
    }

    QStringList filesNotFound;
    QList<PExFileNode*> gmsFiles;
    PExFileNode *fileToShow = nullptr;
    QList<NodeId> newIds;
    for (const QString &item: std::as_const(files)) {
        QFileInfo f(item);
        QDir d(item);

        if (f.isFile()) {
            if (item.endsWith(".gsp", Qt::CaseInsensitive)) {
                emit openProject(item);
                continue;
            }
            PExFileNode* file = findOrCreateFileNode(item, project);
            if (knownIds.contains(file->id())) knownIds.removeAll(file->id());
            if (file->file()->kind() == FileKind::Gms) gmsFiles << file;
            if (!newIds.contains(file->id())) newIds << file->id();
            if (!fileToShow) fileToShow = file;
        } else if (d.exists()) {
            emit openFolder(item, project);
        } else {
            filesNotFound << item;
        }
    }
    for (const NodeId &id: std::as_const(newIds)) {
        QModelIndex mi = mProxyModel->asIndex(id);
        newSelection << mi;
    }
    if (!filesNotFound.isEmpty()) {
        DEB() << "Files not found:\n" << filesNotFound.join("\n");
    }
    if (project && !project->runnableGms() && !gmsFiles.isEmpty()) {
        project->setRunnableGms(gmsFiles.first()->file());
    }
    if (act & Qt::MoveAction) {
        for (const NodeId &nodeId: std::as_const(knownIds)) {
            PExAbstractNode* aNode = node(nodeId);
            PExFileNode* file = aNode->toFile();
            if (!file) continue;
            if (file->parentNode() != project)
                closeNode(file);
        }
    }
    if (projectCount && otherFileCount) {
        emit doFocusProject(nullptr);
        if (fileToShow)
            emit openFile(fileToShow->file(), true, fileToShow->assignedProject());
    } else if (mProxyModel->focussedProject() && project && mProxyModel->focussedProject() != project && !projectCount)
        emit doFocusProject(project);
}

void ProjectRepo::reassignFiles(PExProjectNode *project)
{
    QVector<PExFileNode *> files = project->listFiles();
    FileMeta *runGms = project->runnableGms();
    for (PExFileNode *file: std::as_const(files))
        addToProject(project, file);
    emit openRecentFile();
    project->setRunnableGms(runGms);
}

QVariantMap ProjectRepo::parseProjectFile(const QString &gspFile) const
{
    QJsonDocument json;
    QFile file(gspFile);
    if (file.open(QFile::ReadOnly)) {
        QJsonParseError parseResult;
        json = QJsonDocument::fromJson(file.readAll(), &parseResult);
        if (parseResult.error) {
            if (SysLogLocator::systemLog())
                SysLogLocator::systemLog()->append("Couldn't parse project from " + gspFile, LogMsgType::Error);
            return QVariantMap();
        }
        file.close();
        QVariantMap map = json.object().toVariantMap();
        if (map.contains("projects")) {
            QVariantList list = map.value("projects").toList();
            map = list.at(0).toMap();
        }
        return map;
    } else if (SysLogLocator::systemLog()) {
        SysLogLocator::systemLog()->append("Couldn't open project " + gspFile, LogMsgType::Error);
    }
    return QVariantMap();
}

void ProjectRepo::editorActivated(QWidget* edit, bool select)
{
    PExAbstractNode *node = findProjectForPEdit(edit);
    if (!node)
        node = findFileNode(edit);
    if (!node) return;

    QModelIndex mi = mProxyModel->asIndex(node);
    if (mi.isValid()) {
        mProxyModel->setCurrent(mi);
        mTreeView->setCurrentIndex(mi);
        if (select) mTreeView->selectionModel()->select(mi, QItemSelectionModel::ClearAndSelect);
    }
}

void ProjectRepo::editProjectName(PExProjectNode *project)
{
    QModelIndex mi = mProxyModel->asIndex(project);
    if (!mi.isValid()) return;
    mTreeView->edit(mi);
}

void ProjectRepo::nodeChanged(const NodeId &nodeId)
{
    PExAbstractNode* nd = node(nodeId);
    if (!nd) return;
    QModelIndex ndIndex = mProxyModel->asIndex(nd);
    emit mProxyModel->dataChanged(ndIndex, ndIndex);
}

void ProjectRepo::closeNodeById(const NodeId &nodeId)
{
    PExAbstractNode *aNode = node(nodeId);
    if (!aNode) return;
    PExGroupNode *group = aNode ? aNode->parentNode() : nullptr;
    if (aNode->toFile()) closeNode(aNode->toFile());
    if (aNode->toGroup()) closeGroup(aNode->toGroup());
    if (group) purgeGroup(group);
}

bool ProjectRepo::parseGdxHeader(const QString &location)
{
    QFile file(location);
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.read(50);
        file.close();
        return data.contains("\aGAMSGDX\a");
    }
    return false;
}

QIcon ProjectRepo::runAnimateIcon(QIcon::Mode mode, int alpha)
{
    QPair<QIcon::Mode, int> key(mode, alpha);
    if (!mRunIcons.contains(key)) {
        QVector<QIcon> runIcons;
        runIcons << Theme::icon(":/img/project-run1", mode, alpha);
        runIcons << Theme::icon(":/img/project-run2", mode, alpha);
        runIcons << Theme::icon(":/img/project-run3", mode, alpha);
        runIcons << Theme::icon(":/img/project-run4", mode, alpha);
        runIcons << Theme::icon(":/img/project-run5", mode, alpha);
        runIcons << Theme::icon(":/img/project-run6", mode, alpha);
        mRunIcons.insert(key, runIcons);
        mRunIconCount = runIcons.count();
    }
    return mRunIcons.value(key).at(mRunAnimateIndex);
}

int ProjectRepo::activeProcesses()
{
    int res = 0;
    for (const PExProjectNode *project : projects()) {
        if (project->process() && project->gamsProcessState() != QProcess::NotRunning)
            ++res;
    }
    return res;
}

void ProjectRepo::gamsProcessStateChange(PExProjectNode *project)
{
    if (!project) return;
    QModelIndex ind = mProxyModel->asIndex(project);
    if (project->process()->state() == QProcess::NotRunning) {
        mRunnigGroups.removeAll(project);
        if (ind.isValid()) emit mProxyModel->dataChanged(ind, ind);
    } else if (!mRunnigGroups.contains(project)) {
        mRunnigGroups << project;
        if (ind.isValid()) emit mProxyModel->dataChanged(ind, ind);
    }
    if (mRunnigGroups.isEmpty() && mRunAnimateTimer.isActive()) {
        mRunAnimateTimer.stop();
        mRunAnimateIndex = 0;
    } else if (!mRunnigGroups.isEmpty() && !mRunAnimateTimer.isActive()) {
        mRunAnimateIndex = 0;
        mRunAnimateTimer.start();
    }
}

bool ProjectRepo::debugMode() const
{
    return mDebugMode;
}

void ProjectRepo::fileChanged(const FileId &fileId)
{
    QVector<PExGroupNode*> groups;
    const auto nodes = fileNodes(fileId);
    for (PExFileNode *node: nodes) {
        PExGroupNode *group = node->parentNode();
        while (group && group != mTreeModel->rootNode()) {
            if (groups.contains(group)) break;
            groups << group;
            group = group->parentNode();
        }
        nodeChanged(node->id());
    }
    for (PExGroupNode *group: std::as_const(groups)) {
        nodeChanged(group->id());
    }
}

void ProjectRepo::setDebugMode(bool debug)
{
    mDebugMode = debug;
    mTreeModel->setDebugMode(debug);
    mFileRepo->setDebugMode(debug);
    mTextMarkRepo->setDebugMode(debug);

    for (auto it = mNodes.constBegin() ; it != mNodes.constEnd() ; ++it) {
        if (PExProjectNode *project = it.value()->toProject()) {
            project->setVerbose(debug);
            PExLogNode* log = project->logNode();
            if (log && log->file()->editors().size()) {
                TextView *tv = ViewHelper::toTextView(log->file()->editors().first());
                if (tv) tv->setDebugMode(debug);
            }

        }
    }
}

} // namespace studio
} // namespace gams
