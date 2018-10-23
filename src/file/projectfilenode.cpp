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
#include "projectfilenode.h"
#include "projectgroupnode.h"
#include "projectrepo.h"
#include "exception.h"
#include "syntax/textmarkrepo.h"
#include "filemeta.h"
#include "editors/codeedit.h"
#include "logger.h"
#include <QScrollBar>
#include <QToolTip>
#include <QTextCodec>

namespace gams {
namespace studio {

ProjectFileNode::ProjectFileNode(FileMeta *fileMeta, ProjectGroupNode* group, NodeType type)
    : ProjectAbstractNode(fileMeta?fileMeta->name():"[NULL]", type), mFileMeta(fileMeta)
{
    if (!mFileMeta) EXCEPT() << "The assigned FileMeta must not be null.";
    if (group) setParentNode(group);
}

ProjectFileNode::~ProjectFileNode()
{}

void ProjectFileNode::setParentNode(ProjectGroupNode *parent)
{
    ProjectAbstractNode::setParentNode(parent);
    // TODO(JM) setRunId in FileMeta
}

QIcon ProjectFileNode::icon()
{
    ProjectGroupNode* par = parentNode();
    while (par && !par->toRunGroup()) par = par->parentNode();
    QString runMark = par->toRunGroup()->specialFile(FileKind::Gms) == location() ? "-run" : "";
    if (file()->kind() == FileKind::Gms)
        return QIcon(":/img/gams-w"+runMark);
    if (file()->kind() == FileKind::Gdx)
        return QIcon(":/img/database");
    if (file()->kind() == FileKind::Ref)
        return QIcon(":/img/ref-file");
    return QIcon(":/img/file-alt"+runMark);
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
    QString tip = location();
    if (!file()->exists(true)) tip += "\n--missing--";
    if (!debugMode())
        return tip;
    tip += "\nNodeId: "+QString::number(id());
    tip += "\nFileId: " + (file() ? QString::number(file()->id()) : "?");
    tip += "\nParent-NodeId: " + (parentNode() ? QString::number(parentNode()->id()) : "?");
    QString edFile = "-";
    QString edGroup = "-";
    if (file()->editors().size()) {
        if (FileMeta::toAbstractEdit(file()->editors().first())) {
            edFile = QString::number(FileMeta::toAbstractEdit(file()->editors().first())->fileId());
            edGroup = QString::number(FileMeta::toAbstractEdit(file()->editors().first())->groupId());
        }
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

void ProjectFileNode::enhanceMarksFromLst()
{
    if (file()->kind() != FileKind::Lst) return;
    if (!file()->exists(true)) return;
    if (!file()->isOpen()) {
        file()->load(file()->codecMib());
    }
    // TODO(JM) Perform a large-file-test if this should have an own thread
    const LineMarks* marks = textMarkRepo()->marks(file()->id());
    //                     0     1 2       3 4                    5               6           7       8
    //                            (    $nr               |        nr+description      |  descr.   |  any  )
    QRegularExpression rex("\\*{4}((\\s+)\\$(([0-9,]+).*)|\\s{1,3}([0-9]{1,3})\\s+(.*)|\\s\\s+(.*)|\\s(.*))");
    QList<int> lines = marks->uniqueKeys();
    for (int lineNr: lines) {
        QTextBlock block = document()->findBlockByNumber(lineNr).next();
        QList<TextMark*> currentMarks = marks->values(lineNr);
        bool hasErr = false;
        for (TextMark *mark: currentMarks) {
            if (mark->groupId() != runGroupId()) continue;
            if (mark->isErrorRef()) hasErr = true;
            int size = mark->size();
            if (size <= 0) {
                size = block.text().indexOf('$');
                if (size > 0) mark->setSize(size+1);
            }
        }
        if (!hasErr) continue;
        QList<int> errNrs;
        QList<int> errTextNr;
        QStringList errNrsDeb;
        QStringList errText;
        while (block.isValid()) {
            QRegularExpressionMatch match = rex.match(block.text());
            if (!match.hasMatch()) break;
            if (match.capturedLength(4)) { // first line with error numbers and indent
                int ind = 0;
                QString line(match.captured(3));
                QRegularExpression xpNr("[^0-9]*([0-9]+)");
                do {
                    QRegularExpressionMatch nrMatch = xpNr.match(line, ind);
                    if (!nrMatch.hasMatch()) break;
                    errNrsDeb << nrMatch.captured(1);
                    errNrs << nrMatch.captured(1).toInt();
                    ind = nrMatch.capturedEnd(1);
                } while (true);
            } else if (match.capturedLength(5)) { // line with error number and description
                errText << match.captured(5)+"\t"+match.captured(6);
                errTextNr << match.captured(5).toInt();
            } else if (match.capturedLength(7)) { // indented follow-up line for error description
                errText << "\t"+match.captured(7);
                errTextNr << (errTextNr.isEmpty() ? -1 : errTextNr.last());
            } else if (match.capturedLength(8)) { // non-indented line for additional error description
                errText << match.captured(8);
                errTextNr << (errTextNr.isEmpty() ? -1 : errTextNr.last());
            }
            block = block.next();
        }
        QStringList orderedErrText;
        for (int nr = 0; nr < errNrs.size(); ++nr) {
            for (int errLn = 0; errLn < errTextNr.size(); ++errLn) {
                if (errTextNr.at(errLn) == errNrs.at(nr)) orderedErrText << errText.at(errLn);
            }
        }
        assignedRunGroup()->setLstErrorText(lineNr, orderedErrText.join("\n"));
    }
}

} // namespace studio
} // namespace gams
