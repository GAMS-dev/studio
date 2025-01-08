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
#include "pexabstractnode.h"
#include "pexgroupnode.h"
#include "pexlognode.h"
#include "projectrepo.h"
#include "logger.h"
#include "exception.h"

namespace gams {
namespace studio {

NodeId PExAbstractNode::mNextNodeId = 0;

PExAbstractNode::PExAbstractNode(const QString &name, NodeType type)
    : QObject(), mId(mNextNodeId++), mParent(nullptr), mName(name), mType(type)
{}

void PExAbstractNode::setNameExtIntern(const QString &newNameExt)
{
    mNameExt = newNameExt;
}

PExAbstractNode::~PExAbstractNode()
{
    if (mParent) {
        PExGroupNode* group = mParent;
        mParent = nullptr;
        if (group) group->removeChild(this);
    }
}

NodeId PExAbstractNode::id() const
{
    return mId;
}

NodeType PExAbstractNode::type() const
{
    return mType;
}

const QString &PExAbstractNode::nameExt() const
{
    return mNameExt;
}

void PExAbstractNode::setNameExt(const QString &newNameExt)
{
    mNameExt = newNameExt;
    emit changed(id());
}

QString PExAbstractNode::name(NameModifier mod) const
{
    Q_UNUSED(mod)
    return mName;
}

void PExAbstractNode::setName(const QString& name)
{
    mName = name;
    emit changed(mId);
}

const PExRootNode *PExAbstractNode::root() const
{
    const PExAbstractNode* par = this;
    while (par->parentNode()) par = par->parentNode();
    return par->toRoot();
}

ProjectRepo *PExAbstractNode::projectRepo() const
{
    const PExRootNode* rootNode = root();
    if (rootNode) return rootNode->projectRepo();
    return nullptr;
}

FileMetaRepo *PExAbstractNode::fileRepo() const
{
    const PExRootNode* rootNode = root();
    if (rootNode) return rootNode->fileRepo();
    return nullptr;
}

TextMarkRepo *PExAbstractNode::textMarkRepo() const
{
    const PExRootNode* rootNode = root();
    if (rootNode) return rootNode->textMarkRepo();
    return nullptr;
}

PExGroupNode* PExAbstractNode::parentNode() const
{
    return mParent;
}

void PExAbstractNode::setParentNode(PExGroupNode* parent)
{
    if (parent != mParent) {
        if (mParent) mParent->removeChild(this);
        mParent = parent;
        if (mParent) mParent->appendChild(this);
    }
}

const PExProjectNode *PExAbstractNode::assignedProject() const
{
    const PExAbstractNode* node = this;
    while (node && !node->toProject()) {
        node = node->parentNode();
    }
    if (node) return node->toProject();
    return nullptr;
}

PExProjectNode *PExAbstractNode::assignedProject()
{
    PExAbstractNode* node = this;
    while (node && !node->toProject()) {
        node = node->parentNode();
    }
    if (node) return node->toProject();
    return nullptr;
}

const PExRootNode *PExAbstractNode::toRoot() const
{
    if (mType == NodeType::root) return static_cast<const PExRootNode*>(this);
    return nullptr;
}

const PExGroupNode *PExAbstractNode::toGroup() const
{
    if (mType == NodeType::group) return static_cast<const PExGroupNode*>(this);
    if (mType == NodeType::project) return static_cast<const PExGroupNode*>(this);
    if (mType == NodeType::root) return static_cast<const PExGroupNode*>(this);
    return nullptr;
}

PExGroupNode *PExAbstractNode::toGroup()
{
    if (mType == NodeType::group) return static_cast<PExGroupNode*>(this);
    if (mType == NodeType::project) return static_cast<PExGroupNode*>(this);
    if (mType == NodeType::root) return static_cast<PExGroupNode*>(this);
    return nullptr;
}

const PExProjectNode *PExAbstractNode::toProject() const
{
    if (mType == NodeType::project) return static_cast<const PExProjectNode*>(this);
    return nullptr;
}

PExProjectNode *PExAbstractNode::toProject()
{
    if (mType == NodeType::project) return static_cast<PExProjectNode*>(this);
    return nullptr;
}

const PExFileNode *PExAbstractNode::toFile() const
{
    if (mType == NodeType::file) return static_cast<const PExFileNode*>(this);
    if (mType == NodeType::log) return static_cast<const PExFileNode*>(this);
    return nullptr;
}

PExFileNode *PExAbstractNode::toFile()
{
    if (mType == NodeType::file) return static_cast<PExFileNode*>(this);
    if (mType == NodeType::log) return static_cast<PExFileNode*>(this);
    return nullptr;
}

const PExLogNode *PExAbstractNode::toLog() const
{
    if (mType == NodeType::log) return static_cast<const PExLogNode*>(this);
    return nullptr;
}

bool PExAbstractNode::debugMode() const
{
    return projectRepo() ? projectRepo()->debugMode() : false;
}


} // namespace studio
} // namespace gams
