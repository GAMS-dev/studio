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
#ifndef PEXGROUPNODE_H
#define PEXGROUPNODE_H

#include <QProcess>
#include <QFileInfoList>
#include "gamscom/profiler.h"
#include "process/abstractprocess.h"
#include "editors/logparser.h"
#include "pexabstractnode.h"
#include "gamscom/server.h"

namespace gams {
namespace studio {

class PExLogNode;
class PExFileNode;
class TextMarkRepo;
class FileMeta;
class FileMetaRepo;
namespace option {
struct OptionItem;
class Option;
}
namespace gamscom {
class ContinuousLineData;
class Server;
class Profiler;
}

class PExGroupNode : public PExAbstractNode
{
    Q_OBJECT
public:
    virtual ~PExGroupNode() override;

    QIcon icon(QIcon::Mode mode = QIcon::Normal, int alpha = 100) override;
    int childCount() const;
    bool isEmpty();
    PExAbstractNode* childNode(int index) const;
    int indexOf(PExAbstractNode *child);
    virtual QString location() const;
    QString tooltip() override;
    virtual QString errorText(int lstLine);
    virtual PExFileNode *findFile(QString location) const;
    virtual PExFileNode *findFile(const FileMeta *fileMeta) const;
    QList<PExFileNode*> findFiles(FileKind kind) const;
    PExProjectNode *findProject(const AbstractProcess *process) const;
    PExProjectNode *findProject(const FileId &runId) const;
    const QVector<PExFileNode *> listFiles() const;
    void moveChildNode(int from, int to);
    const QList<PExAbstractNode*> &childNodes() const { return mChildNodes; }

public slots:
    void hasFile(const QString &fName, bool &exists);

protected:
    friend class ProjectRepo;
    friend class PExAbstractNode;
    friend class PExLogNode;
    friend class PExFileNode;

    PExGroupNode(const QString &name, const QString &location, NodeType type = NodeType::group);
    virtual void appendChild(PExAbstractNode *child);
    virtual void removeChild(PExAbstractNode *child);
    virtual void setLocation(const QString &newLocation);

private:
    QList<PExAbstractNode*> mChildNodes;
    QString mLocation;
};

class PExProjectNode : public PExGroupNode
{
    Q_OBJECT
public:
    enum Type {tSmall, tCommon, tSearch, tGams};

public:
    virtual ~PExProjectNode() override;
    QIcon icon(QIcon::Mode mode = QIcon::Normal, int alpha = 100) override;
    QString name(NameModifier mod = NameModifier::raw) const override;
    void setName(const QString& name) override;
    bool hasLogNode() const;
    PExLogNode* logNode();
    FileMeta *mainFile() const;
    void setMainFile(FileMeta *gmsFile = nullptr);
    bool dynamicMainFile() const;
    void setDynamicMainFile(bool newDynamicMainFile);
    bool ownBaseDir() const;
    void setOwnBaseDir(bool ownBaseDir);
    bool hasParameterFile();
    FileMeta *parameterFile() const;
    void setParameterFile(FileMeta *pfFile = nullptr);
    void setParameterFile(const QString &fileName);
    FileMeta *projectEditFileMeta() const;
    void setProjectEditFileMeta(FileMeta *prOptMeta);
    void unlinkProjectEditFileMeta();
    QString mainModelName(bool stripped = true) const;
    QString tooltip() override;
    QString errorText(int lstLine) override;
    void clearErrorTexts();
    void clearTextMarks(const QSet<TextMark::Type> &markTypes);
    bool hasErrorText(int lstLine = -1);
    void setMainFileParameterHistory(FileId fileId, const QStringList &parameterLists);
    QStringList mainFileParameterHistory(FileId fileId) const;
    void addRunParametersHistory(const QString &option);
    QStringList getRunParametersHistory() const;
    QStringList analyzeParameters(const QString &gmsLocation, const QStringList &defaultParameters
                                  , const QList<option::OptionItem> &itemList, option::Option *opt
                                  , gamscom::ComFeatures comMode, int &logOption);
    void setLocation(const QString &newLocation) override;
    void setWorkDir(const QString &workingDir);
    QString workDir() const;
    void refreshProjectTabName();

    QString parameter(const QString& kind) const;
    bool hasParameter(const QString& kind) const;
    void addNodesForSpecialFiles(bool ignoreMissing);
    void setParameter(const QString& kind, const QString& path);
    void clearParameters();

    gamscom::Server *comServer() const;
    bool startComServer(gamscom::ComFeatures mode);
    void stopComServer();
    QString engineJobToken() const;
    void setEngineJobToken(const QString &engineJobToken, bool touch = true);

    bool isProcess(const AbstractProcess *process) const;
    QProcess::ProcessState gamsProcessState() const;
    void setProcess(AbstractProcess* process);
    AbstractProcess *process() const;
    bool jumpToFirstError(bool focus, PExFileNode *lstNode);

    void setNeedSave(bool needSave = true);
    bool needSave() const;
    void setIsClosing();
    bool isClosing() const;

    const QString &fileName() const;
    void setNameExt(const QString &newNameExt) override;
    Type type() const;
    void setHasGspFile(bool hasGspFile);
    bool wantsGspFile();

    QString tempGdx() const;
    void setVerbose(bool verbose);
    gamscom::Profiler *profiler() const;
    gamscom::ContinuousLineData *contLineData() const;
    bool doProfile() const;
    bool isProfilerVisible() const;
    void setProfilerVisible(bool profilerVisible);
    void updateProfilerForOpenNodes();

signals:
    void gamsProcessStateChanged(gams::studio::PExProjectNode* project);
    void getParameterValue(QString param, QString &value);
    void baseDirChanged(gams::studio::PExProjectNode *project);
    void mainFileChanged();
    void addProcessLog(const QByteArray &data);
    void openInPinView(gams::studio::PExProjectNode *project, QWidget *editInMainTabs);
    void openFileNode(gams::studio::PExFileNode *node);
    void switchToTab(gams::studio::FileMeta *fileMeta);
    void updateProfilerAction();

public slots:
    void setErrorText(int lstLine, const QString &text);
    void hasHRef(const QString &href, QString &fileName);
    void jumpToHRef(const QString &href);
    void jumpToFile(const QString &filename, int line = 0);
    void createMarks(const gams::studio::LogParser::MarkData &marks);
    void switchLst(const QString &lstFile);
    void registerGeneratedFile(const QString &fileName);
    void addBreakpoint(const QString &filename, int line);
    void delBreakpoint(const QString &filename, int line);
    void delBreakpoints(const QString &filename, int line, bool before);
    void clearBreakpoints();
    void breakpoints(const QString &filename, gams::studio::SortedIntMap &bps, gams::studio::SortedIntMap &aimedBps) const;
    void switchProfiler(bool active);
    void gotoPaused(int contLine);
    void terminate();
    void processState(QProcess::ProcessState &state);
    void takeCheckedPaths(QStringList &filePaths);

protected slots:
    void onGamsProcessStateChanged(QProcess::ProcessState newState);
    void openDebugGdx(const QString &gdxFile);
    void addLinesMap(const QString &filename, const QList<int> &fileLines, const QList<int> &continuousLines);
    void updateOpenEditors();

protected:
    friend class ProjectRepo;
    friend class PExAbstractNode;
    friend class PExLogNode;
    friend class PExFileNode;

    PExProjectNode(const QString &filePath, const QString &basePath,
                   FileMeta *mainFileMeta, const QString &workDir, Type type);
    void setFileName(const QString &newProjectFile);
    void errorTexts(const QVector<int> &lstLines, QStringList &result);
    void setLogNode(PExLogNode* logNode);
    void appendChild(PExAbstractNode *child) override;
    void removeChild(PExAbstractNode *child) override;
    QString resolveHRef(const QString &href, PExFileNode *&node, int &line, int &col, bool create = false);

private:
    enum ChangeState {csNone, csChanged, csClosing};
    QString mProjectFile;
    QString mWorkDir;
    QString mNameExt;
    Type mType = tCommon;
    QScopedPointer<AbstractProcess> mGamsProcess;
    QString mEngineJobToken;
    QTimer mUpdateEditsTimer;
    PExLogNode* mLogNode = nullptr;
    FileMeta *mProjectEditFileMeta = nullptr;
    bool mDynamicMainFile = false;
    bool mOwnBaseDir = false;
    QHash<int, QString> mErrorTexts;
    QHash<FileId, QStringList> mMainFileParameters;
    QHash<QString, QString> mParameterHash;
    QHash<QString, QMap<int, QString>> mIncludes;
    ChangeState mChangeState = csNone;
    gamscom::Server *mComServer = nullptr;
    QString mTempGdx;
    gamscom::Profiler *mProfiler;
    bool mDoProfile = false;
    bool mProfilerVisible = false;
    bool mVerbose = false;
    gamscom::ContinuousLineData *mContLineData;
    PExFileNode *mPausedInFile = nullptr;
    QStringList mCheckedPaths;

private:
    QString cleanPath(QString path, QString file);
    void setLogLocation(QString path);
    void updateLogName(const QString &name);
};


class PExRootNode : public PExGroupNode
{
    Q_OBJECT
public:
    PExRootNode(ProjectRepo *projectRepo);
    ~PExRootNode() override {}
    ProjectRepo *projectRepo() const override;
    FileMetaRepo *fileRepo() const override;
    TextMarkRepo *textMarkRepo() const override;
    PExProjectNode *findProject(const QString &projectFile) const;

private:
    friend class ProjectRepo;
    void setParentNode(PExGroupNode *parent) override;
    void init(ProjectRepo* projectRepo);

private:
    ProjectRepo* mRepo = nullptr;
};

} // namespace studio
} // namespace gams

#endif // PEXGROUPNODE_H
