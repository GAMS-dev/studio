/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "process.h"
#include "commonpaths.h"
#include "logger.h"
#include "syntax.h"
#include "option/option.h"
#include "editors/sysloglocator.h"
#include "settings.h"
#include <QFileInfo>
#include <QDir>

namespace gams {
namespace studio {

ProjectGroupNode::ProjectGroupNode(QString name, QString location, NodeType type)
    : ProjectAbstractNode(name, type)
{
    setLocation(location);
}

ProjectGroupNode::~ProjectGroupNode()
{
    if (mChildNodes.size())
        DEB() << "Group must be empty before deletion";
}

QIcon ProjectGroupNode::icon(QIcon::Mode mode, int alpha)
{
    return Theme::icon(":/img/folder-open", mode, alpha);
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

void ProjectGroupNode::appendChild(ProjectAbstractNode* child)
{
    if (!child || mChildNodes.contains(child)) return;
    mChildNodes.append(child);
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
    mLocation = mLocation.contains('\\') ? QDir::fromNativeSeparators(location) : location;
    emit changed(id());
}

QString ProjectGroupNode::tooltip()
{
    QString res = QDir::toNativeSeparators(location());
    if (debugMode()) {
        res.append("\nNodeId: "+QString::number(id()));
        res.append("\nParent-NodeId: " + (parentNode() ? QString::number(parentNode()->id()) : "?"));
    }
    return QString(res);
}

QString ProjectGroupNode::errorText(int lstLine)
{
    return parentNode() ? parentNode()->errorText(lstLine) : QString();
}

ProjectFileNode *ProjectGroupNode::findFile(QString location, bool recurse) const
{
    if (location.contains('\\')) location = QDir::fromNativeSeparators(location);
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

QList<ProjectFileNode*> ProjectGroupNode::findFiles(FileKind kind, bool recurse) const
{
    QList<ProjectFileNode*> res;
    for (ProjectAbstractNode* node: mChildNodes) {
        ProjectFileNode* fileNode = node->toFile();
        if (fileNode && fileNode->file()->kind() == kind) res << fileNode;
        if (recurse) {
            const ProjectGroupNode* group = node->toGroup();
            res << (group ? group->findFiles(kind, true) : QList<ProjectFileNode*>());
        }
    }
    return res;
}

ProjectRunGroupNode *ProjectGroupNode::findRunGroup(const AbstractProcess *process) const
{
    for (ProjectAbstractNode* node: childNodes()) {
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
    for (ProjectAbstractNode* node: childNodes()) {
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

void ProjectGroupNode::moveChildNode(int from, int to)
{
    mChildNodes.move(from, to);
}

void ProjectGroupNode::hasFile(QString fName, bool &exists)
{
    exists = findFile(fName);
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

void ProjectRunGroupNode::setProcess(std::unique_ptr<AbstractProcess> process)
{
    if (mGamsProcess == process) return;
    mGamsProcess->disconnect();
    mGamsProcess = std::move(process);
    connect(mGamsProcess.get(), &GamsProcess::stateChanged, this, &ProjectRunGroupNode::onGamsProcessStateChanged);
}

AbstractProcess *ProjectRunGroupNode::process() const
{
    return mGamsProcess.get();
}

QIcon ProjectRunGroupNode::icon(QIcon::Mode mode, int alpha)
{
    if (gamsProcessState() == QProcess::NotRunning)
        return ProjectGroupNode::icon(mode, alpha);
    return projectRepo()->runAnimateIcon(mode, alpha);
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

void ProjectRunGroupNode::appendChild(ProjectAbstractNode *child)
{
    ProjectGroupNode::appendChild(child);
    ProjectFileNode *file = child->toFile();
    if (!mParameterHash.contains("gms") && file && file->file() && file->file()->kind() == FileKind::Gms) {
        setRunnableGms(file->file());
    }
}

void ProjectRunGroupNode::removeChild(ProjectAbstractNode *child)
{
    ProjectGroupNode::removeChild(child);
    bool gmsLost = false;
    ProjectFileNode *file = child->toFile();
    if (file) {
        QList<QString> files = mParameterHash.keys(file->location());
        for (const QString &file: files) {
            mParameterHash.remove(file);
            if (file=="gms") gmsLost = true;
        }
    }
    if (gmsLost) {
        setRunnableGms();
    }
}

QString ProjectRunGroupNode::resolveHRef(QString href, ProjectFileNode *&node, int &line, int &col, bool create)
{
    const QStringList tags {"LST","LS2","INC","LIB","SYS"};
    QString res;

    bool exist = false;
    node = nullptr;
    line = 0;
    col = 0;
    if (href.length() < 5) return res;
    QString code = href.left(3);
    int iCode = tags.indexOf(code);
    QVector<QStringRef> parts = href.rightRef(href.length()-4).split(',');
    if (iCode >= 0) {
        if (iCode < 2) {
            QString lstFile = parameter(code.at(2) == '2' ? "ls2" : "lst");
            exist = QFile(lstFile).exists();
            if (exist) res = lstFile;
            if (!create || !exist) return res;
            line = parts.first().toInt();
            node = projectRepo()->findOrCreateFileNode(lstFile, this, &FileType::from(FileKind::Lst));

        } else {
            QString fName = parts.first().toString();
            if (fName.startsWith("\"") && fName.endsWith("\""))
                fName = fName.mid(1, fName.length()-2);
            QStringList locations;
            if (iCode == 2) { // INC
                locations << location();
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
                    locations << QDir::fromNativeSeparators(path) + "/inclib";
            }
            QString rawName = fName;
            for (QString loc : locations) {
                if (!QDir::isAbsolutePath(loc))
                    loc = location() + '/' + loc;
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
        QString fName = parts.first().mid(1, parts.first().length()-2).toString();
        if (!QDir::isAbsolutePath(fName))
            fName = location() + '/' + fName;

        exist = QFile(fName).exists();
        if (exist) res = fName;
        if (!create || !exist) return res;
        node = projectRepo()->findOrCreateFileNode(fName, this);
        if (parts.size() > 1) line = parts.at(1).toInt();
        if (parts.size() > 2) col = parts.at(2).toInt();
    }
    return res;
}

ProjectLogNode *ProjectRunGroupNode::logNode()
{
    if (!mLogNode) {
        QString suffix = FileType::from(FileKind::Log).defaultSuffix();
        QFileInfo fi = !parameter("gms").isEmpty()
                       ? parameter("gms") : QFileInfo(location()+"/"+name()+"."+suffix);
        QString logName = fi.path()+"/"+fi.completeBaseName()+"."+suffix;
        FileMeta* fm = fileRepo()->findOrCreateFileMeta(logName, &FileType::from(FileKind::Log));
        mLogNode = new ProjectLogNode(fm, this);
    }
    return mLogNode;
}

///
/// \brief ProjectGroupNode::setLogLocation sets the location of the log. Filename can be determined automatically from path.
/// \param path
///
void ProjectRunGroupNode::setLogLocation(QString path)
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
}

FileMeta* ProjectRunGroupNode::runnableGms() const
{
    FileMetaRepo *repo = fileRepo();
    if (!repo) return nullptr;
    return repo->fileMeta(parameter("gms"));
}

void ProjectRunGroupNode::setRunnableGms(FileMeta *gmsFile)
{
    ProjectFileNode *gmsFileNode;
    if (!gmsFile) {
        // find alternative runable file
        for (ProjectAbstractNode *node: childNodes()) {
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
    setParameter("gms", "");
    if (!gmsFile) {
        setParameter("lst", "");
        return;
    }
    setLocation(QFileInfo(gmsFile->location()).absoluteDir().path());
    QString gmsPath = gmsFile->location();
    setParameter("gms", gmsPath);
    if (hasLogNode()) logNode()->resetLst();
}

QString ProjectRunGroupNode::mainModelName(bool stripped) const
{
    FileMeta *fileMeta = toRunGroup()->runnableGms();

    if (!fileMeta) {
        SysLogLocator::systemLog()->append(QString("Could not find a runable gms file for group: %1")
                .arg(toRunGroup()->name()), LogMsgType::Error);
        return QString();
    }

    QFileInfo fileInfo(fileMeta->name());
    if (stripped)
        return fileInfo.completeBaseName();
    return fileInfo.fileName();

}

QString ProjectRunGroupNode::errorText(int lstLine)
{
    return mErrorTexts.value(lstLine);
}

void ProjectRunGroupNode::setErrorText(int lstLine, QString text)
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

void ProjectRunGroupNode::hasHRef(const QString &href, QString &fileName)
{
    ProjectFileNode *node;
    int line;
    int column;
    fileName = resolveHRef(href, node, line, column);
}

void ProjectRunGroupNode::jumpToHRef(const QString &href)
{
    ProjectFileNode *node;
    int line;
    int column;
    resolveHRef(href, node, line, column, true);
    if (node) node->file()->jumpTo(node->runGroupId(), true, line-1, column);
}

void ProjectRunGroupNode::createMarks(const LogParser::MarkData &marks)
{
    if (marks.hasErr() && !marks.hRef.isEmpty()) {
        int col;
        ProjectFileNode *errNode;
        int errLine;
        ProjectFileNode *lstNode;
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

void ProjectRunGroupNode::switchLst(const QString &lstFile)
{
    if (mParameterHash.contains("lst")) {
        setParameter("ls2", lstFile);
    }
}

void ProjectRunGroupNode::registerGeneratedFile(const QString &fileName)
{
    fileRepo()->setAutoReload(QDir::fromNativeSeparators(fileName));
}

void ProjectRunGroupNode::clearErrorTexts()
{
    mErrorTexts.clear();
}

bool ProjectRunGroupNode::hasErrorText(int lstLine)
{
    return (lstLine < 0) ? mErrorTexts.size() > 0 : mErrorTexts.contains(lstLine);
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

///
/// \brief ProjectRunGroupNode::analyzeParameters translates the gms file and an OptionItem list into a single QStringList, while also setting all extracted parameters for this group
/// \param gmsLocation gms file to be run
/// \param itemList list of options given by studio and user
/// \return QStringList all arguments
///
QStringList ProjectRunGroupNode::analyzeParameters(const QString &gmsLocation, QStringList defaultParameters, QList<option::OptionItem> itemList, option::Option *opt)
{
    // set studio default parameters
    QMultiMap<int, QString> gamsArguments;
    QStringList defaultArgumentList;
    QStringList defaultArgumentValueList;
    QList<int> defaultArgumentIdList = QList<int>();
    defaultArgumentList      << "logoption" << "ide" << "errorlog" << "errmsg" << "pagesize" << "lstTitleleftaligned" ;
    defaultArgumentValueList << "3"         << "1"   << "99"       << "1"      << "0"        << "1"                   ;
    int position = 0;
    for (QString arg : defaultArgumentList) {
        defaultArgumentIdList << opt->getOrdinalNumber(arg);
        gamsArguments.insert(opt->getOrdinalNumber(arg), defaultArgumentValueList.at(position++));
    }

    for(QString param: defaultParameters) {
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
    for (option::OptionItem item : itemList) {
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
    if (path.isEmpty()) path = fi.path();
    else if (QDir(path).isRelative()) path = fi.path() + '/' + path;

    setLogLocation(cleanPath(path, filestem + "." + FileType::from(FileKind::Log).defaultSuffix()));

    clearParameters();
    // set default lst name to revert deleted o parameter values
    setParameter("lst", cleanPath(path, filestem + ".lst"));

    bool defaultOverride = false;
    // iterate options
    for (option::OptionItem item : itemList) {
        // convert to native seperator
        QString value = item.value;
        value = value.replace('\\', '/');

        // regex to remove dots at the end of a filename
        QRegularExpression notDotAsEnding("[\\w\\d](\\.)[\"\\\\ ]*$");
        QRegularExpressionMatch match = notDotAsEnding.match(value);
        if (match.hasMatch()) value = value.remove(match.capturedStart(1), match.capturedLength(1));

        // set parameters
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

        } else if (QString::compare(item.key, "lf", Qt::CaseInsensitive) == 0) {
            if (!value.endsWith(".log")) value.append("." + FileType::from(FileKind::Log).defaultSuffix());
            setLogLocation(cleanPath(path, value));
        }

        if (item.optionId != -1 && defaultArgumentIdList.contains(item.optionId))
            defaultOverride = true;
    }

    if (defaultOverride)
        SysLogLocator::systemLog()->append("You are overwriting at least one GAMS Studio default argument. "
                     "Some of these are necessary to ensure a smooth experience. "
                     "Use at your own risk!", LogMsgType::Warning);

    // prepare gams command
    QStringList output { CommonPaths::nativePathForProcess(gmsLocation) };
    // normalize gams parameter format
    position = 0;
    for(int id : defaultArgumentIdList) {
        if (defaultArgumentIdList.contains(id))
            output.append( QString("%1=%2").arg(defaultArgumentList.at(position)).arg( defaultArgumentValueList.at(position)) );
        else
            output.append( QString("%1=%2").arg(defaultArgumentList.at(position)).arg( gamsArguments.value(id)) );
        position++;
    }
    position = 0;
    for(option::OptionItem item : itemList) {
        if (item.recurrent) {
            if (item.recurrentIndices.first() == position)
                output.append( QString("%1=%2").arg(argumentList.at(position)).arg( gamsArguments.value(item.optionId)) );
        } else {
            if (item.optionId == -1) { // double-dashed option or unknown option
                output.append( QString("%1=%2").arg(argumentList.at(position)).arg( item.value ) );
            } else {
                output.append( QString("%1=%2").arg(argumentList.at(position)).arg( gamsArguments.value(item.optionId)) );
            }
        }
        position++;
    }
    return output;
}

///
/// \brief ProjectRunGroupNode::normalizePath removes quotes and trims whitespaces for use within studio. do not pass to gams!
/// \param path workign dir
/// \param file file name, can be absolute or relative
/// \return cleaned path
///
QString ProjectRunGroupNode::cleanPath(QString path, QString file) {

    QString ret = "";

    path.remove("\""); // remove quotes from path
    QDir dir(path);
    if (dir.isRelative()) {
        path = location() + '/' + path;
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

bool ProjectRunGroupNode::isProcess(const AbstractProcess *process) const
{
    return process && mGamsProcess.get() == process;
}

bool ProjectRunGroupNode::jumpToFirstError(bool focus, ProjectFileNode* lstNode)
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

void ProjectRunGroupNode::errorTexts(const QVector<int> &lstLines, QStringList &result)
{
    for (int lstLine: lstLines) {
        QString newTip = errorText(lstLine);
        if (!result.contains(newTip))
            result << newTip;
    }
}

QString ProjectRunGroupNode::parameter(const QString &kind) const
{
    return mParameterHash.value(kind);
}

bool ProjectRunGroupNode::hasParameter(const QString &kind) const
{
    return mParameterHash.contains(kind);
}

void ProjectRunGroupNode::addNodesForSpecialFiles()
{
    FileMeta* runFile = runnableGms();
    for (QString loc : mParameterHash.values()) {

        if (QFileInfo::exists(loc)) {
            ProjectFileNode* node = projectRepo()->findOrCreateFileNode(loc, this, &FileType::from(mParameterHash.key(loc)));
            if (runFile)
                node->file()->setCodec(runFile->codec());
            else {
                int codecMib = Settings::settings()->toInt(skDefaultCodecMib);
                QTextCodec *codec = QTextCodec::codecForMib(codecMib);
                if (codec) node->file()->setCodec(codec);
            }
        } else {
            SysLogLocator::systemLog()->append("Could not add file " + loc, LogMsgType::Error);
        }
    }
}

void ProjectRunGroupNode::setParameter(const QString &kind, const QString &path)
{
    if (path.isEmpty()) {
        mParameterHash.remove(kind);
        return;
    }
    QString fullPath = path.contains('\\') ? QDir::fromNativeSeparators(path) : path;
    if (QFileInfo(fullPath).isRelative())
        fullPath = QFileInfo(location()).canonicalFilePath() + "/" + fullPath;

    fullPath.remove("\"");

    if (QFileInfo(fullPath).suffix().isEmpty()) {
        if (kind == "gdx")
            fullPath += ".gdx";
        else if (kind == "lst" || kind == "ls2")
        { /* do nothing */ } // gams does not add lst extension. unlike .ref or .gdx
        else if (kind == "ref")
            fullPath += ".ref";
        else
            DEB() << "WARNING: unhandled parameter!" << fullPath << "is missing extension.";
    }

    mParameterHash.insert(kind, fullPath);
}

void ProjectRunGroupNode::clearParameters()
{
    QString gms = mParameterHash.value("gms");
    mParameterHash.clear();
    mParameterHash.insert("gms", gms);
}

QProcess::ProcessState ProjectRunGroupNode::gamsProcessState() const
{
    return mGamsProcess ? mGamsProcess->state() : QProcess::NotRunning;
}

QString ProjectRunGroupNode::tooltip()
{
    QString res(QDir::toNativeSeparators(location()));
    if (runnableGms()) res.append("\n\nMain GMS file: ").append(runnableGms()->name());
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

void ProjectRunGroupNode::onGamsProcessStateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState)
    emit gamsProcessStateChanged(this);
}

ProjectRootNode::ProjectRootNode(ProjectRepo* repo)
    : ProjectGroupNode("Root", "", NodeType::root), mRepo(repo)
{
    if (!mRepo) EXCEPT() << "The ProjectRepo must not be null";
}

void ProjectRootNode::setParentNode(ProjectGroupNode *parent)
{
    Q_UNUSED(parent)
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

} // namespace studio
} // namespace gams
