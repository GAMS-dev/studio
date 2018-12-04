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
#include <QScrollBar>
#include <QDir>
#include <QByteArray>
#include <QTextCodec>
#include <QApplication>
#include "file.h"
#include "projectlognode.h"
#include "exception.h"
#include "projectgroupnode.h"
#include "logger.h"
#include "editors/processlogedit.h"
#include "syntax/textmarkrepo.h"
#include "locators/settingslocator.h"
#include "locators/sysloglocator.h"
#include "locators/abstractsystemlogger.h"
#include "studiosettings.h"
#include "editors/viewhelper.h"

namespace gams {
namespace studio {

ProjectLogNode::ProjectLogNode(FileMeta* fileMeta, ProjectRunGroupNode *runGroup)
    : ProjectFileNode(fileMeta, nullptr, NodeType::log)
{
    if (!runGroup) EXCEPT() << "The runGroup must not be null.";
    mRunGroup = runGroup;
    runGroup->setLogNode(this);
}

ProjectLogNode::~ProjectLogNode()
{}

void ProjectLogNode::resetLst()
{
    mLstNode = nullptr;
}

void ProjectLogNode::clearLog()
{
    document()->clear();
}

void ProjectLogNode::markOld()
{
    if (document() && !document()->isEmpty()) {
        QTextCursor cur(document());
        cur.movePosition(QTextCursor::End);
        QTextCharFormat oldFormat = cur.charFormat();
        QTextCharFormat newFormat = oldFormat;
        cur.insertBlock();
        cur.movePosition(QTextCursor::StartOfBlock);
        cur.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        newFormat.setForeground(QColor(165,165,165));
        cur.setCharFormat(newFormat);
        cur.movePosition(QTextCursor::End);
        cur.setBlockCharFormat(oldFormat);
        document()->setModified(false);
        mLastLstLink = nullptr;
        mConceal = false;
    }
}

void ProjectLogNode::logDone()
{
    if (mLogFile) {
        // TODO(JM) rename .log~ to .log
        delete mLogFile;
        mLogFile = nullptr;
    }
}

void ProjectLogNode::addProcessData(const QByteArray &data)
{
    StudioSettings* settings = SettingsLocator::settings();
    if (!mLogFile && settings->writeLog()) mLogFile = new DynamicFile(location(), settings->nrLogBackups(), this);
    // TODO(JM) while creating refs to lst-file some parameters may influence the correct row-in-lst:
    //          PS (PageSize), PC (PageContr), PW (PageWidth)
    if (!document())
        EXCEPT() << "no log-document to add process data";
    QTextCodec::ConverterState convState;
    QString text(file()->codec()->toUnicode(data.constData(), data.size(), &convState));
    if (file()->codec()) {
        text =file()->codec()->toUnicode(data.constData(), data.size(), &convState);
    }
    if (!file()->codec() || convState.invalidChars > 0) {
        QTextCodec* locCodec = QTextCodec::codecForLocale();
        text = locCodec->toUnicode(data.constData(), data.size(), &convState);
    }

    ExtractionState state = Outside;
    QRegularExpressionMatch match;
    QRegularExpression rEx("(\\r\\n?|\\n)");
    int from = 0;
    mLineBuffer.append(data);
    int lstLine = -1;
    while (true) {
        if (mLineBuffer.indexOf(rEx, from, &match) < 0) {
            mLineBuffer.remove(0, from);
            break;
        }
        QString line = mLineBuffer.mid(from, match.capturedStart());
        QList<LinkData> marks;
        QString newLine = extractLinks(line, state, marks);
        // store count of followup lines
        if (mLastLstLink && state == ProjectFileNode::Inside) {
            mLastLstLink->incSpread();
        } else {
            mLastLstLink = nullptr;
        }
        if (state >= ProjectFileNode::Exiting) {
            QString lstErr = assignedRunGroup()->lstErrorText(mCurrentErrorHint.lstLine);
            if (!lstErr.isEmpty()) lstErr += "\n";
            lstErr += mCurrentErrorHint.text;
            assignedRunGroup()->setLstErrorText(mCurrentErrorHint.lstLine, lstErr);
        }
        if (state == ProjectFileNode::FollowupError) {
            newLine = extractLinks(line, state, marks);
        }
        QList<int> scrollVal;
        QList<QTextCursor> cursors;
        for (QWidget* w: file()->editors()) {
            AbstractEdit* ed = ViewHelper::toAbstractEdit(w);
            if (!ed) continue;
            if (ed->verticalScrollBar()->value() >= ed->verticalScrollBar()->maximum()-1) {
                scrollVal << 0;
                cursors << QTextCursor();
            } else {
                scrollVal << ed->verticalScrollBar()->value();
                cursors << ed->textCursor();
            }
        }
        QTextCursor cursor(document());
        cursor.movePosition(QTextCursor::End);
        if (mConceal && !newLine.isNull()) {
            cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
        if (debugMode()) {
            if (mConceal) {
                cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
            }
            QTextCharFormat fmtk;
            fmtk.setForeground(QColor(120,150,100));
            cursor.insertText(line, fmtk);
            QTextCharFormat fmt;
            cursor.insertText("\n", fmt);
        }
        if (!newLine.isNull())  {
            int lineNr = document()->blockCount()-1;
            cursor.insertText(newLine+"\n");
            if (mLogFile) mLogFile->appendLine(newLine);
            int size = marks.length()==0 ? 0 : newLine.length()-marks.first().col;
            for (LinkData mark: marks) {
                TextMark* tm = textMarkRepo()->createMark(file()->id(), runGroupId(), TextMark::link
                                                          , lstLine, lineNr, mark.col, size);
                if (mark.textMark) {
                    tm->setRefMark(mark.textMark);
                    if (mark.textMark->fileKind() == FileKind::Lst)
                        mLastLstLink = mark.textMark;
                    mark.textMark->rehighlight();
                }
                tm->rehighlight();
                size = -1;
            }
        }

        int i = 0;
        for (QWidget* w: file()->editors()) {
            AbstractEdit* ed = ViewHelper::toAbstractEdit(w);
            if (!ed) continue;
            if (mJumpToLogEnd || scrollVal[i] == 0) {
                mJumpToLogEnd = false;
                ed->verticalScrollBar()->setValue(ed->verticalScrollBar()->maximum());
            }
            ++i;
        }
        document()->setModified(false);
        mConceal = match.captured() == "\r";
        from = match.capturedEnd();
    }
}

inline QStringRef capture(const QString &line, int &a, int &b, const int offset, const QChar ch)
{
    a = b + offset;
    b = line.indexOf(ch, a);
    if (b < 0) b = line.length();
    return line.midRef(a, b-a);
}


QString ProjectLogNode::extractLinks(const QString &line, ProjectFileNode::ExtractionState &state
                                     , QList<ProjectLogNode::LinkData> &marks)
{
    if (mInErrorDescription) {
        if (line.startsWith("***") || line.startsWith("---")) {
            state = FollowupError;
            mInErrorDescription = false;
            return QString();
        } else if (line.startsWith(" ")) {
            if (mCurrentErrorHint.text.isEmpty()) {
                if (mCurrentErrorHint.errNr)
                    mCurrentErrorHint.text += QString("%1\t").arg(mCurrentErrorHint.errNr)+line.trimmed();
                else
                    mCurrentErrorHint.text += '\t'+line.trimmed();
            } else
                mCurrentErrorHint.text += "\n\t"+line.trimmed();
            state = Inside;
        } else {
            state = Exiting;
            mInErrorDescription = false;
        }
        return line;
    }

    QString result;
    if (line.isEmpty()) return QString("");
    TextMark* errMark = nullptr;
    bool errFound = false;
    int lstColStart = 4;
    int posA = 0;
    int posB = 0;
    bool isGamsLine = line.startsWith("*** ");
    if (line.startsWith("*** Error ")) {
        bool ok = false;
        posA = 9;
        while (posA < line.length() && (line.at(posA)<'0' || line.at(posA)>'9')) posA++;
        posB = posA;
        while (posB < line.length() && line.at(posB)>='0' && line.at(posB)<='9') posB++;
        int errNr = line.midRef(posA, posB-posA).toInt(&ok);
        bool isValidError = line.midRef(posB, 4) == " in ";
        mCurrentErrorHint.lstLine = -1;
        mCurrentErrorHint.text = "";

        QString fName;
        int lineNr;
        int size = -1;
        int colStart = 0;
        posB = 0;
        if (line.midRef(9, 9) == " at line ") {
            isValidError = true;
            mCurrentErrorHint.errNr = 0;
            result = capture(line, posA, posB, 0, ':').toString();
            // TODO(JM) review for the case the file is in a sub-directory
            fName = mRunGroup->location() + '/' + mLastSourceFile;
            lineNr = errNr-1;
            size = -1;
            colStart = -1;
        } else {
            lstColStart = -1;
            mCurrentErrorHint.errNr = ok ? errNr : 0;
            result = capture(line, posA, posB, 0, '[').toString();
            fName = QDir::fromNativeSeparators(capture(line, posA, posB, 6, '"').toString());
            lineNr = capture(line, posA, posB, 2, ',').toInt()-1;
            size = capture(line, posA, posB, 1, ']').toInt()-1;
            posB++;
        }
        {
            QFileInfo fi(fName);
            if (!fi.exists() || fi.isDir()) fName = "";
        }

        if (isValidError) {
            LinkData mark;
            mark.col = line.indexOf(" ")+1;
            mark.size = result.length() - mark.col;
            if (!fName.isEmpty()) {
                FileMeta *file = fileRepo()->findOrCreateFileMeta(fName);
                mark.textMark = textMarkRepo()->createMark(file->id(), runGroupId(), TextMark::error,
                                                           mCurrentErrorHint.lstLine, lineNr, colStart, size);
            }
            errMark = mark.textMark;
            marks << mark;
            errFound = true;
            mInErrorDescription = true;
        }
    }
    if (line.startsWith("--- ")) {
        isGamsLine = true;
        int fEnd = line.indexOf('(');
        if (fEnd >= 0) {
            int nrEnd = line.indexOf(')', fEnd);
            bool ok;
            line.mid(fEnd+1, nrEnd-fEnd-1).toInt(&ok);
            if (ok) mLastSourceFile = line.mid(4, fEnd-4);
        }
    }

    // Now we should have a system output
    while (posA < line.length()) {
        result += capture(line, posA, posB, 0, '[');

        if (posB+5 < line.length()) {
            TextMark::Type tmType = errFound ? TextMark::link : TextMark::target;
            if (isGamsLine && line.midRef(posB+1,4) == "LST:") {
                int lineNr = capture(line, posA, posB, 5, ']').toInt()-1;
                mCurrentErrorHint.lstLine = lineNr;
                posB++;
                LinkData mark;
                mark.col = lstColStart;
                mark.size = (lstColStart<0) ? 0 : result.length() - mark.col - 1;

                if (!mLstNode) {
                    mLstNode = mRunGroup->findFile(mRunGroup->specialFile(FileKind::Lst));
                    if (!mLstNode) {
                        QFileInfo fi(mRunGroup->specialFile(FileKind::Lst));
                        mLstNode = projectRepo()->findOrCreateFileNode(mRunGroup->specialFile(FileKind::Lst), mRunGroup);
                        if (!mLstNode) {
                            errFound = false;
                            SysLogLocator::systemLog()->appendLog("Could not find lst-file to generate TextMark for."
                                                                  "Did you overwrite default GAMS parameters?", LogMsgType::Error);
                            continue;
                        }
                    }
                }
                mark.textMark = textMarkRepo()->createMark(mLstNode->file()->id(), runGroupId(), tmType
                                                           , mCurrentErrorHint.lstLine, lineNr, 0, 0);
                errFound = false;
                if (errMark) {
                    errMark->setValue(mCurrentErrorHint.lstLine);
                    mark.textMark->setRefMark(errMark);
                    errMark->setRefMark(mark.textMark);
                }
                marks << mark;

            } else if (isGamsLine && (line.midRef(posB+1,4) == "FIL:" || line.midRef(posB+1,4) == "REF:")) {
                LinkData mark;
                QString fName = QDir::fromNativeSeparators(capture(line, posA, posB, 6, '"').toString());
                int lineNr = capture(line, posA, posB, 2, ',').toInt()-1;
                int col = capture(line, posA, posB, 1, ']').toInt()-1;
                posB++;

                mark.col = 4;
                mark.size = result.length() - mark.col - 1;

                FileMeta *file = fileRepo()->findOrCreateFileMeta(fName);
                mark.textMark = textMarkRepo()->createMark(file->id(), runGroupId(), tmType
                                                           , mCurrentErrorHint.lstLine, lineNr, 0, col);
                if (mRunGroup->findFile(file))
                    errFound = false;
                else
                    state = Outside;
                marks << mark;

            } else if (isGamsLine && line.midRef(posB+1,4) == "TIT:") {
                return QString();
            } else {
                // no GAMS line: restore missing braces
                result += '['+capture(line, posA, posB, 1, ']')+']';
                posB++;
            }
        } else {
            if (posB < line.length()) result += line.right(line.length() - posB);
            break;
        }
    }
    return result;
}

void ProjectLogNode::setJumpToLogEnd(bool state)
{
    mJumpToLogEnd = state;
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


/*

TextMark*ProjectLogNode::firstErrorMark()
{
    if (!mMarks) return nullptr;
    return mMarks->firstErrorMark();
}

QTextDocument* ProjectLogNode::document() const
{
    return document();
}

void ProjectLogNode::addEditor(QWidget* edit)
{
    if (!edit) return;

    if (editorList().contains(edit)) {
        editorList().move(editorList().indexOf(edit), 0);
        return;
    }
    LogEditor* logEdit = toLogEdit(edit);
    if (!logEdit) return;
    logEdit->setDocument(document());
    ProjectFileNode::addEditor(edit);
}

void ProjectLogNode::removeEditor(QWidget* edit)
{
    if (!edit) return;
    if (!editorList().contains(edit)) return;

    editorList().append(nullptr);
    ProjectFileNode::removeEditor(edit);
    editorList().removeLast();
}

*/

} // namespace studio
} // namespace gams
