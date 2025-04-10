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
#include "pexfilenode.h"
#include "pexgroupnode.h"
#include "projectrepo.h"
#include "exception.h"
#include "syntax/textmarkrepo.h"
#include "filemeta.h"
#include "fileicon.h"
#include "viewhelper.h"
#include "editors/codeedit.h"
#include "logger.h"
#include <QScrollBar>
#include <QToolTip>
#include <QDir>

namespace gams {
namespace studio {

PExFileNode::PExFileNode(FileMeta *fileMeta, NodeType type)
    : PExAbstractNode(fileMeta?fileMeta->name():"[NULL]", type), mFileMeta(fileMeta)
{
    if (!mFileMeta) EXCEPT() << "The assigned FileMeta must not be null.";
}

PExFileNode::~PExFileNode()
{}

QIcon PExFileNode::icon(QIcon::Mode mode, int alpha)
{
    PExGroupNode* par = parentNode();
    while (par && !par->toProject()) par = par->parentNode();
    if (!par) return QIcon();
    PExProjectNode *project = par->toProject();
    bool active = project && (project->parameter("gms") == location() || project->parameterFile() == this->file());
    return FileIcon::iconForFileKind(file()->kind(), file()->isReadOnly(), active, mode, alpha);
}

QString PExFileNode::name(NameModifier mod) const
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

bool PExFileNode::isModified() const
{
    return mFileMeta->isModified();
}

QTextDocument *PExFileNode::document() const
{
    return mFileMeta->document();
}

FileMeta *PExFileNode::file() const
{
    return mFileMeta;
}

void PExFileNode::replaceFile(FileMeta *fileMeta)
{
    if (mFileMeta != fileMeta) {
        mFileMeta = fileMeta;
        emit changed(id());
    }
}

QString PExFileNode::location() const
{
    return mFileMeta->location();
}

QString PExFileNode::tooltip()
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
        edFile = QString::number(file()->id());
        edGroup = QString::number(file()->projectId());
    }
    tip += "\nedit: " + edFile + " " + edGroup;
    return tip;
}

NodeId PExFileNode::projectId() const
{
    PExGroupNode* group = parentNode();
    while (group && group->type() != NodeType::project)
        group = group->parentNode();
    if (group)
        return group->toProject()->id();
    return NodeId();
}

} // namespace studio
} // namespace gams
