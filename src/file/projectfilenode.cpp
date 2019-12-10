/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "projectfilenode.h"
#include "projectgroupnode.h"
#include "projectrepo.h"
#include "exception.h"
#include "syntax/textmarkrepo.h"
#include "filemeta.h"
#include "editors/codeedit.h"
#include "logger.h"
#include "editors/viewhelper.h"
#include <QScrollBar>
#include <QToolTip>
#include <QTextCodec>
#include <QDir>

namespace gams {
namespace studio {

ProjectFileNode::ProjectFileNode(FileMeta *fileMeta, NodeType type)
    : ProjectAbstractNode(fileMeta?fileMeta->name():"[NULL]", type), mFileMeta(fileMeta)
{
    if (!mFileMeta) EXCEPT() << "The assigned FileMeta must not be null.";
}

ProjectFileNode::~ProjectFileNode()
{}

void ProjectFileNode::setParentNode(ProjectGroupNode *parent)
{
    ProjectAbstractNode::setParentNode(parent);
}

QIcon ProjectFileNode::icon()
{
    ProjectGroupNode* par = parentNode();
    while (par && !par->toRunGroup()) par = par->parentNode();
    if (!par) return QIcon();
    QString runMark = par->toRunGroup()->parameter("gms") == location() ? "-run" : "";
    if (file()->kind() == FileKind::Gms)
        return Scheme::icon(":/img/gams-w"+runMark);
    if (file()->kind() == FileKind::Gdx)
        return Scheme::icon(":/img/database");
    if (file()->kind() == FileKind::Ref)
        return Scheme::icon(":/img/ref-file");
    if (file()->kind() == FileKind::Opt)
        return Scheme::icon(":/img/option-file");
    if (!file()->isReadOnly())
        return Scheme::icon(":/img/file-edit");
    return Scheme::icon(":/img/file-alt"+runMark);
}

QString ProjectFileNode::name(NameModifier mod) const
{
    QString res = mFileMeta->name();
    switch (mod) {
    case NameModifier::editState:
        res += (isModified() ? "*" : "");
        break;
    default:
        break;
    }
    return res;
}

bool ProjectFileNode::isModified() const
{
    return mFileMeta->isModified();
}

QTextDocument *ProjectFileNode::document() const
{
    return mFileMeta->document();
}

FileMeta *ProjectFileNode::file() const
{
    return mFileMeta;
}

void ProjectFileNode::replaceFile(FileMeta *fileMeta)
{
    if (mFileMeta != fileMeta) {
        mFileMeta = fileMeta;
        emit changed(id());
    }
}

QString ProjectFileNode::location() const
{
    return mFileMeta->location();
}

QString ProjectFileNode::tooltip()
{
    QString tip = QDir::toNativeSeparators(location());
    if (!file()->exists(true)) tip += "\n--missing--";
    if (!debugMode())
        return tip;
    tip += "\nNodeId: "+QString::number(id());
    tip += "\nFileId: " + (file() ? QString::number(file()->id()) : "?");
    tip += "\nParent-NodeId: " + (parentNode() ? QString::number(parentNode()->id()) : "?");
    QString edFile = "-";
    QString edGroup = "-";
    if (file()->editors().size()) {
        edFile = QString::number(ViewHelper::fileId(file()->editors().first()));
        edGroup = QString::number(ViewHelper::groupId(file()->editors().first()));
    }
    tip += "\nedit: " + edFile + " " + edGroup;
    return tip;
}

NodeId ProjectFileNode::runGroupId() const
{
    ProjectGroupNode* group = parentNode();
    while (group && group->type() != NodeType::runGroup)
        group = group->parentNode();
    if (group)
        return group->toRunGroup()->id();
    return NodeId();
}

} // namespace studio
} // namespace gams
