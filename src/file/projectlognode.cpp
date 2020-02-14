/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include <QTextCodec>
#include <QApplication>
#include "abstractprocess.h"
#include "editors/processlogedit.h"
#include "syntax/textmarkrepo.h"
#include "settingslocator.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "editors/viewhelper.h"
#include "exception.h"
#include "file.h"
#include "logger.h"
#include "projectgroupnode.h"
#include "projectlognode.h"
#include "studiosettings.h"
#include "syntax/textmarkrepo.h"

namespace gams {
namespace studio {

ProjectLogNode::ProjectLogNode(FileMeta* fileMeta, ProjectRunGroupNode *runGroup)
    : ProjectFileNode(fileMeta, NodeType::log)
{
    if (!runGroup) EXCEPT() << "The runGroup must not be null.";
    mRunGroup = runGroup;
    runGroup->setLogNode(this);
    QTextCharFormat errFmt;
    errFmt.setForeground(QColor(180,0,0));
    errFmt.setUnderlineColor(Qt::red);
    errFmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    mFormat << errFmt;
    QTextCharFormat lnkFmt;
    lnkFmt.setForeground(QColor(10,20,255));
    lnkFmt.setUnderlineColor(QColor(10,20,255));
    lnkFmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    mFormat << lnkFmt;
    mbState = nullptr;
    mLogCloser.setSingleShot(true);
    mLogCloser.setInterval(100);
    connect(&mLogCloser, &QTimer::timeout, this, &ProjectLogNode::closeLog);
}

void ProjectLogNode::closeLog()
{
    if (mLogFile) {
        delete mLogFile;
        mLogFile = nullptr;
    }
}

ProjectLogNode::~ProjectLogNode()
{}

void ProjectLogNode::resetLst()
{
    mLstNode = nullptr;
}

void ProjectLogNode::clearLog()
{
    if (TextView *tv = ViewHelper::toTextView(file()->editors().first()))
        tv->reset();
}

void ProjectLogNode::prepareRun()
{
    StudioSettings *settings = SettingsLocator::settings();
    if (!mLogFile && settings->writeLog()) {
        mLogFile = new DynamicFile(location(), settings->nrLogBackups(), this);
    }
    mLogFinished = false;

    bool first = true;
    for (QWidget *wid: file()->editors()) {
        if (TextView *tv = ViewHelper::toTextView(wid)) {
            tv->prepareRun();
            tv->jumpToEnd();
            if (first)
                connect(tv, &TextView::appendLines, this, &ProjectLogNode::saveLines, Qt::UniqueConnection);
            first = false;
        }
    }
}

void ProjectLogNode::logDone()
{
    mLogFinished = true;
    mLogCloser.start();
    mRepaintCount = -1;
    mErrorCount = 0;
    for (QWidget *edit: file()->editors())
        if (TextView* tv = ViewHelper::toTextView(edit)) {
            tv->endRun();
            disconnect(tv, &TextView::appendLines, this, &ProjectLogNode::saveLines);
        }
}

void ProjectLogNode::setJumpToLogEnd(bool state)
{
    mJumpToLogEnd = state;
}

void ProjectLogNode::repaint()
{
    if (TextView *ed = ViewHelper::toTextView(mFileMeta->topEditor())) {
        ed->viewport()->repaint();
    }
}

void ProjectLogNode::saveLines(const QStringList &lines, bool overwritePreviousLine)
{
    if (!mLogFile) return;
    if (!overwritePreviousLine)
        mLogFile->confirmLastLine();
    for (const QString &line: lines) {
        mLogFile->appendLine(line);
    }
    if (mLogFinished) mLogCloser.start();
}

ProjectFileNode *ProjectLogNode::lstNode() const
{
    return mLstNode;
}

const ProjectRootNode *ProjectLogNode::root() const
{
    if (mRunGroup) return mRunGroup->root();
    return nullptr;
}

NodeId ProjectLogNode::runGroupId() const
{
    if (mRunGroup) return mRunGroup->id();
    return NodeId();
}

ProjectRunGroupNode *ProjectLogNode::assignedRunGroup()
{
    return mRunGroup;
}

void ProjectLogNode::linkToProcess(AbstractProcess *process)
{
    QWidget *wid = file()->editors().size() ? file()->editors().first() : nullptr;
    TextView *tv = ViewHelper::toTextView(wid);
    if (tv) connect(process, &AbstractProcess::newStdChannelData, tv, &TextView::addProcessData, Qt::UniqueConnection);
}

} // namespace studio
} // namespace gams
