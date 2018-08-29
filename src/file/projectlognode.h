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
#ifndef PROJECTLOGNODE_H
#define PROJECTLOGNODE_H

#include "projectfilenode.h"
#include "dynamicfile.h"

namespace gams {
namespace studio {

// TODO(JM) integrate Log node as normal node, set a valid location, set a valid name - no "[LOG]" encoding in name
class ProjectLogNode final: public ProjectFileNode
{
public:
    ~ProjectLogNode() override;
    void resetLst();
    void clearLog();
    void markOld();
    void logDone();

//public:
//    void fileClosed(ProjectFileNode* fc);
//    TextMark* firstErrorMark();
    void setDebugLog(bool debugLog = true) {mDebugLog = debugLog;}
    ProjectFileNode *lstNode() const;
    void setLstNode(ProjectFileNode *lstNode);

public slots:
    void addProcessData(const QByteArray &data);
    void setJumpToLogEnd(bool state);

protected:
    friend class ProjectRepo;
    friend class ProjectRunGroupNode;

    ProjectLogNode(FileMeta *fileMeta, ProjectRunGroupNode *assignedRunGroup);
//    void setParentNode(ProjectGroupNode *parent) override;

    struct LinkData {
        TextMark* textMark = nullptr;
        int col = 0;
        int size = 1;
    };
    QString extractLinks(const QString &text, ExtractionState &state, QList<LinkData>& marks);


private:
    ProjectRunGroupNode *mRunGroup = nullptr;
    ProjectFileNode *mLstNode = nullptr;
    struct ErrorData {
        int lstLine = 0;
        int errNr = 0;
        QString text;
    };
    bool mJumpToLogEnd = true;
    bool mInErrorDescription = false;
    ErrorData mCurrentErrorHint;
// //    QSet<FileNode*> mMarkedNodeList;
    QString mLineBuffer;
    TextMark* mLastLstLink = nullptr;
    bool mConceal = false;
    bool mDebugLog = false;
    QString mLastSourceFile;
    DynamicFile *mLogFile = nullptr;
};

} // namespace studio
} // namespace gams

#endif // PROJECTLOGNODE_H
