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
    if (TextView *tv = ViewHelper::toTextView(file()->editors().first()))
        tv->prepareRun();
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
        delete mLogFile;
        mLogFile = nullptr;
    }
    mRepaintCount = -1;
    mErrorCount = 0;
    for (QWidget *edit: file()->editors())
        if (TextView* tv = ViewHelper::toTextView(edit)) tv->endRun();
}

void ProjectLogNode::addProcessDataX(const QByteArray &data)
{
    StudioSettings* settings = SettingsLocator::settings();

    if (!mLogFile && settings->writeLog())
        mLogFile = new DynamicFile(location(), settings->nrLogBackups(), this);

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
    mLineBuffer.append(text);
    while (true) {
        bool createErrors = true; // (mErrorCount < 50);
        if (mLineBuffer.indexOf(rEx, from, &match) < 0) {
            mLineBuffer.remove(0, from);
            break;
        }
        QString line = mLineBuffer.mid(from, match.capturedStart());
        QVector<LinkData> marks;
        bool hasError = false;
        QString newLine = extractLinks(line, state, marks, createErrors, hasError);
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
            newLine = extractLinks(line, state, marks, createErrors, hasError);
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

            if (hasError) {
//                if (!createErrors) {
//                    LinksCache lc;
//                    lc.line = lineNr;
//                    lc.text = line;
//                }
                ++mErrorCount;
            }

            if (marks.size() && createErrors) {
                int size = marks.length()==0 ? 0 : newLine.length()-marks.first().col;
                for (LinkData mark: marks) {
                    if (mark.textMark->type() == TextMark::error) ++mErrorCount;
                    TextMark* tm = textMarkRepo()->createMark(file()->id(), runGroupId(), TextMark::link
                                                              , -1, lineNr, mark.col, size);
                    if (mark.textMark) {
                        tm->setRefMark(mark.textMark);
                        if (mark.textMark->fileKind() == FileKind::Lst)
                            mLastLstLink = mark.textMark;
                    }
                    size = -1;
                }
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
        if (mRepaintCount >= 0 && ++mRepaintCount > 200) {
            mRepaintCount = 0;
            repaint();
        }
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
                                     , QVector<ProjectLogNode::LinkData> &marks, bool createMarks, bool &hasError)
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
    bool isRuntimeError = false;
    int lstColStart = 4;
    int posA = 0;
    int posB = 0;
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
        int size;
        int colStart = 0;
        posB = 0;
        if (line.midRef(9, 9) == " at line ") {
            isValidError = false;
            isRuntimeError = true;
            mCurrentErrorHint.errNr = 0;
            result = capture(line, posA, posB, 0, ':').toString();
            // TODO(JM) review for the case the file is in a sub-directory
            fName = mRunGroup->location() + '/' + mLastSourceFile;
            if (posB+2 < line.length()) {
                int subLen = (line.contains('[') ? line.indexOf('['): line.length()) - (posB+2);
                mCurrentErrorHint.text = line.mid(posB+2, subLen);
            }
            lineNr = errNr-1;
            size = 0;
            colStart = -1;
        } else {
            lstColStart = -1;
            mCurrentErrorHint.errNr = ok ? errNr : 0;
            result = capture(line, posA, posB, 0, '[').toString();
            fName = QDir::fromNativeSeparators(capture(line, posA, posB, 6, '"').toString());
            lineNr = capture(line, posA, posB, 2, ',').toInt()-1;
            size = capture(line, posA, posB, 1, ']').toInt()-1;
            posB++;
            if (mRepaintCount < 0) mRepaintCount = 0;
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
                hasError = true;
                if (createMarks)
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
//        isGamsLine = true;
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
            if (line.midRef(posB+1,4) == "LST:") {
                if (isRuntimeError) {
                    tmType = TextMark::error;
                    hasError = true;
                }
                int lineNr = capture(line, posA, posB, 5, ']').toInt()-1;
                mCurrentErrorHint.lstLine = lineNr;
                posB++;
                LinkData mark;
                mark.col = lstColStart;
                mark.size = (lstColStart<0) ? 0 : result.length() - mark.col - 1;

                if (!mLstNode) {
                    mLstNode = mRunGroup->findFile(mRunGroup->parameter("lst"));
                    if (!mLstNode) {
                        QFileInfo fi(mRunGroup->parameter("lst"));
                        mLstNode = projectRepo()->findOrCreateFileNode(mRunGroup->parameter("lst"), mRunGroup, &FileType::from(FileKind::Lst));
                        if (!mLstNode) {
                            errFound = false;
                            continue;
                        }
                    }
                }
                if (createMarks)
                    mark.textMark = textMarkRepo()->createMark(mLstNode->file()->id(), runGroupId(), tmType
                                                               , mCurrentErrorHint.lstLine, lineNr, 0, 0);
                errFound = false;
                if (errMark) {
                    errMark->setValue(mCurrentErrorHint.lstLine);
                    mark.textMark->setRefMark(errMark);
                    errMark->setRefMark(mark.textMark);
                }
                marks << mark;

            } else if (line.midRef(posB+1,4) == "FIL:" || line.midRef(posB+1,4) == "REF:") {
                LinkData mark;
                QString fName = QDir::fromNativeSeparators(capture(line, posA, posB, 6, '"').toString());
                int lineNr = capture(line, posA, posB, 2, ',').toInt()-1;
                int col = capture(line, posA, posB, 1, ']').toInt()-1;
                posB++;

                mark.col = 4;
                mark.size = result.length() - mark.col - 1;

                FileMeta *file = fileRepo()->findOrCreateFileMeta(fName);
                if (createMarks)
                    mark.textMark = textMarkRepo()->createMark(file->id(), runGroupId(), tmType
                                                               , mCurrentErrorHint.lstLine, lineNr, 0, col);
                if (mRunGroup->findFile(file))
                    errFound = false;
                else
                    state = Outside;
                marks << mark;

            } else if (line.midRef(posB+1,4) == "TIT:") {
                return QString();
            } else {
                // no link reference: restore missing braces
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

void ProjectLogNode::repaint()
{
    ProcessLogEdit *ed = ViewHelper::toLogEdit(mFileMeta->topEditor());
    if (ed) {
        ed->viewport()->repaint();
    }
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

void ProjectLogNode::addProcessData(const QByteArray &data)
{

    StudioSettings* settings = SettingsLocator::settings();

    if (!mLogFile && settings->writeLog())
        mLogFile = new DynamicFile(location(), settings->nrLogBackups(), this);

    if (!document())
        EXCEPT() << "no log-document to add process data";

    LogParser::ExtractionState state = LogParser::Outside;
    bool conceal = false;
    int from = 0;
    int to = 0;
    int next = -1;
    while (to < data.length()) {
        if (data.at(to) == '\n') next = to+1;
        else if (data.at(to) == '\r') {
            if (to == data.length()-1)
                next = to+1;
            else if (data.at(to) != '\n') {
                next = to+1;
                conceal = true;
            } else
                next = to+2;
        }
        if (next < 0) {
            ++to;
            continue;
        }
        int len = to-from;
        QString line;
        if (len > 0) {
            LogParser parser(textMarkRepo(), fileRepo(), file()->id(), mRunGroup, file()->codec());
            parser.setDebugMode(debugMode());
            bool hasError = false;
            QByteArray lineData = data.mid(from, len);
            QStringList lines = parser.parseLine(lineData, state, hasError, mbState); // 1 line (2 lines on debugging)

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
            if (mConceal && !line.isNull()) {
                cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
            }
            if (lines.size() > 1) {
                QTextCharFormat fmtk;
                fmtk.setForeground(QColor(120,150,100));
                cursor.insertText(lines.first(), fmtk);
                QTextCharFormat fmt;
                cursor.insertText("\n", fmt);
            }
            cursor.insertText(lines.last()+"\n");
            if (mLogFile) mLogFile->appendLine(line);
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
        }
        document()->setModified(false);

        from = next;
        to = next;
        conceal = false;
    }


}

} // namespace studio
} // namespace gams
