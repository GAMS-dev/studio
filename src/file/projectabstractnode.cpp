/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "projectabstractnode.h"
#include "projectgroupnode.h"
#include "projectlognode.h"
#include "projectrepo.h"
#include "logger.h"
#include "exception.h"

namespace gams {
namespace studio {

NodeId ProjectAbstractNode::mNextNodeId = 0;

ProjectAbstractNode::ProjectAbstractNode(QString name, NodeType type)
    : QObject(), mId(mNextNodeId++), mParent(nullptr), mName(name), mType(type)
{}

ProjectAbstractNode::~ProjectAbstractNode()
{
    if (mParent) {
        ProjectGroupNode* group = mParent;
        mParent = nullptr;
        if (group) group->removeChild(this);
    }
}

NodeId ProjectAbstractNode::id() const
{
    return mId;
}

NodeType ProjectAbstractNode::type() const
{
    return mType;
}

QString ProjectAbstractNode::name(NameModifier mod) const
{
    Q_UNUSED(mod);
    return mName;
}

void ProjectAbstractNode::setName(const QString& name)
{
    if (mName != name) {
        mName = name;
        emit changed(mId);
    }
}

const ProjectRootNode *ProjectAbstractNode::root() const
{
    const ProjectAbstractNode* par = this;
    while (par->parentNode()) par = par->parentNode();
    return par->toRoot();
}

ProjectRepo *ProjectAbstractNode::projectRepo() const
{
    const ProjectRootNode* rootNode = root();
    if (rootNode) return rootNode->projectRepo();
    return nullptr;
}

FileMetaRepo *ProjectAbstractNode::fileRepo() const
{
    const ProjectRootNode* rootNode = root();
    if (rootNode) return rootNode->fileRepo();
    return nullptr;
}

TextMarkRepo *ProjectAbstractNode::textMarkRepo() const
{
    const ProjectRootNode* rootNode = root();
    if (rootNode) return rootNode->textMarkRepo();
    return nullptr;
}

ProjectGroupNode* ProjectAbstractNode::parentNode() const
{
    return mParent;
}

void ProjectAbstractNode::setParentNode(ProjectGroupNode* parent)
{
    if (parent != mParent) {
        if (mParent) mParent->removeChild(this);
        mParent = parent;
        if (mParent) mParent->appendChild(this);
    }
}

ProjectRunGroupNode *ProjectAbstractNode::assignedRunGroup()
{
    ProjectAbstractNode* node = this;
    while (node && !node->toRunGroup()) {
        node = node->parentNode();
    }
    if (node) return node->toRunGroup();
    return nullptr;
}

const ProjectRootNode *ProjectAbstractNode::toRoot() const
{
    if (mType == NodeType::root) return static_cast<const ProjectRootNode*>(this);
    return nullptr;
}

const ProjectGroupNode *ProjectAbstractNode::toGroup() const
{
    if (mType == NodeType::group) return static_cast<const ProjectGroupNode*>(this);
    if (mType == NodeType::runGroup) return static_cast<const ProjectGroupNode*>(this);
    if (mType == NodeType::root) return static_cast<const ProjectGroupNode*>(this);
    return nullptr;
}

ProjectGroupNode *ProjectAbstractNode::toGroup()
{
    if (mType == NodeType::group) return static_cast<ProjectGroupNode*>(this);
    if (mType == NodeType::runGroup) return static_cast<ProjectGroupNode*>(this);
    if (mType == NodeType::root) return static_cast<ProjectGroupNode*>(this);
    return nullptr;
}

const ProjectRunGroupNode *ProjectAbstractNode::toRunGroup() const
{
    if (mType == NodeType::runGroup) return static_cast<const ProjectRunGroupNode*>(this);
    return nullptr;
}

ProjectRunGroupNode *ProjectAbstractNode::toRunGroup()
{
    if (mType == NodeType::runGroup) return static_cast<ProjectRunGroupNode*>(this);
    return nullptr;
}

const ProjectFileNode *ProjectAbstractNode::toFile() const
{
    if (mType == NodeType::file) return static_cast<const ProjectFileNode*>(this);
    if (mType == NodeType::log) return static_cast<const ProjectFileNode*>(this);
    return nullptr;
}

ProjectFileNode *ProjectAbstractNode::toFile()
{
    if (mType == NodeType::file) return static_cast<ProjectFileNode*>(this);
    if (mType == NodeType::log) return static_cast<ProjectFileNode*>(this);
    return nullptr;
}

const ProjectLogNode *ProjectAbstractNode::toLog() const
{
    if (mType == NodeType::log) return static_cast<const ProjectLogNode*>(this);
    return nullptr;
}

bool ProjectAbstractNode::debugMode() const
{
    return projectRepo() ? projectRepo()->debugMode() : false;
}


} // namespace studio
} // namespace gams
