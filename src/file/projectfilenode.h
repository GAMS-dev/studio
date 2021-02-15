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
#ifndef PROJECTFILENODE_H
#define PROJECTFILENODE_H

#include <QFileSystemWatcher>
#include "projectabstractnode.h"
#include "syntax.h"

namespace gams {
namespace studio {

class CodeEdit;
class ProjectGroupNode;
class TextMark;
class FileMeta;
typedef QPair<int,QString> ErrorHint;

///
/// The <c>ProjectFileNode</c> class represents a file. It is derived from <c>ProjectAbstractNode</c>.
/// \see ProjectAbstractNode, ProjectGroupNode, ProjectLogNode
///
class ProjectFileNode : public ProjectAbstractNode
{
    Q_OBJECT
public:
    enum ExtractionState {
        Outside,
        Entering,
        Inside,
        Exiting,
        FollowupError,
    };
    Q_ENUM(ExtractionState)

    ~ProjectFileNode() override;


    /// The icon for this file type.
    /// \return The icon for this file type.
    QIcon icon() override;

    /// The caption of this file, which is its extended display name.
    /// \return The caption of this node.
    QString name(NameModifier mod = NameModifier::raw) const override;
    bool isModified() const;
    QTextDocument* document() const;
    FileMeta* file() const;
    QString location() const;
    QString tooltip() override;
    virtual NodeId runGroupId() const;
//    QTextCodec *codec() const;
//    void enhanceMarksFromLst();

protected:
    friend class ProjectLogNode;
    friend class ProjectRepo;
    friend class FileMeta;

    ProjectFileNode(FileMeta* fileMeta, NodeType type = NodeType::file);
    void setParentNode(ProjectGroupNode *parent) override;
    void replaceFile(FileMeta* fileMeta);

private:
    FileMeta* mFileMeta;
};

} // namespace studio
} // namespace gams

#endif // PROJECTFILENODE_H
