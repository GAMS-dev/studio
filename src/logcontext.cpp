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
#include "logcontext.h"
#include "exception.h"
#include "filegroupcontext.h"
#include "logger.h"
#include "editors/logeditor.h"

namespace gams {
namespace studio {

LogContext::LogContext(FileId fileId, QString name)
    : FileContext(fileId, name, "[LOG]", FileSystemContext::Log)
{
    mMetrics = FileMetrics(QFileInfo(name+".log"));
    mDocument = new QTextDocument(this);
    mDocument->setDocumentLayout(new QPlainTextDocumentLayout(mDocument));
    mDocument->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void LogContext::clearLog()
{
    document()->clear();
}

void LogContext::markOld()
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
        mLastLstLink = nullptr;
        mConceal = false;
    }
}

QTextDocument* LogContext::document() const
{
    return mDocument;
}

void LogContext::addEditor(QWidget* edit)
{
    if (!edit) return;

    if (editorList().contains(edit)) {
        editorList().move(editorList().indexOf(edit), 0);
        return;
    }
    LogEditor* ptEdit = static_cast<LogEditor*>(edit);
    if (!ptEdit) return;
    ptEdit->setDocument(mDocument);
    FileContext::addEditor(edit);
}

void LogContext::removeEditor(QWidget* edit)
{
    if (!edit) return;
    if (!editorList().contains(edit)) return;

    editorList().append(nullptr);
    FileContext::removeEditor(edit);
    editorList().removeLast();
}

void LogContext::setParentEntry(FileGroupContext* parent)
{
    if (parent) {
        parent->setLogContext(this);
        mMarks = parent->marks(location());
    } else {
        mParent->setLogContext(nullptr);
        mMarks = nullptr;
    }
    mParent = parent;
}

TextMark*LogContext::firstErrorMark()
{
    if (!mMarks) return nullptr;
    return mMarks->firstErrorMark();
}

void LogContext::addProcessData(QProcess::ProcessChannel channel, QString text)
{
    Q_UNUSED(channel)
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
        if (mLastLstLink && state == FileContext::Inside) {
            mLastLstLink->incSpread();
        } else {
            mLastLstLink = nullptr;
        }
        if (state >= FileContext::Exiting)
            parentEntry()->setLstErrorText(mCurrentErrorHint.lstLine, mCurrentErrorHint.text);
        if (state == FileContext::FollowupError) {
            newLine = extractLinks(line, state, marks);
        }
        QList<int> scrollVal;
        QList<QTextCursor> cursors;
        for (QWidget* w: editors()) {
            QPlainTextEdit* ed = FileSystemContext::toPlainEdit(w);
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
            QPlainTextEdit* ed = FileSystemContext::toPlainEdit(w);
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


QString LogContext::extractLinks(const QString &line, FileContext::ExtractionState &state, QList<LogContext::LinkData> &marks)
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
    if (line.startsWith("*** Error ") || line.startsWith("--- Error ")) {
        bool ok = false;
        posA = 9;
        while (posA < line.length() && (line.at(posA)<'0' || line.at(posA)>'9')) posA++;
        posB = posA;
        while (posB < line.length() && line.at(posB)>='0' && line.at(posB)<='9') posB++;
        int errNr = line.midRef(posA, posB-posA).toInt(&ok);
        mCurrentErrorHint.lstLine = 0;
        mCurrentErrorHint.text = "";

        QString fName;
        int lineNr;
        int colNr = -1;
        int colStart = 0;
        posB = 0;
        if (line.midRef(9, 9) == " at line ") {
            mCurrentErrorHint.errNr = 0;
            result = capture(line, posA, posB, 0, ':').toString();
            int from = mDashLine.indexOf(' ')+1;
            fName = parentEntry()->location() + '/' + mDashLine.mid(from, mDashLine.indexOf('(')-from);
            lineNr = errNr;
            colNr = -1;
            colStart = -1;
        } else {
            lstColStart = -1;
            mCurrentErrorHint.errNr = ok ? errNr : 0;
            result = capture(line, posA, posB, 0, '[').toString();
            fName = QDir::fromNativeSeparators(capture(line, posA, posB, 6, '"').toString());
            lineNr = capture(line, posA, posB, 2, ',').toInt()-1;
            colNr = capture(line, posA, posB, 1, ']').toInt()-1;
            posB++;
        }

        LinkData mark;
        mark.col = line.indexOf(" ")+1;
        mark.size = result.length() - mark.col;
        FileContext *fc = nullptr;
        emit findFileContext(fName, &fc, parentEntry());
        if (fc) {
            mark.textMark = fc->generateTextMark(TextMark::error, mCurrentErrorHint.lstLine, lineNr, colStart, colNr);
        } else {
            mark.textMark = generateTextMark(fName, TextMark::error, mCurrentErrorHint.lstLine, lineNr, colStart, colNr);
        }
        errMark = mark.textMark;
        marks << mark;
        errFound = true;
        mInErrorDescription = true;
    }
    if (line.startsWith("--- ")) mDashLine = line;
    while (posA < line.length()) {
        result += capture(line, posA, posB, 0, '[');

        if (posB+5 < line.length()) {
            if (line.midRef(posB+1,4) == "LST:") {
                QString fName = parentEntry()->lstFileName();
                int lineNr = capture(line, posA, posB, 5, ']').toInt()-1;
                posB++;
                LinkData mark;
                mark.col = lstColStart;
                mark.size = (lstColStart<0) ? 0 : result.length() - mark.col;
                if (!mLstContext) {
                    emit findFileContext(fName, &mLstContext, parentEntry());
                }
                if (mLstContext) {
                    mark.textMark = mLstContext->generateTextMark((errFound ? TextMark::link : TextMark::none)
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

                FileContext *fc = nullptr;
                emit findFileContext(fName, &fc, parentEntry());
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

void LogContext::setJumpToLogEnd(bool state)
{
    mJumpToLogEnd = state;
}

} // namespace studio
} // namespace gams
