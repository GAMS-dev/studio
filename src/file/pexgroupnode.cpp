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
#include "pexgroupnode.h"
#include "pexfilenode.h"
#include "pexlognode.h"
#include "projectrepo.h"
#include "filemeta.h"
#include "filemetarepo.h"
#include "exception.h"
#include "editors/systemlogedit.h"
#include "process.h"
#include "commonpaths.h"
#include "logger.h"
#include "syntax.h"
#include "option/option.h"
#include "editors/sysloglocator.h"
#include "projectedit.h"
#include "settings.h"
#include "viewhelper.h"
#include "debugger/breakpointdata.h"

#include <QtConcurrent>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

namespace gams {
namespace studio {

PExGroupNode::PExGroupNode(const QString &name, const QString &location, NodeType type)
    : PExAbstractNode(name, type)
{
    PExGroupNode::setLocation(location);
}

PExGroupNode::~PExGroupNode()
{
    if (mChildNodes.size())
        DEB() << "Group must be empty before deletion";
}

QIcon PExGroupNode::icon(QIcon::Mode mode, int alpha)
{
    PExProjectNode *project = assignedProject();
    if (project) {
        FileMeta *fmGms = project->runnableGms();
        if (fmGms) {
            PExFileNode *gms = projectRepo()->findFile(fmGms, project);
            if (gms) {
                PExGroupNode *group = gms->parentNode();
                while (group != project) {
                    if (group == this)
                        return Theme::icon(":/img/folder-open-run", mode, alpha);
                    group = group->parentNode();
                }
            }
        }
    }
    return Theme::icon(":/img/folder-open", mode, alpha);
}

int PExGroupNode::childCount() const
{
    return mChildNodes.count();
}

bool PExGroupNode::isEmpty()
{
    return (mChildNodes.count() == 0);
}

PExAbstractNode*PExGroupNode::childNode(int index) const
{
    return mChildNodes.at(index);
}

int PExGroupNode::indexOf(PExAbstractNode* child)
{
    return mChildNodes.indexOf(child);
}

void PExGroupNode::appendChild(PExAbstractNode* child)
{
    if (!child || mChildNodes.contains(child)) return;
    mChildNodes.append(child);
}

void PExGroupNode::removeChild(PExAbstractNode* child)
{
    mChildNodes.removeOne(child);
}

QString PExGroupNode::location() const
{
    return mLocation;
}

void PExGroupNode::setLocation(const QString& newLocation)
{
    mLocation = mLocation.contains('\\') ? QDir::fromNativeSeparators(newLocation) : newLocation;
    if (mLocation.endsWith('/')) mLocation.remove(mLocation.length()-1, 1);
    emit changed(id());
}

QString PExGroupNode::tooltip()
{
    QString res = QDir::toNativeSeparators(location());
    if (debugMode()) {
        res.append("\nNodeId: "+QString::number(id()));
        res.append("\nParent-NodeId: " + (parentNode() ? QString::number(parentNode()->id()) : "?"));
    }
    return QString(res);
}

QString PExGroupNode::errorText(int lstLine)
{
    return parentNode() ? parentNode()->errorText(lstLine) : QString();
}

PExFileNode *PExGroupNode::findFile(QString location) const
{
    if (location.contains('\\')) location = QDir::fromNativeSeparators(location);
    QFileInfo fi(location);
    for (PExFileNode* file: listFiles())
        if (file && FileMetaRepo::equals(QFileInfo(file->location()), fi)) return file;
    return nullptr;
}

PExFileNode *PExGroupNode::findFile(const FileMeta *fileMeta) const
{
    if (!fileMeta) return nullptr;
    if (fileMeta->kind() == FileKind::Log) return nullptr;
    for (PExFileNode* fileNode: listFiles()) {
        if (fileNode && fileNode->file() == fileMeta) return fileNode;
    }
    return nullptr;
}

QList<PExFileNode*> PExGroupNode::findFiles(FileKind kind) const
{
    QList<PExFileNode*> res;
    for (PExFileNode* fileNode: listFiles())
        if (fileNode && fileNode->file()->kind() == kind) res << fileNode;
    return res;
}

PExProjectNode *PExGroupNode::findProject(const AbstractProcess *process) const
{
    for (PExAbstractNode* node: childNodes()) {
        PExProjectNode* project = node->toProject();
        if (project && project->isProcess(process))
            return project;
        const PExGroupNode* group = node->toGroup();
        if (group) {
            project = findProject(process);
            if (project) return project;
        }
    }
    return nullptr;
}

PExProjectNode *PExGroupNode::findProject(const FileId &runId) const
{
    for (PExAbstractNode* node: childNodes()) {
        PExProjectNode* project = node->toProject();
        if (project && project->runnableGms()->id() == runId)
            return project;
        const PExGroupNode* group = node->toGroup();
        if (group) {
            project = findProject(runId);
            if (project) return project;
        }
    }
    return nullptr;
}

const QVector<PExFileNode *> PExGroupNode::listFiles() const
{
    QVector<PExFileNode *> res;
    for (PExAbstractNode *node: mChildNodes) {
        PExFileNode *fileNode = node->toFile();
        if (fileNode)
            res << fileNode;
        else {
            PExGroupNode *sub = node->toGroup();
            if (sub) res << sub->listFiles();
        }
    }
    return res;
}

void PExGroupNode::moveChildNode(int from, int to)
{
    mChildNodes.move(from, to);
}

void PExGroupNode::hasFile(const QString &fName, bool &exists)
{
    exists = findFile(fName);
}

PExProjectNode::PExProjectNode(const QString &filePath, const QString &basePath,
                               FileMeta* runFileMeta, const QString &workDir, Type type)
    : PExGroupNode(QFileInfo(filePath).completeBaseName(), basePath, NodeType::project)
    , mProjectFile(filePath)
    , mWorkDir(workDir)
    , mType(type)
{
    mBreakpointData = new debugger::BreakpointData();
    if (mWorkDir.isEmpty()) mWorkDir = basePath;
    if (runFileMeta && runFileMeta->kind() == FileKind::Gms) {
        setRunnableGms(runFileMeta);
        setNeedSave(false);
    }
}

void PExProjectNode::setHasGspFile(bool hasGspFile)
{
    if (mType > Type::tCommon) return;
    mType = hasGspFile ? Type::tCommon : Type::tSmall;
}

bool PExProjectNode::wantsGspFile()
{
    if (Settings::settings()->toBool(skProGspNeedsMain) && !runnableGms())
        return false;
    int fileCountLimit = Settings::settings()->toInt(skProGspByFileCount);
    if (fileCountLimit == 0 || childCount() < fileCountLimit)
        return false;
    return true;
}

///
/// \brief Set the process.
/// \remark Takes ownership of the of the process pointer.
///
void PExProjectNode::setProcess(AbstractProcess* process)
{
    if (mGamsProcess.get() == process) return;
    if (mGamsProcess)
        mGamsProcess->disconnect();
    mGamsProcess.reset(process);
    connect(mGamsProcess.get(), &GamsProcess::stateChanged, this, &PExProjectNode::onGamsProcessStateChanged);
    if (mDebugServer)
        connect(mGamsProcess.get(), &AbstractProcess::interruptGenerated, mDebugServer, &debugger::Server::sendStepLine);

}

AbstractProcess *PExProjectNode::process() const
{
    return mGamsProcess.get();
}

PExProjectNode::~PExProjectNode()
{
    delete mBreakpointData;
    mBreakpointData = nullptr;
    setProjectEditFileMeta(nullptr);
}

QIcon PExProjectNode::icon(QIcon::Mode mode, int alpha)
{
    if (gamsProcessState() == QProcess::NotRunning)
        return Theme::icon(":/img/project", mode, alpha);
    return projectRepo()->runAnimateIcon(mode, alpha);
}

QString PExProjectNode::name(NameModifier mod) const
{
    if (mod == NameModifier::withNameExt)
        return PExGroupNode::name() + nameExt();
    return PExGroupNode::name(mod);
}

void PExProjectNode::setName(const QString &name)
{
    PExGroupNode::setName(name);
    updateLogName(name);
}

void PExProjectNode::setNameExt(const QString &newNameExt)
{
    PExAbstractNode::setNameExtIntern(newNameExt);
    updateLogName(name());
    emit changed(id());
}

const QString &PExProjectNode::fileName() const
{
    return mProjectFile;
}

void PExProjectNode::setFileName(const QString &newProjectFile)
{
    mProjectFile = newProjectFile;
    setName(QFileInfo(mProjectFile).completeBaseName());
    emit changed(id());
}

bool PExProjectNode::hasLogNode() const
{
    return mLogNode;
}

void PExProjectNode::setLogNode(PExLogNode* logNode)
{
    if (!logNode) return;
    if (mLogNode && mLogNode != logNode)
        EXCEPT() << "Reset the logNode is not allowed";
    mLogNode = logNode;
    QFileInfo fi(mLogNode->location());
    mLogNode->setName(fi.completeBaseName() + nameExt());
}

void PExProjectNode::appendChild(PExAbstractNode *child)
{
    PExGroupNode::appendChild(child);
    PExFileNode *file = child->toFile();
    if (!mParameterHash.contains("gms") && file && file->file() && file->file()->kind() == FileKind::Gms) {
        setRunnableGms(file->file());
    }
    if (!mParameterHash.contains("pf") && file && file->file() && file->file()->kind() == FileKind::Pf) {
        setParameterFile(file->file());
    }
    if (wantsGspFile())
        setHasGspFile(true);
    setNeedSave();
}

void PExProjectNode::removeChild(PExAbstractNode *child)
{
    PExGroupNode::removeChild(child);
    bool gmsLost = false;
    bool pfLost = false;
    PExFileNode *file = child->toFile();
    if (file) {
        const QList<QString> keys = mParameterHash.keys(file->location());
        for (const QString &key: keys) {
            mParameterHash.remove(key);
            if (key=="gms") gmsLost = true;
            if (key=="pf") pfLost = true;
        }
    }
    if (gmsLost) setRunnableGms();
    if (pfLost) setParameterFile();
    setNeedSave();
}

QString PExProjectNode::resolveHRef(const QString &href, PExFileNode *&node, int &line, int &col, bool create)
{
    const QStringList tags {"LST","LS2","INC","LIB","SYS","DIR"};
    QString res;

    bool exist = false;
    node = nullptr;
    line = 0;
    col = 0;
    if (href.length() < 5) return res;
    QString code = href.left(3);
    int iCode = tags.indexOf(code);
    QStringList parts = href.right(href.length()-4).split(',');
    if (iCode >= 0) {
        if (iCode < 2) {
            QString lstFile = parameter(code.at(2) == '2' ? "ls2" : "lst");
            exist = QFile(lstFile).exists();
            if (exist) res = lstFile;
            if (!create || !exist) return res;
            line = parts.first().toInt();
            node = projectRepo()->findOrCreateFileNode(lstFile, this, &FileType::from(FileKind::Lst));

        } else {
            QString fName = parts.first();
            if (fName.startsWith("\"") && fName.endsWith("\""))
                fName = fName.mid(1, fName.length()-2);
            QStringList locations;
            if (iCode == 2) { // INC
                locations << workDir();
                QString inDir;
                emit getParameterValue("InputDir", inDir);
                if (inDir.isNull()) emit getParameterValue("IDir", inDir);
                if (!inDir.isNull()) {
                    // check if there are joined paths
                    locations << QDir::fromNativeSeparators(inDir).split(QDir::listSeparator(), Qt::SkipEmptyParts);
                } else {
                    emit getParameterValue("InputDir*", inDir);
                    if (inDir.isNull()) emit getParameterValue("IDir*", inDir);
                    if (!inDir.isNull()) {
                        // there is at least one inputDir with number -> read all
                        for (int i = 1; i <= 40 ; ++i) {
                            QString inDirX;
                            emit getParameterValue(QString("InputDir%1").arg(i), inDirX);
                            if (inDirX.isNull()) emit getParameterValue(QString("IDir%1").arg(i), inDirX);
                            if (!inDirX.isNull()) {
                                locations << QDir::fromNativeSeparators(inDirX);
                            }
                        }
                    }
                }

            } else if (iCode == 5) { // DIR
                QString path = parts.first();
                if (path.startsWith("\"") && path.endsWith("\""))
                    path = path.mid(1, path.length()-2);
                return path;

            } else if (iCode == 4) { // SYS
                QString sysDir;
                emit getParameterValue("sysIncDir", sysDir);
                if (sysDir.isNull()) emit getParameterValue("SDir", sysDir);
                QDir dir(sysDir);
                if (!sysDir.isNull()) {
                    if (dir.isAbsolute()) {
                        locations << QDir::fromNativeSeparators(sysDir);
                    } else {
                        locations << CommonPaths::systemDir() + '/' + sysDir;
                    }
                } else
                    locations << CommonPaths::systemDir();

            } else { // LIB
                QString libDir;
                emit getParameterValue("libIncDir", libDir);
                if (libDir.isNull()) emit getParameterValue("LDir", libDir);

                if (!libDir.isNull()) {
                    libDir = QDir::fromNativeSeparators(libDir);
                    QDir dir(libDir);
                    if (dir.isAbsolute()) {
                        locations << libDir;
                    } else {
                        locations << CommonPaths::systemDir() + '/' + libDir;
                    }
                }
                for (QString &path: CommonPaths::gamsStandardPaths(CommonPaths::StandardDataPath))
                    locations << path + "/inclib";
            }
            QString rawName = fName;
            for (QString loc : std::as_const(locations)) {
                if (!QDir::isAbsolutePath(loc))
                    loc = workDir() + '/' + loc;
                fName = loc + '/' + rawName;
                QFileInfo file(fName);
                exist = file.exists() && file.isFile();
                if (!exist) {
                    file.setFile(file.filePath()+".gms");
                    exist = file.exists() && file.isFile();
                    if (exist) fName += ".gms";
                }
                if (exist) break;
            }
            if (exist) res = fName;
            if (!create || !exist) return res;
            node = projectRepo()->findOrCreateFileNode(fName, this);
        }
    } else if (parts.first().startsWith('"')) {
        QString fName = parts.first().mid(1, parts.first().length()-2);
        if (!QDir::isAbsolutePath(fName))
            fName = workDir() + '/' + fName;

        exist = QFile(fName).exists();
        if (exist) res = fName;
        if (!create || !exist) return res;
        node = projectRepo()->findOrCreateFileNode(fName, this);
        if (parts.size() > 1) line = parts.at(1).toInt();
        if (parts.size() > 2) col = parts.at(2).toInt();
    }
    return res;
}

void PExProjectNode::setVerbose(bool verbose)
{
    mVerbose = verbose;
    if (mDebugServer)
        mDebugServer->setVerbose(mVerbose);
}

QString PExProjectNode::engineJobToken() const
{
    return mEngineJobToken;
}

void PExProjectNode::setEngineJobToken(const QString &engineJobToken, bool touch)
{
    mEngineJobToken = engineJobToken;
    if (touch)
        setNeedSave();
}

QString PExProjectNode::tempGdx() const
{
    return mTempGdx;
}

debugger::Server *PExProjectNode::debugServer() const
{
    return mDebugServer;
}

PExProjectNode::Type PExProjectNode::type() const
{
    return mType;
}

bool PExProjectNode::needSave() const
{
    return mChangeState == csChanged;
}

bool PExProjectNode::isClosing() const
{
    return mChangeState == csClosing;
}

void PExProjectNode::setNeedSave(bool needSave)
{
    if (mChangeState != csClosing && type() != PExProjectNode::tSearch) {
        mChangeState = needSave ? csChanged : csNone;
        emit changed(id());
    }
}

void PExProjectNode::setIsClosing()
{
    mChangeState = csClosing;
}

void PExProjectNode::updateLogName(const QString &name)
{
    if (mLogNode) {
        QString suffix = FileType::from(FileKind::Log).defaultSuffix();
        QString logName = workDir() + "/" + name + nameExt() + "." + suffix;
        mLogNode->file()->setLocation(logName);
        if (mLogNode->file()->editors().size())
            emit projectRepo()->logTabRenamed(mLogNode->file()->editors().first(), mLogNode->file()->name());
    }
}

void PExProjectNode::refreshProjectTabName()
{
    if (mProjectEditFileMeta) {
        for (QWidget *wid : mProjectEditFileMeta->editors())
            emit projectRepo()->refreshProjectTabName(wid);
    }
}

PExLogNode *PExProjectNode::logNode()
{
    if (!mLogNode) {
        QString suffix = FileType::from(FileKind::Log).defaultSuffix();
        QString logName = workDir()+"/"+name()+"."+suffix;
        FileMeta* fm = fileRepo()->findOrCreateFileMeta(logName, &FileType::from(FileKind::Log));
        mLogNode = new PExLogNode(fm, this);
    }
    return mLogNode;
}

///
/// \brief PExGroupNode::setLogLocation sets the location of the log. Filename can be determined automatically from path.
/// \param path
///
void PExProjectNode::setLogLocation(QString path)
{
    if (path.contains('\\'))
        path = QDir::fromNativeSeparators(path);
    QFileInfo log(path);
    QString fullPath = log.filePath();

    if (log.isDir()) {
        if (!fullPath.endsWith('/'))
            fullPath.append('/');

        QFileInfo filename(mLogNode->file()->location());
        fullPath.append(filename.completeBaseName() + "." + FileType::from(FileKind::Log).defaultSuffix());
    }

    mLogNode->file()->setLocation(fullPath);
    updateLogName(log.completeBaseName());
}

FileMeta* PExProjectNode::runnableGms() const
{
    FileMetaRepo *repo = fileRepo();
    if (!repo) return nullptr;
    return repo->fileMeta(parameter("gms"));
}

void PExProjectNode::setRunnableGms(FileMeta *gmsFile)
{
    if (mType > PExProjectNode::tCommon) return;
    QList<NodeId> ids;
    PExFileNode *gmsFileNode = nullptr;
    if (!gmsFile) {
        // find alternative runable file
        for (PExAbstractNode *node: listFiles()) {
            gmsFileNode = node->toFile();
            if (gmsFileNode->file()->kind() == FileKind::Gms) {
                gmsFile = gmsFileNode->file();
                break;
            }
        }
    } else {
        gmsFileNode = findFile(gmsFile);
    }
    if (gmsFile && gmsFile->kind() != FileKind::Gms) {
        DEB() << "Only files of FileKind::Gms can become runable";
        return;
    }

    ids << id();
    if (gmsFileNode) ids << gmsFileNode->id();
    if (!parameter("gms").isEmpty()) {
        if (PExFileNode *oldMain = findFile(parameter("gms")))
            ids << oldMain->id();
    }
    setParameter("gms", "");
    if (!gmsFile) {
        setParameter("lst", "");
        for (NodeId id : ids) {
            emit changed(id);
        }
        emit runnableChanged();
        return;
    }
    if (workDir().isEmpty())
        setLocation(QFileInfo(gmsFile->location()).absoluteDir().path());

    QString gmsPath = gmsFile->location();
    setParameter("gms", gmsPath);
    if (hasLogNode()) logNode()->resetLst();

    for (NodeId id : ids) {
        emit changed(id);
    }
    emit runnableChanged();
}

bool PExProjectNode::hasParameterFile()
{
    return mParameterHash.contains("pf");
}

FileMeta *PExProjectNode::parameterFile() const
{
    FileMetaRepo *repo = fileRepo();
    if (!repo) return nullptr;
    return repo->fileMeta(parameter("pf"));
}

void PExProjectNode::setParameterFile(FileMeta *pfFile)
{
    if (mType > PExProjectNode::tCommon) return;
    if (pfFile && pfFile->kind() != FileKind::Pf) {
        DEB() << "Only files of FileKind::Pf can become a parameter file";
        return;
    }
    QString pfPath = pfFile ? pfFile->location() : "";
    setParameterFile(pfPath);

    emit changed(id());
}

void PExProjectNode::setParameterFile(const QString &fileName)
{
    setParameter("pf", fileName);
    emit changed(id());
}

FileMeta *PExProjectNode::projectEditFileMeta() const
{
    return mProjectEditFileMeta;
}

void PExProjectNode::setProjectEditFileMeta(FileMeta *prOptMeta)
{
    if (prOptMeta == mProjectEditFileMeta) return;
    if (mProjectEditFileMeta) delete mProjectEditFileMeta;
    mProjectEditFileMeta = prOptMeta;
}

void PExProjectNode::unlinkProjectEditFileMeta()
{
    mProjectEditFileMeta = nullptr;
}

QString PExProjectNode::mainModelName(bool stripped) const
{
    FileMeta *fileMeta = runnableGms();

    if (!fileMeta) {
        SysLogLocator::systemLog()->append(QString("Could not find a runable gms file for project: %1")
                .arg(name()), LogMsgType::Error);
        return QString();
    }

    QFileInfo fileInfo(fileMeta->name());
    if (stripped)
        return fileInfo.completeBaseName();
    return fileInfo.fileName();

}

QString PExProjectNode::errorText(int lstLine)
{
    return mErrorTexts.value(lstLine);
}

void PExProjectNode::setErrorText(int lstLine, const QString &text)
{
    if (text.isEmpty()) {
        DEB() << "Empty LST-text ignored for line " << lstLine;
        return;
    }
    if (mErrorTexts.contains(lstLine)) {
        mErrorTexts.insert(lstLine, mErrorTexts.value(lstLine)+"\n"+text);
    } else {
        mErrorTexts.insert(lstLine, text);
    }
}

void PExProjectNode::hasHRef(const QString &href, QString &fileName)
{
    PExFileNode *node;
    int line;
    int column;
    fileName = resolveHRef(href, node, line, column);
}

void PExProjectNode::jumpToHRef(const QString &href)
{
    PExFileNode *node;
    int line;
    int column;
    QString file = resolveHRef(href, node, line, column, true);
    if (node) node->file()->jumpTo(node->projectId(), true, line-1, column);
    else if (href.startsWith("DIR:")) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(file));
    }
}

void PExProjectNode::createMarks(const LogParser::MarkData &marks)
{
    if (marks.hasErr() && !marks.hRef.isEmpty()) {
        int col;
        PExFileNode *errNode;
        int errLine;
        PExFileNode *lstNode;
        int lstLine;
        resolveHRef(marks.hRef, lstNode, lstLine, col, true);
        resolveHRef(marks.errRef, errNode, errLine, col, true);
        TextMark *errMark = nullptr;
        TextMark *lstMark = nullptr;
        if (errNode)
            errMark = textMarkRepo()->createMark(errNode->file()->id(), id(), TextMark::error, lstLine, errLine-1, col-1, 1);
        if (lstNode) {
            lstMark = textMarkRepo()->createMark(lstNode->file()->id(), id(), TextMark::link, lstLine, lstLine-1, -1, 1);
            if (errMark) {
                errMark->setValue(lstLine);
                errMark->setRefMark(lstMark);
                lstMark->setRefMark(errMark);
            }
        }
    }
}

void PExProjectNode::switchLst(const QString &lstFile)
{
    if (mParameterHash.contains("lst")) { // TODO:LST
        setParameter("ls2", lstFile);
    }
}

void PExProjectNode::registerGeneratedFile(const QString &fileName)
{
    if (fileRepo())
        fileRepo()->setAutoReload(QDir::fromNativeSeparators(fileName));
}

void insertSorted(QList<int> &list, int value)
{
    int index = 0;
    while (list.size() > index && list.at(index) < value)
        ++index;
    if (list.size() == index || list.at(index) > value)
        list.insert(index, value);
}

void PExProjectNode::addBreakpoint(const QString &filename, int line)
{
    bool isRunning = mGamsProcess && mGamsProcess.get()->state() != QProcess::NotRunning;
    int resLine = mBreakpointData->addBreakpoint(filename, line, isRunning);

    if (mDebugServer) {
        int contLine = mBreakpointData->continuousLine(filename, resLine);
        if (contLine >= 0)
            mDebugServer->addBreakpoint(contLine);
    }
}

void PExProjectNode::delBreakpoint(const QString &filename, int line)
{
    mBreakpointData->delBreakpoint(filename, line);

    if (mDebugServer) {
        int contLine = mBreakpointData->continuousLine(filename, line);
        if (contLine >= 0)
            mDebugServer->delBreakpoint(contLine);
    }
}

void PExProjectNode::clearBreakpoints()
{
    mBreakpointData->delBreakpoints();
    if (mDebugServer)
        mDebugServer->clearBreakpoints();
    for (const PExFileNode *node : listFiles())
        node->file()->updateBreakpoints();
}

void PExProjectNode::breakpoints(const QString &filename, SortedIntMap &bps, SortedIntMap &aimedBps) const
{
    bps = mBreakpointData->bpFileLines(filename);
    aimedBps = mBreakpointData->bpAimedFileLines(filename);
}

void PExProjectNode::clearErrorTexts()
{
    mErrorTexts.clear();
}

bool PExProjectNode::hasErrorText(int lstLine)
{
    return (lstLine < 0) ? mErrorTexts.size() > 0 : mErrorTexts.contains(lstLine);
}

void PExProjectNode::setRunFileParameterHistory(FileId fileId, QStringList parameterLists)
{
    mRunFileParameters.insert(fileId, parameterLists);
}

QStringList PExProjectNode::runFileParameterHistory(FileId fileId) const
{
    return mRunFileParameters.value(fileId, QStringList());
}

void PExProjectNode::addRunParametersHistory(const QString &runParameters)
{
    FileId id;
    if (Settings::settings()->toBool(skOptionsPerMainFile))
        if (FileMeta *meta = runnableGms())
            id = meta->id();
    if (!runParameters.simplified().isEmpty()) {
       QStringList list = mRunFileParameters[id].filter(runParameters.simplified());
       if (list.size() > 0) {
           mRunFileParameters[id].removeOne(runParameters.simplified());
       }
    } else {
        for (int i = 0; i < mRunFileParameters[id].size(); ++i) {
            QString str = mRunFileParameters[id].at(i);
            if (str.simplified().isEmpty()) {
                mRunFileParameters[id].removeAt(i);
                break;
            }
        }
    }
    mRunFileParameters[id].append(runParameters.simplified());
    if (id.isValid()) {
        // update the project global parameters list
        mRunFileParameters.insert(FileId(), runFileParameterHistory(id));
    }
}

QStringList PExProjectNode::getRunParametersHistory() const
{
    FileId id;
    if (Settings::settings()->toBool(skOptionsPerMainFile))
        if (FileMeta *meta = runnableGms())
            id = meta->id();
    QStringList params = mRunFileParameters.value(id, QStringList());
    if (params.size() == 1 && params.at(0).isEmpty())
        params.clear();
    if (params.isEmpty())
        return mRunFileParameters.value(FileId(), QStringList());
    return params;
}

///
/// \brief PExProjectNode::analyzeParameters translates the gms file and an OptionItem list into a single QStringList, while also setting all extracted parameters for this project
/// \param gmsLocation gms file to be run
/// \param itemList list of options given by studio and user
/// \return QStringList all arguments
///
QStringList PExProjectNode::analyzeParameters(const QString &gmsLocation, const QStringList &defaultParameters, const QList<option::OptionItem> &itemList, option::Option *opt, int &logOption)
{
    bool ok;
    // set studio default parameters
    QMultiMap<int, QString> gamsArguments;
    QStringList defaultArgumentList;
    QStringList defaultArgumentValueList;
    QList<int> defaultArgumentIdList = QList<int>();
    defaultArgumentList      << "logoption" << "ide" << "errorlog" << "errmsg" << "pagesize" << "lstTitleleftaligned" ;
    defaultArgumentValueList << "3"         << "1"   << "99"       << "1"      << "0"        << "1"                   ;
    int position = 0;
    for (const QString &arg : std::as_const(defaultArgumentList)) {
        defaultArgumentIdList << opt->getOrdinalNumber(arg);
        if (defaultArgumentIdList.last() == opt->getOrdinalNumber("logoption")) {
            int lo = defaultArgumentValueList.at(position).toInt(&ok);
            if (ok) logOption = lo;
        }
        gamsArguments.insert(defaultArgumentIdList.last(), defaultArgumentValueList.at(position++));
    }

    for (const QString &param: defaultParameters) {
        QStringList list = param.split("=", Qt::SkipEmptyParts);
        if (list.count() != 2)
            continue;
        defaultArgumentList << list[0];
        defaultArgumentValueList << list[1];
        defaultArgumentIdList << opt->getOrdinalNumber(list[0]);
    }

    // find directory changes first
    QString path = "";
    QString cdir = "";
    QString wdir = "";
    QString filestem = "";
    QStringList argumentList;
    QStringList argumentValueList;
    QList<int> argumentIdList = QList<int>();
    for (const option::OptionItem &item : itemList) {
        argumentList      << item.key;
        argumentIdList    << item.optionId;
        argumentValueList << item.value;
        gamsArguments.insert(item.optionId, item.value);
        if (item.optionId == opt->getOrdinalNumber("curdir")) {
            cdir = item.value;
        } else if (item.optionId == opt->getOrdinalNumber("workdir")) {
            wdir = item.value;
        } else if (item.optionId == opt->getOrdinalNumber("filestem")) {
            filestem = item.value;
        }
    }

    if (!cdir.isEmpty()) path = cdir;

    // wdir replaces cdir for output files
    // if wdir is relative, it is appended to cdir
    if (!wdir.isEmpty()) {
        if (!cdir.isEmpty() && QDir(wdir).isRelative())
            path += '/' + wdir;
        else path = wdir;
    }

    QFileInfo fi(gmsLocation);
    if (filestem.isEmpty()) filestem = fi.completeBaseName();
    if (path.isEmpty()) path = workDir();
    else if (QDir(path).isRelative()) path = workDir() + '/' + path;

    setLogLocation(cleanPath(path, name() + "." + FileType::from(FileKind::Log).defaultSuffix()));

    clearParameters();
    // set default lst name to revert deleted o parameter values
    setParameter("lst", cleanPath(path, filestem + ".lst"));

    bool defaultOverride = false;
    // iterate options
    for (const option::OptionItem &item : itemList) {
        // convert to native seperator
        QString value = item.value;
        value = value.replace('\\', '/');

        // regex to remove dots at the end of a filename
        QRegularExpression notDotAsEnding("[\\w\\d](\\.)[\"\\\\ ]*$");
        QRegularExpressionMatch match = notDotAsEnding.match(value);
        if (match.hasMatch()) value = value.remove(match.capturedStart(1), match.capturedLength(1));

        // set parameters
        if (item.optionId != -1) {
            if (item.optionId == opt->getOrdinalNumber("output")) {
                mParameterHash.remove("lst"); // remove default
                if (!(QString::compare(value, "nul", Qt::CaseInsensitive) == 0
                      || QString::compare(value, "/dev/null", Qt::CaseInsensitive) == 0))
                    setParameter("lst", cleanPath(path, value));

            } else if (item.optionId == opt->getOrdinalNumber("GDX")) {
                if (value == "default") value = "\"" + filestem + ".gdx\"";
                setParameter("gdx", cleanPath(path, value));

            } else if (item.optionId == opt->getOrdinalNumber("Reference")) {
                if (value == "default") value = "\"" + filestem + ".ref\"";
                setParameter("ref", cleanPath(path, value));

            } else if (item.optionId == opt->getOrdinalNumber("ParmFile")) {
                if (value == "default") value = "\"" + filestem + ".pf\"";
                setParameter("pf", value.isEmpty() ? "" : cleanPath(path, value));

            } else if (item.optionId == opt->getOrdinalNumber("logoption")) {
                int lo = item.value.toInt(&ok);
                if (ok) logOption = lo;

            } else if (item.optionId == opt->getOrdinalNumber("logfile")) {
                if (!value.endsWith(".log", FileType::fsCaseSense()))
                    value.append("." + FileType::from(FileKind::Log).defaultSuffix());
                setLogLocation(cleanPath(path, value));
            }

            if (defaultArgumentIdList.contains(item.optionId))
                defaultOverride = true;
        }
    }

    if (defaultOverride)
        SysLogLocator::systemLog()->append("You are overwriting at least one GAMS Studio default argument. "
                     "Some of these are necessary to ensure a smooth experience. "
                     "Use at your own risk!", LogMsgType::Warning);

    // prepare gams command
    QStringList output { CommonPaths::nativePathForProcess(gmsLocation) };
    // normalize gams parameter format
    position = 0;
    for(int id : std::as_const(defaultArgumentIdList)) {
        if (defaultArgumentIdList.contains(id))
            output.append( QString("%1=%2").arg(defaultArgumentList.at(position), defaultArgumentValueList.at(position)));
        else
            output.append( QString("%1=%2").arg(defaultArgumentList.at(position), gamsArguments.value(id)));
        position++;
    }
    position = 0;
    for(const option::OptionItem &item : itemList) {
        if (item.recurrent) {
            if (item.recurrentIndices.first() == position) {
                output.append( QString("%1=%2").arg(argumentList.at(position), gamsArguments.value(item.optionId)));
            }
        } else {
            output.append( QString("%1=%2").arg(argumentList.at(position), item.value));
        }
        position++;
    }
    return output;
}

void PExProjectNode::setLocation(const QString &newLocation)
{
    // check if changed more than letter case
    bool changed = location().isEmpty() || location().compare(newLocation, FileType::fsCaseSense()) != 0;
    PExGroupNode::setLocation(newLocation);
    if (changed)
        emit baseDirChanged(this);
}

void PExProjectNode::setWorkDir(const QString &workingDir)
{
    mWorkDir = workingDir;
}

QString PExProjectNode::workDir() const
{
    return mWorkDir;
}

///
/// \brief PexProjectNode::normalizePath removes quotes and trims whitespaces for use within studio. do not pass to gams!
/// \param path working dir
/// \param file file name, can be absolute or relative
/// \return cleaned path
///
QString PExProjectNode::cleanPath(QString path, QString file) {

    QString ret = "";

    path.remove("\""); // remove quotes from path
    QDir dir(path);
    if (dir.isRelative()) {
        path = workDir() + '/' + path;
    }

    file.remove("\""); // remove quotes from filename
    file = file.trimmed();
    if (file.isEmpty() || QFileInfo(file).isRelative()) {
        ret = path;
        if (! ret.endsWith('/'))
            ret += '/';
    }
    ret.append(file);

    return QFileInfo(ret).absoluteFilePath();
}

bool PExProjectNode::isProcess(const AbstractProcess *process) const
{
    return process && mGamsProcess.get() == process;
}

bool PExProjectNode::jumpToFirstError(bool focus, PExFileNode* lstNode)
{
    if (!runnableGms()) return false;
//    QList<TextMark*> marks = textMarkRepo()->marks(runnableGms()->id(), -1, id(), TextMark::error, 1);
//    TextMark* textMark = marks.size() ? marks.first() : nullptr;
    TextMark* textMark = textMarkRepo()->marks(runnableGms()->id())->firstError(id());

    if (textMark) {
        if (Settings::settings()->toBool(skOpenLst)) {
            textMark->jumpToMark(false);
            textMark->jumpToRefMark(focus);
        } else {
            if (lstNode && !lstNode->file()->editors().isEmpty()) textMark->jumpToRefMark(false);
            textMark->jumpToMark(focus);
        }
        // jump to first error in LOG
        QVector<TextMark*> backRef = textMark->backRefs(logNode()->file()->id());
        if (backRef.size()) backRef.at(0)->jumpToMark(false, true);
        return true;
    }
    return false;
}

void PExProjectNode::errorTexts(const QVector<int> &lstLines, QStringList &result)
{
    for (int lstLine: lstLines) {
        QString newTip = errorText(lstLine);
        if (!result.contains(newTip))
            result << newTip;
    }
}

QString PExProjectNode::parameter(const QString &kind) const
{
    return mParameterHash.value(kind);
}

bool PExProjectNode::hasParameter(const QString &kind) const
{
    return mParameterHash.contains(kind);
}

void PExProjectNode::addNodesForSpecialFiles()
{
    FileMeta* runFile = runnableGms();
    for (auto it = mParameterHash.constBegin(); it != mParameterHash.constEnd(); ++it) {
        const QString &loc = it.value();
        if (loc.isEmpty())
            continue;
        if (QFileInfo::exists(loc)) {
            PExFileNode* node = projectRepo()->findOrCreateFileNode(loc, this, &FileType::from(QFileInfo(loc).fileName()));
            QString encoding = node->file()->encoding();
            node->file()->setEncoding(runFile ? runFile->encoding()
                                              : Settings::settings()->toString(skDefaultEncoding));
            if (encoding != node->file()->encoding())
                setNeedSave();
        } else {
            SysLogLocator::systemLog()->append("Could not add file " + loc, LogMsgType::Error);
        }
    }
}

void PExProjectNode::setParameter(const QString &kind, const QString &path)
{
    if (path.isEmpty()) {
        if (kind == "pf")
            mParameterHash.insert(kind, "");
        else
            mParameterHash.remove(kind);
        return;
    }
    QString fullPath = path.contains('\\') ? QDir::fromNativeSeparators(path) : path;
    if (QFileInfo(fullPath).isRelative())
        fullPath = QFileInfo(workDir()).canonicalFilePath() + "/" + fullPath;

    fullPath.remove("\"");

    if (QFileInfo(fullPath).suffix().isEmpty()) {
        if (kind == "gdx")
            fullPath += ".gdx";
        else if (kind == "lst" || kind == "ls2")
        { /* do nothing */ } // gams does not add lst extension. unlike .ref or .gdx
        else if (kind == "ref")
            fullPath += ".ref";
        else
            DEB() << "WARNING: unhandled parameter! " << fullPath << " is missing extension.";
    }
    if (mParameterHash.value(kind) != fullPath) {
        mParameterHash.insert(kind, fullPath);
        setNeedSave();
    }
}

void PExProjectNode::clearParameters()
{
    bool willChange = mParameterHash.size() != 1 || !mParameterHash.contains("gms");
    QString gms = mParameterHash.value("gms");
    QString pf = mParameterHash.value("pf");
    mParameterHash.clear();
    mParameterHash.insert("gms", gms);
    mParameterHash.insert("pf", pf);
    if (willChange) setNeedSave();
}

bool PExProjectNode::startDebugServer(debugger::DebugStartMode mode)
{
    if (!mDebugServer) {
        mDebugServer = new debugger::Server(workDir(), this);
        mDebugServer->setVerbose(mVerbose);
        connect(mDebugServer, &debugger::Server::connected, this, [this]() {
            mBreakpointData->clearLinesMap();
        });
        connect(mDebugServer, &debugger::Server::signalMapDone, this, [this, mode]() {
            mBreakpointData->adjustBreakpoints();
            for (const QString &file : mBreakpointData->bpFiles()) {
                FileMeta *meta = fileRepo()->fileMeta(file);
                if (meta)
                    meta->updateBreakpoints();
            }
            mDebugServer->addBreakpoints(mBreakpointData->bpContinuousLines());
            if (mode == debugger::RunDebug)
                mDebugServer->sendRun();
            else
                mDebugServer->sendStepLine();
        });
        connect(mDebugServer, &debugger::Server::addProcessLog, this, &PExProjectNode::addProcessLog);
        connect(mDebugServer, &debugger::Server::signalGdxReady, this, &PExProjectNode::openDebugGdx);
        connect(mDebugServer, &debugger::Server::signalPaused, this, &PExProjectNode::gotoPaused);
        connect(mDebugServer, &debugger::Server::signalStop, this, &PExProjectNode::terminate);
        connect(mDebugServer, &debugger::Server::signalLinesMap, this, &PExProjectNode::addLinesMap);
        if (process())
            connect(process(), &AbstractProcess::interruptGenerated, mDebugServer, &debugger::Server::sendStepLine);
    }
    bool res = mDebugServer->start();
    if (process()) {
        QStringList params = process()->parameters();
        params << ("DebugPort=" + QString::number(mDebugServer->port()));
        process()->setParameters(params);
    } else {
        DEB() << "Process isn't up already.";
    }
    return res;
}

void PExProjectNode::stopDebugServer()
{
    if (mDebugServer) {
        disconnect(mDebugServer);
        debugger::Server *server = mDebugServer;
        mDebugServer = nullptr;
        delete server;
    }
    mBreakpointData->resetAimedBreakpoints();
    for (const PExFileNode *node : listFiles())
        node->file()->updateBreakpoints();
}

QProcess::ProcessState PExProjectNode::gamsProcessState() const
{
    return mGamsProcess ? mGamsProcess->state() : QProcess::NotRunning;
}

QString PExProjectNode::tooltip()
{
    QString res(QDir::toNativeSeparators(fileName()));
    if (mType <= PExProjectNode::tCommon)
        res.append( "\n\nBase directory: " + QDir::toNativeSeparators(location()) +
                    "\nWorking directory: " + QDir::toNativeSeparators(workDir()));
    else if (mType == PExProjectNode::tSearch)
        res.append( "\n\nResults from searching in\n" + QDir::toNativeSeparators(location()));
    else
        res.append( "\n\nContaining special GAMS system files");
    if (runnableGms()) res.append("\nMain GMS file: ").append(runnableGms()->name());
    if (!parameter("lst").isEmpty())
        res.append("\nLast output file: ").append(QFileInfo(parameter("lst")).fileName());
    if (!parameter("ls2").isEmpty())
        res.append("\nadditional output: ").append(QFileInfo(parameter("ls2")).fileName());
    if (debugMode()) {
        res.append("\nNodeId: "+QString::number(id()));
        res.append("\nParent-NodeId: " + (parentNode() ? QString::number(parentNode()->id()) : "?"));
    }
    return res;
}

void PExProjectNode::onGamsProcessStateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState)
    emit gamsProcessStateChanged(this);
}

void PExProjectNode::openDebugGdx(const QString &gdxFile)
{
    QDir dir(mWorkDir);
    QString absFile = dir.absoluteFilePath(gdxFile);
    PExFileNode *node = projectRepo()->findOrCreateFileNode(absFile, this);
    if (node) {
        if (node->file()->isOpen())
            node->file()->invalidate();
        node->file()->jumpTo(node->projectId(), false);
        if (!node->file()->editors().isEmpty())
            emit openInPinView(this, node->file()->editors().first());
    } else
        DEB() << "GDX not found: " << absFile;
}

void PExProjectNode::gotoPaused(int contLine)
{
    QString file = mBreakpointData->filename(contLine);
    int fileLine = mBreakpointData->fileLine(contLine);

    PExFileNode *node = projectRepo()->findOrCreateFileNode(file, this);
    if (mPausedInFile && mPausedInFile != node) {
        for (QWidget *wid : mPausedInFile->file()->editors()) {
            if (CodeEdit * ce = ViewHelper::toCodeEdit(wid))
                ce->setPausedPos(-1);
        }
    }
    if (node) {
        if (!node->file()->isOpen())
            openFileNode(node);
        for (QWidget *wid : node->file()->editors()) {
            if (CodeEdit * ce = ViewHelper::toCodeEdit(wid))
                ce->setPausedPos(fileLine-1);
        }
        if (!node->file()->editors().isEmpty())
            emit switchToTab(node->file());
    }
    mPausedInFile = node;
    if (contLine > 0) {
        mTempGdx = mDebugServer->gdxTempFile();
        mDebugServer->sendWriteGdx(QDir::toNativeSeparators(mTempGdx));
    }
}

void PExProjectNode::terminate()
{
    std::ignore = QtConcurrent::run(&AbstractProcess::terminate, process());
}

void PExProjectNode::processState(QProcess::ProcessState &state)
{
    state = gamsProcessState();
}

void PExProjectNode::addLinesMap(const QString &filename, const QList<int> &fileLines, const QList<int> &continuousLines)
{
    mBreakpointData->addLinesMap(filename, fileLines, continuousLines);
}

PExRootNode::PExRootNode(ProjectRepo* repo)
    : PExGroupNode("Root", "", NodeType::root), mRepo(repo)
{
    if (!mRepo) EXCEPT() << "The ProjectRepo must not be null";
}

void PExRootNode::setParentNode(PExGroupNode *parent)
{
    Q_UNUSED(parent)
    EXCEPT() << "The root node has no parent";
}

ProjectRepo *PExRootNode::projectRepo() const
{
    return mRepo;
}

FileMetaRepo *PExRootNode::fileRepo() const
{
    return mRepo ? mRepo->fileRepo() : nullptr;
}

TextMarkRepo *PExRootNode::textMarkRepo() const
{
    return mRepo ? mRepo->textMarkRepo() : nullptr;
}

PExProjectNode *PExRootNode::findProject(const QString &projectFile) const
{
    for (PExAbstractNode *node : childNodes()) {
        PExProjectNode *project = node->toProject();
        if (project && project->fileName().compare(projectFile, FileType::fsCaseSense()) == 0)
            return project;
    }
    return nullptr;
}

} // namespace studio
} // namespace gams
