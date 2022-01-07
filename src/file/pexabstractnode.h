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
#ifndef PEXABSTRACTNODE_H
#define PEXABSTRACTNODE_H

#include <QObject>

#include "lxiviewer/lxiviewer.h"
#include "gdxviewer/gdxviewer.h"
#include "editors/codeedit.h"
#include "editors/processlogedit.h"
#include "common.h"

namespace gams {
namespace studio {

class ProjectRootNode;
class PExGroupNode;
class PExProjectNode;
class PExFileNode;
class PExLogNode;
class ProjectRepo;
class FileMetaRepo;
class TextMarkRepo;

class PExAbstractNode : public QObject
{
    Q_OBJECT

public:
    virtual ~PExAbstractNode() override;

    NodeId id() const;

    virtual const ProjectRootNode *root() const;
    virtual ProjectRepo *projectRepo() const;
    virtual FileMetaRepo *fileRepo() const;
    virtual TextMarkRepo *textMarkRepo() const;

    /// The raw name of this node.
    /// \param mod The kind of modification applied to the raw name
    /// \return The requested name of this node.
    virtual QString name(NameModifier mod = NameModifier::raw) const;

    /// Sets the raw name of this node.
    /// \param name The raw name for this node.
    void setName(const QString& name);

    /// The icon for this file type.
    /// \return The icon for this file type.
    virtual QIcon icon(QIcon::Mode mode = QIcon::Normal, int alpha = 100) = 0;

    virtual void setParentNode(PExGroupNode *parent);
    PExGroupNode* parentNode() const;
    virtual PExProjectNode *assignedProject();

    /// \brief File node type.
    /// \return Returns the file node type as <c>int</c>.
    NodeType type() const;
    virtual QString tooltip()=0;

    const ProjectRootNode *toRoot() const;
    const PExGroupNode* toGroup() const;
    PExGroupNode* toGroup();
    const PExProjectNode *toProject() const;
    PExProjectNode *toProject();
    const PExFileNode* toFile() const;
    PExFileNode* toFile();
    const PExLogNode* toLog() const;

    bool debugMode() const;

signals:
    void changed(NodeId nodeId);

protected:
    PExAbstractNode(QString name, NodeType type);

private:
    static NodeId mNextNodeId;
    NodeId mId;
    PExGroupNode* mParent;
    QString mName;
    NodeType mType;
    bool mDebugMode = false;
};

} // namespace studio
} // namespace gams

#endif // PEXABSTRACTNODE_H
