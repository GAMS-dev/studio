#include "logcontext.h"
#include "exception.h"
#include "filegroupcontext.h"
#include "logger.h"

namespace gams {
namespace studio {

LogContext::LogContext(FileId fileId, QString name)
    : FileContext(fileId, name, "", FileSystemContext::Log)
{
    mDocument = new QTextDocument(this);
    mDocument->setDocumentLayout(new QPlainTextDocumentLayout(mDocument));
    mDocument->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    mSyntaxHighlighter = new ErrorHighlighter(this, &mMarks);
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
    }
}

QTextDocument* LogContext::document()
{
    return mDocument;
}

void LogContext::addEditor(QPlainTextEdit* edit)
{
    if (!edit) return;

    if (editorList().contains(edit)) {
        editorList().move(editorList().indexOf(edit), 0);
        return;
    }
    edit->setDocument(mDocument);
    FileContext::addEditor(edit);
}

void LogContext::removeEditor(QPlainTextEdit* edit)
{
    if (!edit) return;

    editorList().append(nullptr);
    FileContext::removeEditor(edit);
    editorList().removeLast();
}

void LogContext::setParentEntry(FileGroupContext* parent)
{
    if (parent){
        parent->setLogContext(this);
    } else {
        mParent->setLogContext(nullptr);
    }
    mParent = parent;
}

TextMark*LogContext::firstErrorMark()
{
    return mMarks.firstErrorMark();
}

void LogContext::addProcessData(QProcess::ProcessChannel channel, QString text)
{
    Q_UNUSED(channel)
    bool debugTheLog = false;
    // TODO(JM) while creating refs to lst-file some parameters may influence the correct row-in-lst:
    //          PS (PageSize), PC (PageContr), PW (PageWidth)
    if (!mDocument)
        EXCEPT() << "no explicit document to add process data";
    if (text.contains("compilation")) {
        qDebug() << "start";
    }
    ExtractionState state;
    QRegularExpression rEx("(\\r?\\n|\\r\\n?)");
    QStringList lines = text.split(rEx);
    if (!mLineBuffer.isEmpty()) {
        lines.replace(0,mLineBuffer+lines.at(0));
    }
    mLineBuffer = lines.last();
    lines.removeAt(lines.count()-1);
    for (QString line: lines) {
        QList<LinkData> marks;
        QString newLine = extractError(line, state, marks);
        // TODO(JM) cleanup usage of createErrorHint
        if (state >= FileContext::Exiting)
            parentEntry()->setLstErrorText(mCurrentErrorHint.lstLine, mCurrentErrorHint.text);
//            emit createErrorHint(mCurrentErrorHint.first, mCurrentErrorHint.second);
        if (state == FileContext::FollowupError)
            newLine = extractError(line, state, marks);
        if (true || state != FileContext::Inside) {
            QList<int> scrollVal;
            QList<QTextCursor> cursors;
            for (QPlainTextEdit* ed: editors()) {
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
            if (debugTheLog) {
                QTextCharFormat fmtk; fmtk.setForeground(QColor(120,150,100));
                cursor.insertText(line, fmtk);
                QTextCharFormat fmt;
                cursor.insertText("\n", fmt);
            }
            int line = mDocument->lineCount()-1;
            cursor.insertText(newLine+"\n");
            int size = marks.length()==0 ? 0 : newLine.length()-marks.first().col;
            for (LinkData mark: marks) {
                TextMark* tm = generateTextMark(TextMark::link, mCurrentErrorHint.lstLine, line, mark.col, size);
                tm->setRefMark(mark.textMark);
                if (mark.textMark) mark.textMark->rehighlight();
                tm->rehighlight();
                size = 0;
            }

            int i = 0;
            for (QPlainTextEdit* ed: editors()) {
                if (mJumpToLogEnd || scrollVal[i] == 0) {
                    mJumpToLogEnd = false;
                    ed->verticalScrollBar()->setValue(ed->verticalScrollBar()->maximum());
                }
                ++i;
            }
            mDocument->setModified(false);
        }
    }
}

QString LogContext::extractError(QString line, FileContext::ExtractionState& state, QList<LogContext::LinkData>& marks)
{
    QString result;
    QRegularExpression errRX1("^([\\*\\-]{3} Error +(\\d+) in (.*)|ERR:\"([^\"]+)\",(\\d+),(\\d+)|LST:(\\d+)|FIL:\"([^\"]+)\",(\\d+),(\\d+))");
    if (mInErrorDescription && (line.startsWith("***") || line.startsWith("---"))) {
        state = FollowupError;
        mInErrorDescription = false;
        return QString();
    }
    if (!mInErrorDescription) {
        // look, if we find the start of an error
        QStringList parts = line.split(QRegularExpression("(\\[|]\\[|])"), QString::SkipEmptyParts);
        if (parts.size() > 1) {
            TextMark* errMark = nullptr;
            bool errFound = false;
            for (QString part: parts) {
                bool ok;
                QRegularExpressionMatch match = errRX1.match(part);
                if (part.startsWith("***") || part.startsWith("---")) {
                    result += part;
                    int errNr = match.captured(2).toInt(&ok);
                    mCurrentErrorHint.errNr = ok ? errNr : 0;
                    mCurrentErrorHint.lstLine = 0;
                    mCurrentErrorHint.text = "";

                } else if (part.startsWith("ERR")) {
                    QString fName = QDir::fromNativeSeparators(match.captured(4));
                    int line = match.captured(5).toInt()-1;
                    int col = match.captured(6).toInt()-1;
                    LinkData mark;
                    mark.col = result.indexOf(" ")+1;
                    result += " ";
                    mark.size = result.length() - mark.col - 1;
                    FileContext *fc;
                    emit findOrCreateFileContext(fName, &fc, parentEntry());
                    if (fc) {
                        mark.textMark = fc->generateTextMark(TextMark::error, mCurrentErrorHint.lstLine, line, 0, col);
                        mMarkedContextList << fc;
                    }
                    errMark = mark.textMark;
                    marks << mark;
                    errFound = true;
                    mInErrorDescription = true;

                } else if (part.startsWith("LST")) {
                    QString fName = parentEntry()->lstFileName();
                    int lineNr = match.captured(7).toInt()-1;
                    LinkData mark;
                    mark.col = 4;
//                    result += QString("[LST:%1]").arg(lineNr+1);
                    mark.size = result.length() - mark.col - 1;
                    FileContext *fc;
                    emit findOrCreateFileContext(fName, &fc, parentEntry());
                    if (fc) {
                        mCurrentErrorHint.lstLine = lineNr;
                        mark.textMark = fc->generateTextMark((errFound ? TextMark::link : TextMark::none)
                                                             , mCurrentErrorHint.lstLine, lineNr, 0, 0);
                        mMarkedContextList << fc;
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
                    QString fName = QDir::fromNativeSeparators(match.captured(8));
                    LinkData mark;
                    int line = match.captured(9).toInt()-1;
                    int col = match.captured(10).toInt()-1;
                    mark.col = 4;
//                    result += QString("[%1]").arg(QFileInfo(fName).suffix().toUpper());
                    mark.size = result.length() - mark.col - 1;

                    FileContext *fc;
                    emit findOrCreateFileContext(fName, &fc, parentEntry());
                    if (fc) {
                        mark.textMark = fc->generateTextMark((errFound ? TextMark::link : TextMark::none)
                                                             , mCurrentErrorHint.lstLine, line, 0, col);
                        mMarkedContextList << fc;
                        errFound = false;
                    } else {
                        state = Outside;
                        break;
                    }
                    marks << mark;
                }
            }
            state = Entering;
        } else {
            result += line;
            state = Outside;
        }
    } else {
        if (line.startsWith(" ")) {
            // TODO(JM) get description from LST-file instead (-> there are more details)
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

void LogContext::clearRecentMarks()
{
    for (FileContext* fc: mMarkedContextList) {
        fc->removeTextMarks(TextMark::all);
    }
    removeTextMarks(TextMark::all);
}

void LogContext::setJumpToLogEnd(bool state)
{
    mJumpToLogEnd = state;
}

} // namespace studio
} // namespace gams
