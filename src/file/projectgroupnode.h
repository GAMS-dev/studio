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

namespace gams {
namespace studio {

class ProjectLogNode;
class ProjectFileNode;
class GamsProcess;
class TextMarkRepo;
class FileMeta;
class FileMetaRepo;

class ProjectGroupNode : public ProjectAbstractNode
{
    Q_OBJECT
public:
    virtual ~ProjectGroupNode() override;

    QIcon icon() override;
    int childCount() const;
    ProjectAbstractNode* childEntry(int index) const;
    int indexOf(ProjectAbstractNode *child);
    virtual QString location() const;
    QString tooltip() override;

protected:
    friend class ProjectRepo;
    friend class ProjectAbstractNode;
    friend class ProjectLogNode;
    friend class ProjectFileNode;

    ProjectGroupNode(QString name, NodeType type);
    void insertChild(ProjectAbstractNode *child);
    void removeChild(ProjectAbstractNode *child);
    void setLocation(const QString &location);

private:
    QList<ProjectAbstractNode*> mChildList;
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
//    QProcess::ProcessState gamsProcessState() const;

//    void attachFile(const QString &filepath);
//    void detachFile(const QString &filepath);
//    void updateChildNodes();
//    void jumpToFirstError(bool focus);

//    QString lstErrorText(int line);
//    void setLstErrorText(int line, QString text);
//    void clearLstErrorTexts();
//    bool hasLstErrorText( int line = -1);
//    void saveGroup();

//    void dumpMarks();

//signals:
//    void gamsProcessStateChanged(ProjectGroupNode* group);
//    void removeNode(ProjectAbstractNode *node);
//    void requestNode(QString name, QString location, ProjectGroupNode* parent = nullptr);
//    void findOrCreateFileNode(QString filePath, ProjectFileNode *&resultFile, ProjectGroupNode* fileGroup = nullptr);

//protected slots:
//    void onGamsProcessStateChanged(QProcess::ProcessState newState);

//protected:

//    void checkFlags() override;
//    void updateRunState(const QProcess::ProcessState &state);
//    void addMark(const QString &filePath, TextMark* mark);
//    TextMarkRepo* marks(const QString &fileName);
//    void removeMarks(QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());
//    void removeMarks(QString fileName, QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());
//    int peekIndex(const QString &name, bool* hit = nullptr);

//private:
//    QFileInfoList mAttachedFiles;

//    QHash<int, QString> mLstErrorTexts;
//    QHash<QString, TextMarkRepo*> mMarksForFilenames;

};

class ProjectRunGroupNode : public ProjectGroupNode
{
    Q_OBJECT
public:
    ProjectLogNode* logNode() const;
    void setLogNode(ProjectLogNode* logNode);
    ProjectLogNode* getOrCreateLogNode(TextMarkRepo* textMarkRepo, FileMetaRepo* fileMetaRepo);
    QString runnableGms() const;
    void setRunnableGms(ProjectFileNode *gmsFileNode);
    void removeRunnableGms();
    QString lstFileName() const;
    void setLstFileName(const QString &lstFileName);
    QString tooltip() override;

protected:
    friend class ProjectRepo;
    friend class ProjectAbstractNode;
    friend class ProjectLogNode;
    friend class ProjectFileNode;

    ProjectRunGroupNode(QString name, FileMeta *runFileMeta);

private:
    std::unique_ptr<GamsProcess> mGamsProcess;
    QString mGmsFileName;
    QString mLstFileName;
    ProjectLogNode* mLogNode = nullptr;
};

class ProjectRootNode : public ProjectGroupNode
{
    Q_OBJECT
public:
    ProjectRootNode();
    ~ProjectRootNode() {}
    void setParentNode(ProjectRunGroupNode *parent);
    ProjectRepo *repo() const;
    TextMarkRepo *textMarkRepo() const;

private:
    friend class ProjectRepo;
    void init(ProjectRepo* repo, TextMarkRepo* textMarkRepo);

private:
    ProjectRepo* mRepo = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;
};

} // namespace studio
} // namespace gams

#endif // PROJECTGROUPNODE_H
