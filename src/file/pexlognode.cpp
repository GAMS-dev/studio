/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include <QScrollBar>
#include <QDir>
#include <QByteArray>
#include <QApplication>
#include "process/abstractprocess.h"
#include "editors/processlogedit.h"
#include "syntax/textmarkrepo.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "viewhelper.h"
#include "exception.h"
#include "file.h"
#include "logger.h"
#include "pexgroupnode.h"
#include "pexlognode.h"
#include "settings.h"
#include "syntax/textmarkrepo.h"

namespace gams {
namespace studio {

PExLogNode::PExLogNode(FileMeta* fileMeta, PExProjectNode *project)
    : PExFileNode(fileMeta, NodeType::log)
{
    if (!project) EXCEPT() << "The project must not be null.";
    mProject = project;
    project->setLogNode(this);
    mbState = nullptr;
    mLogCloser.setSingleShot(true);
    mLogCloser.setInterval(100);
    connect(&mLogCloser, &QTimer::timeout, this, &PExLogNode::closeLog);
}

void PExLogNode::closeLog()
{
    if (mLogFile) {
        delete mLogFile;
        mLogFile = nullptr;
    }
}

PExLogNode::~PExLogNode()
{}

void PExLogNode::resetLst()
{
    mLstNode = nullptr;
}

void PExLogNode::clearLog()
{
    if (TextView *tv = ViewHelper::toTextView(file()->editors().first()))
        tv->reset();
}

void PExLogNode::prepareRun(int logOption)
{
    Settings *settings = Settings::settings();
    if (logOption > 2 && !mLogFile && settings->toBool(skEdWriteLog)) {
        mLogFile = new DynamicFile(location(), settings->toInt(skEdLogBackupCount), this);
    }
    mLogFinished = false;

    bool first = true;
    for (QWidget *wid: file()->editors()) {
        if (TextView *tv = ViewHelper::toTextView(wid)) {
            tv->prepareRun();
            tv->jumpToEnd();
            if (first)
                connect(tv, &TextView::appendLines, this, &PExLogNode::saveLines, Qt::UniqueConnection);
            first = false;
        }
    }
}

void PExLogNode::logDone()
{
    mLogFinished = true;
    mLogCloser.start();
    mRepaintCount = -1;
    mErrorCount = 0;
    for (QWidget *edit: file()->editors())
        if (TextView* tv = ViewHelper::toTextView(edit)) {
            tv->endRun();
            disconnect(tv, &TextView::appendLines, this, &PExLogNode::saveLines);
        }
}

void PExLogNode::setJumpToLogEnd(bool state)
{
    mJumpToLogEnd = state;
}

void PExLogNode::repaint()
{
    if (TextView *ed = ViewHelper::toTextView(mFileMeta->topEditor())) {
        ed->viewport()->repaint();
    }
}

void PExLogNode::saveLines(const QStringList &lines, bool overwritePreviousLine)
{
    if (!mLogFile) return;
    if (!overwritePreviousLine)
        mLogFile->confirmLastLine();
    for (const QString &line: lines) {
        mLogFile->appendLine(line);
    }
    if (mLogFinished) mLogCloser.start();
}

PExFileNode *PExLogNode::lstNode() const
{
    return mLstNode;
}

const ProjectRootNode *PExLogNode::root() const
{
    if (mProject) return mProject->root();
    return nullptr;
}

NodeId PExLogNode::projectId() const
{
    if (mProject) return mProject->id();
    return NodeId();
}

PExProjectNode *PExLogNode::assignedProject()
{
    return mProject;
}

void PExLogNode::linkToProcess(AbstractProcess *process)
{
    QWidget *wid = file()->editors().size() ? file()->editors().first() : nullptr;
    TextView *tv = ViewHelper::toTextView(wid);
    if (tv) connect(process, &AbstractProcess::newStdChannelData, tv, &TextView::addProcessData, Qt::UniqueConnection);
}

} // namespace studio
} // namespace gams
