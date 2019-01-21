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
#ifndef PROJECTGROUPNODE_H
#define PROJECTGROUPNODE_H

#include <QProcess>
#include <QFileInfoList>
#include <memory>
#include "projectabstractnode.h"
#include "syntax/textmark.h"
#include "gamsprocess.h"

namespace gams {
namespace studio {

class ProjectLogNode;
class ProjectFileNode;
class TextMarkRepo;
class FileMeta;
class FileMetaRepo;
class AbstractProcess;
struct OptionItem;

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
    virtual QString lstErrorText(int line);
    ProjectFileNode *findFile(const QString &location, bool recurse = true) const;
    ProjectFileNode *findFile(const FileMeta *fileMeta, bool recurse = true) const;
    ProjectFileNode *findOrCreateFileNode(const QString &location);
    ProjectRunGroupNode *findRunGroup(const AbstractProcess *process) const;
    ProjectRunGroupNode *findRunGroup(FileId runId) const;
    QVector<ProjectFileNode*> listFiles(bool recurse = false) const;

protected:
    friend class ProjectRepo;
    friend class ProjectAbstractNode;
    friend class ProjectLogNode;
    friend class ProjectFileNode;

    ProjectGroupNode(QString name, QString location, NodeType type = NodeType::group);
    void insertChild(ProjectAbstractNode *child);
    void removeChild(ProjectAbstractNode *child);
    void setLocation(const QString &location);
    int peekIndex(const QString &name, bool* hit = nullptr);
    const QList<ProjectAbstractNode*> &internalNodeList() const { return mChildNodes; }

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
    QString tooltip() override;
    QString lstErrorText(int line) override;
    void setLstErrorText(int line, QString text);
    void clearLstErrorTexts();
    bool hasLstErrorText( int line = -1);
    void addRunParametersHistory(QString option);
    QStringList getRunParametersHistory() const;
    QStringList analyzeParameters(const QString &gmsLocation, QList<OptionItem> itemList);

    QString specialFile(const FileKind& kind) const;
    bool hasSpecialFile(const FileKind& kind) const;
    void addNodesForSpecialFiles();
    void setSpecialFile(const FileKind& kind, const QString& path);
    void clearSpecialFiles();

    bool isProcess(const AbstractProcess *process) const;
    QProcess::ProcessState gamsProcessState() const;
    GamsProcess *gamsProcess() const;
    void jumpToFirstError(bool focus);

signals:
    void gamsProcessStateChanged(ProjectGroupNode* group);

protected slots:
    void onGamsProcessStateChanged(QProcess::ProcessState newState);

protected:
    friend class ProjectRepo;
    friend class ProjectAbstractNode;
    friend class ProjectLogNode;
    friend class ProjectFileNode;

    ProjectRunGroupNode(QString name, QString path, FileMeta *runFileMeta = nullptr);
    void updateRunState(const QProcess::ProcessState &state);
    void lstTexts(const QList<TextMark*> &marks, QStringList &result);
    void setLogNode(ProjectLogNode* logNode);

private:
    std::unique_ptr<GamsProcess> mGamsProcess;
    ProjectLogNode* mLogNode = nullptr;
    QHash<int, QString> mLstErrorTexts;
    QStringList mRunParametersHistory;
    QHash<FileKind, QString> mSpecialFiles;

private:
    QString normalizePath(QString path, QString file);

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
