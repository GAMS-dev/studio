/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "editors/sysloglocator.h"
#include "settingslocator.h"
#include "studiosettings.h"
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

QIcon ProjectGroupNode::icon()
{
    return QIcon(":/img/folder-open");
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

GamsProcess *ProjectRunGroupNode::gamsProcess() const
{
    return mGamsProcess.get();
}

QIcon ProjectRunGroupNode::icon()
{
    if (gamsProcessState() == QProcess::NotRunning)
        return ProjectGroupNode::icon();
    return projectRepo()->runAnimateIcon();
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

void ProjectRunGroupNode::removeChild(ProjectAbstractNode *child)
{
    ProjectGroupNode::removeChild(child);
    ProjectFileNode *file = child->toFile();
    if (file) {
        QList<QString> files = mParameterHash.keys(file->location());
        for (const QString &file: files) {
            mParameterHash.remove(file);
        }
    }
}

void ProjectRunGroupNode::resolveHRef(QString href, bool &exist, ProjectFileNode *&node, int &line, int &col, bool create)
{
    exist = false;
    node = nullptr;
    line = 0;
    col = 0;
    if (href.length() < 5) return;
    QStringRef code = href.leftRef(3);
    QVector<QStringRef> parts = href.rightRef(href.length()-4).split(',');
    if (code.compare(QString("LST")) == 0) {
        QString lstFile = parameter("lst");
        exist = QFile(lstFile).exists();
        if (!create || !exist) return;
        line = parts.first().toInt();
        node = projectRepo()->findOrCreateFileNode(lstFile, this, &FileType::from(FileKind::Lst));
    } else if (parts.first().startsWith('"')) {
        QString fName = parts.first().mid(1, parts.first().length()-2).toString();
        exist = QFile(fName).exists();
        if (!create || !exist) return;
        node = projectRepo()->findOrCreateFileNode(fName, this);
        if (parts.size() > 1) line = parts.at(1).toInt();
        if (parts.size() > 2) col = parts.at(2).toInt();
    }
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
    return fileRepo()->fileMeta(parameter("gms"));
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

void ProjectRunGroupNode::hasHRef(const QString &href, bool &exist)
{
    ProjectFileNode *node;
    int line;
    int column;
    resolveHRef(href, exist, node, line, column);
}

void ProjectRunGroupNode::jumpToHRef(const QString &href)
{
    bool exist;
    ProjectFileNode *node;
    int line;
    int column;
    resolveHRef(href, exist, node, line, column, true);
    if (node) node->file()->jumpTo(node->runGroupId(), true, line-1, column);
}

void ProjectRunGroupNode::createMarks(const LogParser::MarkData &marks)
{
    if (marks.hasErr() && !marks.hRef.isEmpty()) {
        bool exist;
        int col;
        ProjectFileNode *errNode;
        int errLine;
        ProjectFileNode *lstNode;
        int lstLine;
        resolveHRef(marks.hRef, exist, lstNode, lstLine, col, true);
        resolveHRef(marks.errRef, exist, errNode, errLine, col, true);
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
QStringList ProjectRunGroupNode::analyzeParameters(const QString &gmsLocation, QList<option::OptionItem> itemList)
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
    QString cdir = "";
    QString wdir = "";
    QString filestem = "";
    for (option::OptionItem item : itemList) {
        if (QString::compare(item.key, "curdir", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "cdir", Qt::CaseInsensitive) == 0) {
            cdir = item.value;
            gamsArgs[item.key] = item.value;
        }

        if (QString::compare(item.key, "workdir", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "wdir", Qt::CaseInsensitive) == 0) {
            wdir = item.value;
            gamsArgs[item.key] = item.value;
        }

        if (QString::compare(item.key, "filestem", Qt::CaseInsensitive) == 0)
            filestem = item.value;
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
    if (filestem.isEmpty()) filestem = fi.path() + '/' + fi.completeBaseName();
    if (path.isEmpty()) path = fi.path();
    else if (QDir(path).isRelative()) path = fi.path() + '/' + path;

    setLogLocation(cleanPath(path, filestem + "." + FileType::from(FileKind::Log).defaultSuffix()));

    clearParameters();
    // set default lst name to revert deleted o parameter values
    setParameter("lst", cleanPath(path, filestem + ".lst"));

    bool defaultOverride = false;
    // iterate options
    for (option::OptionItem item : itemList) {

        // keep unmodified value as option for output
        gamsArgs[item.key] = item.value;

        // convert to native seperator
        QString value = item.value;
        value = value.replace('\\', '/');

        // regex to remove dots at the end of a filename
        QRegularExpression notDotAsEnding("[\\w\\d](\\.)[\"\\\\ ]*$");
        QRegularExpressionMatch match = notDotAsEnding.match(value);
        if (match.hasMatch()) value = value.remove(match.capturedStart(1), match.capturedLength(1));

        // set parameters
        if (QString::compare(item.key, "o", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "output", Qt::CaseInsensitive) == 0) {
            mParameterHash.remove("lst"); // remove default

            if (!(QString::compare(value, "nul", Qt::CaseInsensitive) == 0
                        || QString::compare(value, "/dev/null", Qt::CaseInsensitive) == 0))
            setParameter("lst", cleanPath(path, value));

        } else if (QString::compare(item.key, "gdx", Qt::CaseInsensitive) == 0) {
            if (value == "default") value = "\"" + filestem + ".gdx\"";
            setParameter("gdx", cleanPath(path, value));

        } else if (QString::compare(item.key, "rf", Qt::CaseInsensitive) == 0) {
            if (value == "default") value = "\"" + filestem + ".ref\"";
            setParameter("ref", cleanPath(path, value));

        } else if (QString::compare(item.key, "lf", Qt::CaseInsensitive) == 0) {
            if (!value.endsWith(".log")) value.append("." + FileType::from(FileKind::Log).defaultSuffix());
            setLogLocation(cleanPath(path, value));
        }

        if (defaultGamsArgs.contains(item.key))
            defaultOverride = true;
    }

    if (defaultOverride)
        SysLogLocator::systemLog()->append("You are overwriting at least one GAMS Studio default argument. "
                     "Some of these are necessary to ensure a smooth experience. "
                     "Use at your own risk!", LogMsgType::Warning);

    // prepare gams command
#if defined(__unix__) || defined(__APPLE__)
    QStringList output { QDir::toNativeSeparators(gmsLocation) };
#else
    QStringList output { "\""+QDir::toNativeSeparators(gmsLocation)+"\"" };
#endif
    // normalize gams parameter format
    for(QString k : gamsArgs.keys()) {
        output.append(k + "=" + gamsArgs.value(k));
    }
    // console output
    QString msg = "Running GAMS:";
    msg.append(output.join(" "));
    SysLogLocator::systemLog()->append(msg, LogMsgType::Info);

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
        if (SettingsLocator::settings()->openLst()) {
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
            if (runFile) node->file()->setCodec(runFile->codec());
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
        else if (kind == "lst")
        { /* do nothing */ } // gams does not add lst extension. unlike .ref or .gdx
        else if (kind == "ref")
            fullPath += ".ref";
        else
            qDebug() << "WARNING: unhandled parameter!" << fullPath << "is missing extension.";
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
    if (debugMode()) {
        res.append("\nNodeId: "+QString::number(id()));
        res.append("\nParent-NodeId: " + (parentNode() ? QString::number(parentNode()->id()) : "?"));
    }
    return res;
}

void ProjectRunGroupNode::onGamsProcessStateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState);
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

} // namespace studio
} // namespace gams
