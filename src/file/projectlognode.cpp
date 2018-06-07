/*
 *
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
#include "file.h"
#include "projectlognode.h"
#include "exception.h"
#include "projectgroupnode.h"
#include "logger.h"
#include "editors/logeditor.h"

namespace gams {
namespace studio {

ProjectLogNode::ProjectLogNode(FileMeta* fileMeta, ProjectRunGroupNode *runGroup)
    : ProjectFileNode(fileMeta, nullptr, NodeType::log)
{
    if (!runGroup) EXCEPT() << "The runGroup must not be null.";
    mRunGroup = runGroup;
    runGroup->setLogNode(this);
}

void ProjectLogNode::resetLst()
{
    mLstNode = nullptr;
}


/*

TextMark*ProjectLogNode::firstErrorMark()
{
    if (!mMarks) return nullptr;
    return mMarks->firstErrorMark();
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

QTextDocument* ProjectLogNode::document() const
{
    return mDocument;
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
    logEdit->setDocument(mDocument);
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

void ProjectLogNode::setParentEntry(ProjectGroupNode* parent)
{
    if (parent) {
        parent->setLogNode(this);
        mMarks = parent->marks(location());
    } else {
        mParent->setLogNode(nullptr);
        mMarks = nullptr;
    }
    mParent = parent;
}

void ProjectLogNode::fileClosed(ProjectFileNode *fc)
{
    if (fc == mLstNode) mLstNode = nullptr;
}

void ProjectLogNode::addProcessData(QString text)
{
    // TODO(JM) while creating refs to lst-file some parameters may influence the correct row-in-lst:
    //          PS (PageSize), PC (PageContr), PW (PageWidth)
    if (!mDocument)
        EXCEPT() << "no log-document to add process data";

    ExtractionState state = Outside;
    QRegularExpressionMatch match;
    QRegularExpression rEx("(\\r\\n?|\\n)");
    int from = 0;
    mLineBuffer.append(text);
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
        if (state >= ProjectFileNode::Exiting)
            parentEntry()->setLstErrorText(mCurrentErrorHint.lstLine, mCurrentErrorHint.text);
        if (state == ProjectFileNode::FollowupError) {
            newLine = extractLinks(line, state, marks);
        }
        QList<int> scrollVal;
        QList<QTextCursor> cursors;
        for (QWidget* w: editors()) {
            AbstractEditor* ed = ProjectFileNode::toAbstractEdit(w);
            if (!ed) continue;
            if (ed->verticalScrollBar()->value() >= ed->verticalScrollBar()->maximum()-1) {
                scrollVal << 0;
                cursors << QTextCursor();
            } else {
                scrollVal << ed->verticalScrollBar()->value();
                cursors << ed->textCursor();
            }
        }
        QTextCursor cursor(mDocument);
        cursor.movePosition(QTextCursor::End);
        if (mConceal && !newLine.isNull()) {
            cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
        if (mDebugLog) {
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
            int lineNr = mDocument->blockCount()-1;
            cursor.insertText(newLine+"\n");
            int size = marks.length()==0 ? 0 : newLine.length()-marks.first().col;
            for (LinkData mark: marks) {
                TextMark* tm = generateTextMark(TextMark::link, mCurrentErrorHint.lstLine, lineNr, mark.col, size);
                if (mark.textMark) {
                    tm->setRefMark(mark.textMark);
                    if (mark.textMark->fileKind() == FileType::Lst)
                        mLastLstLink = mark.textMark;
                    mark.textMark->rehighlight();
                }
                tm->rehighlight();
                size = -1;
            }
        }

        int i = 0;
        for (QWidget* w: editors()) {
            AbstractEditor* ed = ProjectFileNode::toAbstractEdit(w);
            if (!ed) continue;
            if (mJumpToLogEnd || scrollVal[i] == 0) {
                mJumpToLogEnd = false;
                ed->verticalScrollBar()->setValue(ed->verticalScrollBar()->maximum());
            }
            ++i;
        }
        mDocument->setModified(false);
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


QString ProjectLogNode::extractLinks(const QString &line, ProjectFileNode::ExtractionState &state, QList<ProjectLogNode::LinkData> &marks)
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
    if (line.startsWith("*** Error ")) {
        bool ok = false;
        posA = 9;
        while (posA < line.length() && (line.at(posA)<'0' || line.at(posA)>'9')) posA++;
        posB = posA;
        while (posB < line.length() && line.at(posB)>='0' && line.at(posB)<='9') posB++;
        int errNr = line.midRef(posA, posB-posA).toInt(&ok);
        bool isValidError = line.midRef(posB, 4) == " in ";
        mCurrentErrorHint.lstLine = 0;
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
            fName = parentEntry()->location() + '/' + mLastSourceFile;
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
            ProjectFileNode *fc = nullptr;
            if (!fName.isEmpty()) {
                emit findFileNode(fName, &fc, parentEntry());
                if (fc) {
                    mark.textMark = fc->generateTextMark(TextMark::error, mCurrentErrorHint.lstLine, lineNr, colStart, size);
                } else {
                    mark.textMark = generateTextMark(fName, TextMark::error, mCurrentErrorHint.lstLine, lineNr, colStart, size);
                }
            }
            errMark = mark.textMark;
            marks << mark;
            errFound = true;
            mInErrorDescription = true;
        }
    }
    if (line.startsWith("--- ")) {
        int fEnd = line.indexOf('(');
        if (fEnd >= 0) {
            int nrEnd = line.indexOf(')', fEnd);
            bool ok;
            line.mid(fEnd+1, nrEnd-fEnd-1).toInt(&ok);
            if (ok) mLastSourceFile = line.mid(4, fEnd-4);
        }
    }
    while (posA < line.length()) {
        result += capture(line, posA, posB, 0, '[');

        if (posB+5 < line.length()) {
            if (line.midRef(posB+1,4) == "LST:") {
                QString fName = parentEntry()->lstFileName();
                int lineNr = capture(line, posA, posB, 5, ']').toInt()-1;
                posB++;
                LinkData mark;
                mark.col = lstColStart;
                mark.size = (lstColStart<0) ? 0 : result.length() - mark.col - 1;
                if (!mLstNode) {
                    emit findFileNode(fName, &mLstNode, parentEntry());
                }
                if (mLstNode) {
                    mark.textMark = mLstNode->generateTextMark((errFound ? TextMark::link : TextMark::none)
                                                                  , mCurrentErrorHint.lstLine, lineNr, 0, 0);
                    errFound = false;
                } else {
                    mark.textMark = generateTextMark(fName, (errFound ? TextMark::link : TextMark::none)
                                                     , mCurrentErrorHint.lstLine, lineNr, 0, 0);
                    errFound = false;
                }
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

                ProjectFileNode *fc = nullptr;
                emit findFileNode(fName, &fc, parentEntry());
                if (fc) {
                    mark.textMark = fc->generateTextMark((errFound ? TextMark::link : TextMark::none)
                                                         , mCurrentErrorHint.lstLine, lineNr, 0, col);
                    errFound = false;
                } else {
                    mark.textMark = generateTextMark(fName, (errFound ? TextMark::link : TextMark::none)
                                                     , mCurrentErrorHint.lstLine, lineNr, 0, col);
                    state = Outside;
                }
                marks << mark;

            } else if (line.midRef(posB+1,4) == "TIT:") {
                return QString();
            } else {
                result += capture(line, posA, posB, 1, ']');
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

*/

} // namespace studio
} // namespace gams
