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
#include "gamsproperties.h"
#include "projectabstractnode.h"
#include "syntax/textmark.h"

namespace gams {
namespace studio {

class ProjectLogNode;
class ProjectFileNode;
class GamsProcess;
class TextMarkList;

class ProjectGroupNode : public ProjectAbstractNode
{
    Q_OBJECT

public:
    virtual ~ProjectGroupNode() override;
    void setFlag(ContextFlag flag, bool value = true) override;
    void unsetFlag(ContextFlag flag) override;

    void setLocation(const QString &location) override;

    int childCount() const override;
    int indexOf(ProjectAbstractNode *child);
    ProjectAbstractNode* childEntry(int index) const override;
    ProjectAbstractNode* findNode(QString filePath);
    ProjectFileNode* findFile(QString filePath);
    QIcon icon() override;

    QString runnableGms();
    void setRunnableGms(ProjectFileNode *gmsFileNode);
    void removeRunnableGms();
    ProjectLogNode* logNode() const;

    GamsProcess* gamsProcess();
    QProcess::ProcessState gamsProcessState() const;

    void attachFile(const QString &filepath);
    void detachFile(const QString &filepath);
    void updateChildNodes();
    void jumpToFirstError(bool focus);

    QString lstErrorText(int line);
    void setLstErrorText(int line, QString text);
    void clearLstErrorTexts();
    bool hasLstErrorText( int line = -1);
    void saveGroup();

    void dumpMarks();
    QString tooltip() override;

    void addRunParametersHistory(QString option);
    QStringList getRunParametersHistory();

    GamsProperties &gamsProperties();
    void setGamsProperties(GamsProperties &gamsProps);

    QString lstFile() const;
    void setLstFile(const QString &lstFile);

    QString inputFile() const;
    void setInputFile(const QString &inputFile);

signals:
    void gamsProcessStateChanged(ProjectGroupNode* group);
    void removeNode(ProjectAbstractNode *node);
    void requestNode(QString name, QString location, ProjectGroupNode* parent = nullptr);
    void findOrCreateFileNode(QString filePath, ProjectFileNode *&resultFile, ProjectGroupNode* fileGroup = nullptr);

protected slots:
    void onGamsProcessStateChanged(QProcess::ProcessState newState);

protected:
    friend class ProjectRepo;
    friend class ProjectAbstractNode;
    friend class ProjectFileNode;
    friend class ProjectLogNode;

    ProjectGroupNode(FileId id, QString name, QString location, QString fileName);
    int peekIndex(const QString &location, bool* hit = nullptr);
    void insertChild(ProjectAbstractNode *child);
    void removeChild(ProjectAbstractNode *child);
    void checkFlags() override;
    void setLogNode(ProjectLogNode* logNode);
    void updateRunState(const QProcess::ProcessState &state);
    void addMark(const QString &filePath, TextMark* mark);
    TextMarkList* marks(const QString &fileName);
    void removeMarks(QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());
    void removeMarks(QString fileName, QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());

private:
    GamsProperties mGamsProps;
    QStringList mRunParametersHistory;
    QList<ProjectAbstractNode*> mChildList;
    ProjectLogNode* mLogNode = nullptr;
    std::unique_ptr<GamsProcess> mGamsProcess;
    QFileInfoList mAttachedFiles;

    QString mInputFile;
    QString mLstFile;

    QHash<int, QString> mLstErrorTexts;
    QHash<QString, TextMarkList*> mMarksForFilenames;

};

} // namespace studio
} // namespace gams

#endif // PROJECTGROUPNODE_H
