/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#ifndef PROJECTGROUPNODE_H
#define PROJECTGROUPNODE_H

#include <QProcess>
#include <QFileInfoList>
#include <memory>
#include "abstractprocess.h"
#include "editors/logparser.h"
#include "projectabstractnode.h"
#include "syntax/textmark.h"

namespace gams {
namespace studio {

class ProjectLogNode;
class ProjectFileNode;
class TextMarkRepo;
class FileMeta;
class FileMetaRepo;
namespace option {
struct OptionItem;
}

class ProjectGroupNode : public ProjectAbstractNode
{
    Q_OBJECT
public:
    virtual ~ProjectGroupNode() override;

    QIcon icon() override;
    int childCount() const;
    bool isEmpty();
    ProjectAbstractNode* childNode(int index) const;
    int indexOf(ProjectAbstractNode *child);
    virtual QString location() const;
    QString tooltip() override;
    virtual QString errorText(int lstLine);
    ProjectFileNode *findFile(QString location, bool recurse = true) const;
    ProjectFileNode *findFile(const FileMeta *fileMeta, bool recurse = true) const;
    ProjectRunGroupNode *findRunGroup(const AbstractProcess *process) const;
    ProjectRunGroupNode *findRunGroup(FileId runId) const;
    QVector<ProjectFileNode*> listFiles(bool recurse = false) const;
    void moveChildNode(int from, int to);
    const QList<ProjectAbstractNode*> &childNodes() const { return mChildNodes; }

public slots:
    void hasFile(QString fName, bool &exists);

protected:
    friend class ProjectRepo;
    friend class ProjectAbstractNode;
    friend class ProjectLogNode;
    friend class ProjectFileNode;

    ProjectGroupNode(QString name, QString location, NodeType type = NodeType::group);
    void appendChild(ProjectAbstractNode *child);
    void removeChild(ProjectAbstractNode *child);
    void setLocation(const QString &location);

private:
    QList<ProjectAbstractNode*> mChildNodes;
    QString mLocation;
};

class ProjectRunGroupNode : public ProjectGroupNode
{
    Q_OBJECT
public:
    QIcon icon() override;
    bool hasLogNode() const;
    ProjectLogNode* logNode();
    FileMeta *runnableGms() const;
    void setRunnableGms(FileMeta *gmsFile = nullptr);
    QString mainModelName(bool stripped = true) const;
    QString tooltip() override;
    QString errorText(int lstLine) override;
    void clearErrorTexts();
    bool hasErrorText(int lstLine = -1);
    void addRunParametersHistory(QString option);
    QStringList getRunParametersHistory() const;
    QStringList analyzeParameters(const QString &gmsLocation, QStringList defaultParameters, QList<option::OptionItem> itemList);

    QString parameter(const QString& kind) const;
    bool hasParameter(const QString& kind) const;
    void addNodesForSpecialFiles();
    void setParameter(const QString& kind, const QString& path);
    void clearParameters();

    bool isProcess(const AbstractProcess *process) const;
    QProcess::ProcessState gamsProcessState() const;
    void setProcess(std::unique_ptr<AbstractProcess> process);
    AbstractProcess *process() const;
    bool jumpToFirstError(bool focus, ProjectFileNode *lstNode);

    void unwatchFiles();
    void watchFiles();
    void reloadOpenFiles();

signals:
    void gamsProcessStateChanged(ProjectGroupNode* group);

public slots:
    void setErrorText(int lstLine, QString text);
    void hasHRef(const QString &href, bool &exist);
    void jumpToHRef(const QString &href);
    void createMarks(const LogParser::MarkData &marks);
    void switchLst(const QString &lstFile);

protected slots:
    void onGamsProcessStateChanged(QProcess::ProcessState newState);

protected:
    friend class ProjectRepo;
    friend class ProjectAbstractNode;
    friend class ProjectLogNode;
    friend class ProjectFileNode;

    ProjectRunGroupNode(QString name, QString path, FileMeta *runFileMeta = nullptr);
    void errorTexts(const QVector<int> &lstLines, QStringList &result);
    void setLogNode(ProjectLogNode* logNode);
    void removeChild(ProjectAbstractNode *child);
    void resolveHRef(QString href, bool &exist, ProjectFileNode *&node, int &line, int &col, bool create = false);

private:
    std::unique_ptr<AbstractProcess> mGamsProcess;
    ProjectLogNode* mLogNode = nullptr;
    QHash<int, QString> mErrorTexts;
    QStringList mRunParametersHistory;
    QHash<QString, QString> mParameterHash;

private:
    QString cleanPath(QString path, QString file);
    void setLogLocation(QString path);
};


class ProjectRootNode : public ProjectGroupNode
{
    Q_OBJECT
public:
    ProjectRootNode(ProjectRepo *projectRepo);
    ~ProjectRootNode() override {}
    ProjectRepo *projectRepo() const override;
    FileMetaRepo *fileRepo() const override;
    TextMarkRepo *textMarkRepo() const override;

private:
    friend class ProjectRepo;
    void setParentNode(ProjectGroupNode *parent) override;
    void init(ProjectRepo* projectRepo);

private:
    ProjectRepo* mRepo = nullptr;
};

} // namespace studio
} // namespace gams

#endif // PROJECTGROUPNODE_H
