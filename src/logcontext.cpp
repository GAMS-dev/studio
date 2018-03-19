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
        EXCEPT() << "no explicit document to add process data";

    ExtractionState state;
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
        QString newLine = extractError(line, state, marks);
        // store count of followup lines
        if (mLastLstLink && state == FileContext::Inside) {
            mLastLstLink->incSpread();
        } else {
            mLastLstLink = nullptr;
        }
        if (state >= FileContext::Exiting)
            parentEntry()->setLstErrorText(mCurrentErrorHint.lstLine, mCurrentErrorHint.text);
        if (state == FileContext::FollowupError)
            newLine = extractError(line, state, marks);
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
        if (mConceal) {
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
            size = 0;
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

QString LogContext::extractError(QString line, FileContext::ExtractionState& state, QList<LogContext::LinkData>& marks)
{
    QString result;
    if (mInErrorDescription && (line.startsWith("***") || line.startsWith("---"))) {
        state = FollowupError;
        mInErrorDescription = false;
        return QString();
    }
    if (!mInErrorDescription) {
        QRegularExpression errRX1("^([\\*\\-]{3} Error +(\\d+) in (.*)|ERR:\"([^\"]+)\",(\\d+),(\\d+)|LST:(\\d+)|FIL:\"([^\"]+)\",(\\d+),(\\d+))");
        // look for the start of an error
        QRegularExpression exp("(\\[|]\\[|])");
        TextMark* errMark = nullptr;
        bool errFound = false;
        bool trailingBrace = false;
        int pos = 0;
        QString capturedBraces;
        while (pos < line.length()) {
            trailingBrace = false;
            QRegularExpressionMatch match = exp.match(line, pos);
            int posEnd = match.hasMatch() ? match.capturedStart() : line.length();
            if (posEnd <= pos) break;
            QString part = line.mid(pos, posEnd-pos);
            bool ok;
            QRegularExpressionMatch errMatch = errRX1.match(part);
            if (part.startsWith("***") || part.startsWith("---")) {
                result += part;
                int errNr = errMatch.captured(2).toInt(&ok);
                mCurrentErrorHint.errNr = ok ? errNr : 0;
                mCurrentErrorHint.lstLine = 0;
                mCurrentErrorHint.text = "";

            } else if (part.startsWith("ERR")) {
                QString fName = QDir::fromNativeSeparators(errMatch.captured(4));
                int lineNr = errMatch.captured(5).toInt()-1;
                int col = errMatch.captured(6).toInt()-1;
                LinkData mark;
                mark.col = result.indexOf(" ")+1;
                result += " ";
                mark.size = result.length() - mark.col - 1;
                FileContext *fc;
                emit findFileContext(fName, &fc, parentEntry());
                if (fc) {
                    mark.textMark = fc->generateTextMark(TextMark::error, mCurrentErrorHint.lstLine, lineNr, 0, col);
                } else {
                    mark.textMark = generateTextMark(fName, TextMark::error, mCurrentErrorHint.lstLine, lineNr, 0, col);
                }
                errMark = mark.textMark;
                marks << mark;
                errFound = true;
                mInErrorDescription = true;

            } else if (part.startsWith("LST")) {
                QString fName = parentEntry()->lstFileName();
                int lineNr = errMatch.captured(7).toInt()-1;
                LinkData mark;
                mark.col = 4;
                mark.size = result.length() - mark.col - 1;
                FileContext *fc;
                emit findOrCreateFileContext(fName, fc, parentEntry());
                if (fc) {
                    mCurrentErrorHint.lstLine = lineNr;
                    mark.textMark = fc->generateTextMark((errFound ? TextMark::link : TextMark::none)
                                                         , mCurrentErrorHint.lstLine, lineNr, 0, 0);
                    errFound = false;
                } else {
                    result += line;
                    state = Outside;
                    break;
                }
                if (errMark) {
                    errMark->setValue(mCurrentErrorHint.lstLine);
                    mark.textMark->setRefMark(errMark);
                    errMark->setRefMark(mark.textMark);
                }
                marks << mark;
            } else if (part.startsWith("FIL") || part.startsWith("REF")) {
                QString fName = QDir::fromNativeSeparators(errMatch.captured(8));
                LinkData mark;
                int lineNr = errMatch.captured(9).toInt()-1;
                int col = errMatch.captured(10).toInt()-1;
                mark.col = 4;
                mark.size = result.length() - mark.col - 1;

                FileContext *fc;
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
            } else {
                result += capturedBraces+part;
                trailingBrace = true;
            }
            pos = match.hasMatch() ? match.capturedEnd() : line.length();
            capturedBraces = match.captured();
        }
        if (trailingBrace) result += capturedBraces;

    } else {
        if (line.startsWith(" ")) {
            if (mCurrentErrorHint.text.isEmpty()) {
                if (mCurrentErrorHint.errNr)
                    mCurrentErrorHint.text += QString("%1\t").arg(mCurrentErrorHint.errNr)+line.trimmed();
                else
                    mCurrentErrorHint.text += '\t'+line.trimmed();
            } else
                mCurrentErrorHint.text += "\n\t"+line.trimmed();
            state = Inside;
            result += line;
        } else {
            result += line;
            state = Exiting;
            mInErrorDescription = false;
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
