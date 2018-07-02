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
#ifndef PROJECTABSTRACTNODE_H
#define PROJECTABSTRACTNODE_H

#include <QObject>

#include "common.h"

namespace gams {
namespace studio {

class ProjectRootNode;
class ProjectGroupNode;
class ProjectRunGroupNode;
class ProjectFileNode;
class ProjectLogNode;
class ProjectRepo;
class FileMetaRepo;

class ProjectAbstractNode : public QObject
{   // TODO(AF) Make this thing abstract and use is as a interface for all common functions?
    // TODO(JM) Disagree: to many common members - this would lead to code doubling. If you want an interface,
    //                    it could be set on top of this FileSystemContext (e.g. AbstractContext)
    Q_OBJECT

public:
    virtual ~ProjectAbstractNode();

    NodeId id() const;

    /// The raw name of this node.
    /// \return The raw name of this node.
    virtual QString name(NameModifier mod = NameModifier::raw) const;

    /// Sets the raw name of this node.
    /// \param name The raw name of this node.
    void setName(const QString& name);

    /// The icon for this file type.
    /// \return The icon for this file type.
    virtual QIcon icon() = 0;

    virtual ProjectGroupNode *parentNode() const;
    virtual void setParentNode(ProjectGroupNode *parent);
    const ProjectRunGroupNode *runParentNode() const;
    ProjectRunGroupNode *runParentNode();

    /// \brief File node type.
    /// \return Returns the file node type as <c>int</c>.
    NodeType type() const;
    virtual QString tooltip() = 0;

    const ProjectRootNode *toRoot() const;
    const ProjectGroupNode* toGroup() const;
    ProjectGroupNode* toGroup();
    const ProjectRunGroupNode *toRunGroup() const;
    ProjectRunGroupNode *toRunGroup();
    const ProjectFileNode* toFile() const;
    ProjectFileNode* toFile();
    const ProjectLogNode* toLog() const;

    bool isActive() const;
    void setActive();

    inline const ProjectRootNode *root() const;
    ProjectRepo* projectRepo() const;

//    virtual int childCount() const;
//    virtual ProjectAbstractNode* childEntry(int index) const;

signals:
    void changed(NodeId nodeId);

protected:
//    friend class ProjectLogNode;

    ProjectAbstractNode(QString name, NodeType type);

private:
    static NodeId mNextNodeId;
    NodeId mId;
    ProjectGroupNode* mParent;
    QString mName;
    NodeType mType;
};

} // namespace studio
} // namespace gams

#endif // PROJECTABSTRACTNODE_H
