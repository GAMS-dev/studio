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
//#include <file.h>
#include "projectabstractnode.h"
#include "syntax/textmark.h"
#include "gamsprocess.h"

namespace gams {
namespace studio {

class ProjectLogNode;
class ProjectFileNode;
class GamsProcess;
class TextMarkRepo;
class FileMeta;
class FileMetaRepo;
class AbstractProcess;

class ProjectGroupNode : public ProjectAbstractNode
{
    Q_OBJECT
public:
    virtual ~ProjectGroupNode() override;

    QIcon icon() override;
    int childCount() const;
    ProjectAbstractNode* childNode(int index) const;
    int indexOf(ProjectAbstractNode *child);
    virtual QString location() const;
    QString tooltip() override;
    virtual QString lstErrorText(int line);
    ProjectAbstractNode *findNode(const QString &location, bool recurse = true) const;
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

//public:
//    void setFlag(ContextFlag flag, bool value = true) override;
//    void unsetFlag(ContextFlag flag) override;

//    ProjectAbstractNode* findNode(QString filePath);
//    ProjectFileNode* findFile(FileId fileId);
//    ProjectFileNode* findFile(QString filePath);

//    QString lstFileName();
//    void setLstFileName(const QString &lstFileName);

//    GamsProcess* gamsProcess();

//    void detachFile(const QString &filepath);
//    void updateChildNodes();
//    void jumpToFirstError(bool focus);

//    void saveGroup();

//    void dumpMarks();

//signals:
//    void removeNode(ProjectAbstractNode *node);
//    void requestNode(QString name, QString location, ProjectGroupNode* parent = nullptr);
//    void findOrCreateFileNode(QString filePath, ProjectFileNode *&resultFile, ProjectGroupNode* fileGroup = nullptr);


//protected:

//    void checkFlags() override;
//    void addMark(const QString &filePath, TextMark* mark);
//    TextMarkRepo* marks(const QString &fileName);
//    void removeMarks(QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());
//    void removeMarks(QString fileName, QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());

//private:
//    QFileInfoList mAttachedFiles;
//    QHash<QString, TextMarkRepo*> mMarksForFilenames;

};

class ProjectRunGroupNode : public ProjectGroupNode
{
    Q_OBJECT
public:
    ProjectLogNode* logNode() const;
    void setLogNode(ProjectLogNode* logNode);
    ProjectLogNode* getOrCreateLogNode(FileMetaRepo* fileMetaRepo);
    FileMeta *runnableGms() const;
    void setRunnableGms(FileMeta *gmsFile = nullptr);
    FileId runFileId() const;
    QString lstFileName() const;
    void setLstFileName(const QString &lstFileName);
    QString tooltip() override;
    QString lstErrorText(int line);
    void setLstErrorText(int line, QString text);
    void clearLstErrorTexts();
    bool hasLstErrorText( int line = -1);
    bool isProcess(const AbstractProcess *process) const;
    void jumpToFirstError(bool focus);
    QProcess::ProcessState gamsProcessState() const;
    GamsProcess *gamsProcess() const;

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

private:
    std::unique_ptr<GamsProcess> mGamsProcess;
    FileMeta* mGmsFile = nullptr;
    QString mGmsFileName;
    QString mLstFileName;
    ProjectLogNode* mLogNode = nullptr;
    QHash<int, QString> mLstErrorTexts;

};


class ProjectRootNode : public ProjectGroupNode
{
    Q_OBJECT
public:
    ProjectRootNode(ProjectRepo *projectRepo);
    ~ProjectRootNode() {}
    ProjectRepo *projectRepo() const;

private:
    friend class ProjectRepo;
    void setParentNode(ProjectGroupNode *parent);
    void init(ProjectRepo* projectRepo);

private:
    ProjectRepo* mRepo = nullptr;
};

} // namespace studio
} // namespace gams

#endif // PROJECTGROUPNODE_H
