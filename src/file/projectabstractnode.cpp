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
#include "projectabstractnode.h"
#include "projectgroupnode.h"
#include "logger.h"

namespace gams {
namespace studio {

ProjectAbstractNode::ProjectAbstractNode(NodeId nodeId, QString name, NodeType type)
    : QObject(), mId(nodeId), mParent(nullptr), mName(name), mType(type)
{}

ProjectAbstractNode::~ProjectAbstractNode()
{
    if (mParent) {
        ProjectGroupNode* group = mParent;
        mParent = nullptr;
        if (group) group->removeChild(this);
    }
}

FileId ProjectAbstractNode::id() const
{
    return mId;
}

const QString ProjectAbstractNode::name(NameModifier mod)
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

ProjectGroupNode* ProjectAbstractNode::parentEntry() const
{
    return mParent;
}

void ProjectAbstractNode::setParentEntry(ProjectGroupNode* parent)
{
    if (parent != mParent) {
        if (mParent) mParent->removeChild(this);
        mParent = parent;
        if (mParent) mParent->insertChild(this);
    }
}

NodeType ProjectAbstractNode::type() const
{
    return mType;
}

//int ProjectAbstractNode::childCount() const
//{
//    return 0;
//}

//ProjectAbstractNode* ProjectAbstractNode::childEntry(int index) const
//{
//    Q_UNUSED(index);
//    return nullptr;
//}


/*

void ProjectAbstractNode::checkFlags()
{
}


bool ProjectAbstractNode::canShowAsTab() const
{
    static QList<int> showableTypes = {NodeType::File};
    return showableTypes.contains(mType);
}

const QString ProjectAbstractNode::caption()
{
    return mName;
}

const QString& ProjectAbstractNode::location() const
{
    return mLocation;
}

void ProjectAbstractNode::setLocation(const QString& location)
{
    if (!location.isEmpty()) {
        QFileInfo fi(location);
        if(!fi.exists()) {
            QFile newFile(location);
            newFile.open(QIODevice::WriteOnly);
            newFile.close();
        }
        mLocation = fi.absoluteFilePath();
        setName(fi.fileName());
    }
}

const ProjectAbstractNode::ContextFlags& ProjectAbstractNode::flags() const
{
    return mFlags;
}

void ProjectAbstractNode::setFlag(ContextFlag flag, bool value)
{
    bool current = testFlag(flag);
    if (current == value) return;
    mFlags.setFlag(flag, value);
    if (mParent)
        mParent->checkFlags();
    emit changed(mId);
}

void ProjectAbstractNode::unsetFlag(ContextFlag flag)
{
    setFlag(flag, false);
}

bool ProjectAbstractNode::testFlag(ProjectAbstractNode::ContextFlag flag)
{
    return mFlags.testFlag(flag);
}

ProjectAbstractNode* ProjectAbstractNode::findFile(QString filePath)
{
    if(location() == filePath)
        return this;
    else
        return nullptr;
}

*/

} // namespace studio
} // namespace gams
