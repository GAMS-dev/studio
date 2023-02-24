/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include <memory>
#include "process/abstractprocess.h"
#include "editors/logparser.h"
#include "pexabstractnode.h"
#include "syntax/textmark.h"

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
    PExProjectNode *findProject(FileId runId) const;
    const QVector<PExFileNode *> listFiles() const;
    void moveChildNode(int from, int to);
    const QList<PExAbstractNode*> &childNodes() const { return mChildNodes; }

public slots:
    void hasFile(QString fName, bool &exists);

protected:
    friend class ProjectRepo;
    friend class PExAbstractNode;
    friend class PExLogNode;
    friend class PExFileNode;

    PExGroupNode(QString name, QString location, NodeType type = NodeType::group);
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
    enum Type {tCommon, tSearch, tGams};

public:
    virtual ~PExProjectNode() override;
    QIcon icon(QIcon::Mode mode = QIcon::Normal, int alpha = 100) override;
    QString name(NameModifier mod = NameModifier::raw) const override;
    void setName(const QString& name) override;
    bool hasLogNode() const;
    PExLogNode* logNode();
    FileMeta *runnableGms() const;
    void setRunnableGms(FileMeta *gmsFile = nullptr);
    FileMeta *projectEditFileMeta() const;
    void setProjectEditFileMeta(FileMeta *prOptMeta);
    void unlinkProjectEditFileMeta();
    QString mainModelName(bool stripped = true) const;
    QString tooltip() override;
    QString errorText(int lstLine) override;
    void clearErrorTexts();
    bool hasErrorText(int lstLine = -1);
    void addRunParametersHistory(QString option);
    QStringList getRunParametersHistory() const;
    QStringList analyzeParameters(const QString &gmsLocation, const QStringList &defaultParameters, const QList<option::OptionItem> &itemList, option::Option *opt, int &logOption);
    void setLocation(const QString &newLocation) override;
    void setWorkDir(const QString &workingDir);
    QString workDir() const;
    void refreshProjectTabName();

    QString parameter(const QString& kind) const;
    bool hasParameter(const QString& kind) const;
    void addNodesForSpecialFiles();
    void setParameter(const QString& kind, const QString& path);
    void clearParameters();

    bool isProcess(const AbstractProcess *process) const;
    QProcess::ProcessState gamsProcessState() const;
    void setProcess(std::unique_ptr<AbstractProcess> process);
    AbstractProcess *process() const;
    bool jumpToFirstError(bool focus, PExFileNode *lstNode);

    void setNeedSave(bool needSave = true);
    bool needSave() const;
    void setIsClosing();
    bool isClosing() const;

    const QString &fileName() const;

    const QString &nameExt() const;
    void setNameExt(const QString &newNameExt);

    Type type() const;

signals:
    void gamsProcessStateChanged(gams::studio::PExGroupNode* group);
    void getParameterValue(QString param, QString &value);
    void baseDirChanged(gams::studio::PExProjectNode *project);
    void runnableChanged();

public slots:
    void setErrorText(int lstLine, QString text);
    void hasHRef(const QString &href, QString &fileName);
    void jumpToHRef(const QString &href);
    void createMarks(const LogParser::MarkData &marks);
    void switchLst(const QString &lstFile);
    void registerGeneratedFile(const QString &fileName);

protected slots:
    void onGamsProcessStateChanged(QProcess::ProcessState newState);

protected:
    friend class ProjectRepo;
    friend class PExAbstractNode;
    friend class PExLogNode;
    friend class PExFileNode;

    PExProjectNode(QString filePath, QString basePath, FileMeta *runFileMeta, QString workDir, Type type);
    void setFileName(const QString &newProjectFile);
    void errorTexts(const QVector<int> &lstLines, QStringList &result);
    void setLogNode(PExLogNode* logNode);
    void appendChild(PExAbstractNode *child) override;
    void removeChild(PExAbstractNode *child) override;
    QString resolveHRef(QString href, PExFileNode *&node, int &line, int &col, bool create = false);

private:
    enum ChangeState {csNone, csChanged, csClosing};
    QString mProjectFile;
    QString mWorkDir;
    QString mNameExt;
    Type mType = tCommon;
    std::unique_ptr<AbstractProcess> mGamsProcess;
    PExLogNode* mLogNode = nullptr;
    FileMeta *mProjectEditFileMeta = nullptr;
    QHash<int, QString> mErrorTexts;
    QStringList mRunParametersHistory;
    QHash<QString, QString> mParameterHash;
    ChangeState mChangeState = csNone;

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
