/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "filecontext.h"
#include "filegroupcontext.h"
#include "exception.h"
#include "codeeditor.h"
#include "logger.h"

namespace gams {
namespace studio {

const QStringList FileContext::mDefaulsCodecs = QStringList() << "Utf-8" << "GB2312" << "Shift-JIS"
                                                              << "System" << "Windows-1250" << "Latin-1";

FileContext::FileContext(int id, QString name, QString location)
    : FileSystemContext(id, name, location, FileSystemContext::File)
{
    mDocument = nullptr;
    mMetrics = FileMetrics(QFileInfo(location));
}

bool FileContext::isModified()
{
    return document() && document()->isModified();
}

void FileContext::save()
{
    if (isModified()) {
        if (location().isEmpty())
            EXCEPT() << "Can't save without file name";
        QFile file(location());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            EXCEPT() << "Can't open the file";
        mMetrics = FileMetrics();
        QTextStream out(&file);
        out.setCodec(mCodec.toLatin1().data());
        out << document()->toPlainText();
        out.flush();
        file.close();
        mMetrics = FileMetrics(QFileInfo(file));
        document()->setModified(false);
    }
}

void FileContext::save(QString filePath)
{
    qDebug() << "saving file to" << filePath;
    if (filePath == "")
        EXCEPT() << "Can't save without file name";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        EXCEPT() << "Can't open the file";
    mMetrics = FileMetrics();
    QTextStream out(&file);
    out.setCodec(mCodec.toLatin1().data());
    out << document()->toPlainText();
    out.flush();
    file.close();
    mMetrics = FileMetrics(QFileInfo(file));
    document()->setModified(false);
}

void FileContext::load(QString codecName)
{
    if (!document())
        EXCEPT() << "There is no document assigned to the file " << location();

    QStringList codecNames = codecName.isEmpty() ? mDefaulsCodecs : QStringList() << codecName;
    QFile file(location());
    if (!file.fileName().isEmpty() && file.exists()) {
        if (!file.open(QFile::ReadOnly | QFile::Text))
            EXCEPT() << "Error opening file " << location();
        mMetrics = FileMetrics();
        const QByteArray data(file.readAll());
        QString text;
        QString nameOfUsedCodec;
        for (QString tcName: codecNames) {
            QTextCodec::ConverterState state;
            QTextCodec *codec = QTextCodec::codecForName(tcName.toLatin1().data());
            if (codec) {
                nameOfUsedCodec = tcName;
                text = codec->toUnicode(data.constData(), data.size(), &state);
                if (state.invalidChars == 0) {
                    qDebug() << "opened with codec " << nameOfUsedCodec;
                    break;
                }
                qDebug() << "Codec " << nameOfUsedCodec << " contains " << QString::number(state.invalidChars) << "invalid chars.";
            } else {
                qDebug() << "System doesn't contain codec " << nameOfUsedCodec;
                nameOfUsedCodec = QString();
            }
        }
        if (!nameOfUsedCodec.isEmpty()) {
            document()->setPlainText(text);
            mCodec = nameOfUsedCodec;
        }
        file.close();
        document()->setModified(false);
        mMetrics = FileMetrics(QFileInfo(file));
        QTimer::singleShot(100, this, &FileContext::updateMarks);
    }
    if (!mWatcher) {
        mWatcher = new QFileSystemWatcher(this);
        connect(mWatcher, &QFileSystemWatcher::fileChanged, this, &FileContext::onFileChangedExtern);
        mWatcher->addPath(location());
    }
}

const QList<QPlainTextEdit*> FileContext::editors() const
{
    return mEditors;
}

void FileContext::setLocation(const QString& _location)
{
    if (_location.isEmpty())
        EXCEPT() << "File can't be set to an empty location.";
    QFileInfo newLoc(_location);
    if (QFileInfo(location()) == newLoc)
        return; // nothing to do

    // TODO(JM) adapt parent group
    if (document())
        document()->setModified(true);
    FileSystemContext::setLocation(_location);
    mMetrics = FileMetrics(newLoc);
}

QIcon FileContext::icon()
{
    if (mMetrics.fileType() == FileType::Gms)
        return QIcon(":/img/gams-w");
    return QIcon(":/img/file-alt");
}

void FileContext::addEditor(QPlainTextEdit* edit)
{
    if (!edit || mEditors.contains(edit))
        return;

    mEditors.append(edit);
    if (mEditors.size() == 1 && !mDocument) {
        document()->setParent(this);
        connect(document(), &QTextDocument::modificationChanged, this, &FileContext::modificationChanged, Qt::UniqueConnection);
        QTimer::singleShot(100, this, &FileContext::updateMarks);
    } else {
        edit->setDocument(document());
    }
    CodeEditor* ce = dynamic_cast<CodeEditor*>(edit);
    if (ce) {
        connect(ce, &CodeEditor::getHintForPos, this, &FileContext::shareHintForPos);
    }
    setFlag(FileSystemContext::cfActive);
}

void FileContext::removeEditor(QPlainTextEdit* edit)
{
    int i = mEditors.indexOf(edit);
    if (i < 0)
        return;
    bool wasModified = isModified();
    mEditors.removeAt(i);
    if (mEditors.isEmpty()) {
        // After removing last editor: paste document-parency back to editor
        edit->document()->setParent(edit);
        unsetFlag(FileSystemContext::cfActive);
        if (wasModified) emit changed(id());
    } else {
        edit->setDocument(document()->clone(edit));
    }
}

void FileContext::removeAllEditors()
{
    auto editors = mEditors;
    for (auto editor : editors) {
        removeEditor(editor);
    }
    mEditors = editors;
}

bool FileContext::hasEditor(QPlainTextEdit* edit)
{
    return mEditors.contains(edit);
}

QTextDocument*FileContext::document()
{
    if (mDocument)
        return mDocument;
    if (mEditors.isEmpty())
        return nullptr;
    return mEditors.first()->document();
}

void FileContext::setKeepDocument(bool keep)
{
    if (keep && !mDocument) {
        if (mEditors.isEmpty()) {
            mDocument = new QTextDocument(this);
            mDocument->setDocumentLayout(new QPlainTextDocumentLayout(mDocument));
            mDocument->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
            connect(mDocument, &QTextDocument::modificationChanged, this, &FileContext::modificationChanged, Qt::UniqueConnection);
        } else {
            mDocument = mEditors.first()->document();
        }
    } else if (!keep && mDocument) {
        if (mEditors.isEmpty()) {
            mDocument->deleteLater();
        }
        mDocument = nullptr;
    }
}

const FileMetrics& FileContext::metrics()
{
    return mMetrics;
}

void FileContext::jumpTo(const QTextCursor &cursor)
{
    if (mEditors.size()) {
        QTextCursor tc(cursor);
        tc.clearSelection();
        mEditors.first()->setTextCursor(tc);
    }
}

void FileContext::markOld()
{
    if (document() && !document()->isEmpty()) {
        QTextCursor cur(document());
        cur.movePosition(QTextCursor::End);
        QTextCharFormat oldFormat = cur.charFormat();
        QTextCharFormat newFormat = oldFormat;
        cur.insertBlock();
        cur.movePosition(QTextCursor::StartOfBlock);
        cur.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        newFormat.setForeground(QColor(80,80,80));
        cur.setCharFormat(newFormat);
        cur.movePosition(QTextCursor::End);
        cur.setBlockCharFormat(oldFormat);
    }
}

void FileContext::addProcessData(QProcess::ProcessChannel channel, QString text)
{
    if (!mDocument)
        EXCEPT() << "no explicit document to add process data";
    ExtractionState state;
    for (QString line: text.split("\n", QString::SkipEmptyParts)) {
        QList<LinkData> marks;
        QString newLine = extractError(line, state, marks);
        if (state != FileContext::Inside) {
            QList<bool> atEnd;
            for (QPlainTextEdit* ed: mEditors) {
                atEnd << ed->textCursor().atEnd();
            }
            QTextCursor cursor(mDocument);
            cursor.movePosition(QTextCursor::End);
            int line = mDocument->lineCount()-1;
            cursor.insertText(newLine+"\n");
            for (LinkData mark: marks) {
                int size = (mark.size<=0) ? newLine.length()-mark.col : mark.size;
                TextMark* tm = generateTextMark(TextMark::link, line, mark.col, size);
                tm->setRefMark(mark.textMark);
            }
            int i = 0;
            for (QPlainTextEdit* ed: mEditors) {
                if (atEnd[i]) {
                    ed->moveCursor(QTextCursor::End);
                }
                ++i;
            }
            mDocument->setModified(false);
        }
    }
}

FileContext::~FileContext()
{
    removeAllEditors();
}

QString FileContext::codec() const
{
    return mCodec;
}

void FileContext::setCodec(const QString& codec)
{
    // TODO(JM) changing the codec must trigger conversion (not necessarily HERE)
    mCodec = codec;
}

const QString FileContext::caption()
{
    return name() + (isModified() ? "*" : "");
}

struct LocalLinkData {
    int sourceLine = 0;
    int sourceCol = 0;
    int localLine = 0;
    int localCol = 0;
    int localPos = 0;
    int errCode = 0;
    int lastPos = 0;
};

void FileContext::parseErrorHints(const QString& text, int startChar, int endChar)
{
    QString part = text.mid(startChar, endChar-startChar);
    QVector<QStringRef> lines = part.splitRef("\n", QString::SkipEmptyParts);
    int len = 0;
    int errCode = 0;
    QString errHint;
//    QString body("<table><tr><td><b>%1</b></td><td>%2</td></tr></table>");
    QRegularExpression regEx("^( *(\\d+) +)");
    for (QStringRef ref: lines) {
        QRegularExpressionMatch match = regEx.match(ref);
        if (match.hasMatch()) {
            if (!len)
                len = match.captured(1).length();
            if (!errHint.isEmpty()) {
                mErrHints.insert(errCode, new GamsErrorHint(errCode, errHint));
            }
            errCode = match.captured(2).toInt();
            errHint = QString::number(errCode) + "\t" +ref.right(ref.length()-len).toString().trimmed();
        } else {
            errHint += "\n\t" + ref.right(ref.length()-len).toString().trimmed();
        }
    }
    if (!errHint.isEmpty()) {
        mErrHints.insert(errCode, new GamsErrorHint(errCode, errHint));
    }
}

void FileContext::clearLinksAndErrorHints()
{
    while (!mErrHints.isEmpty()) {
        int key = mErrHints.constBegin().key();
        delete mErrHints.take(key);
    }
    while (!mLinks.isEmpty()) {
        int key = mLinks.constBegin().key();
        delete mLinks.take(key);
    }
}

QString FileContext::extractError(QString line, ExtractionState &state, QList<LinkData> &marks)
{
    TRACE();
    QString result;
    if (mBeforeErrorExtraction) {
        // look, if we find the start of an error
        QStringList parts = line.split(QRegularExpression("(\\[|]\\[|])"), QString::SkipEmptyParts);
        if (parts.size() > 1) {
            QRegularExpression errRX1("^(\\*{3} Error +(\\d+) in (.*)|ERR:\"([^\"]+)\",(\\d+),(\\d+)|LST:(\\d+))");
            for (QString part: parts) {
                bool ok;
                QRegularExpressionMatch match = errRX1.match(part);
                if (part.startsWith("***")) {
                    result = part;
                    int errNr = match.captured(2).toInt(&ok);
                    if (ok) {
                        mCurrentErrorHint.first = errNr;
                    } else {
                        mCurrentErrorHint.first = 0;
                    }
                    mCurrentErrorHint.second = "";
                }
                if (part.startsWith("ERR")) {
                    LinkData mark;
                    mark.col = result.length()+1;
                    result += "[ERR]";
                    mark.size = result.length() - mark.col - 1;
                    QString fName = QDir::fromNativeSeparators(match.captured(4));
                    int line = match.captured(5).toInt()-1;
                    int col = match.captured(6).toInt()-1;
                    DEB() << "captured fileName: " << fName;
                    emit requestTextMark(TextMark::error, fName, line, 0, col, mark.textMark, parentEntry());
                    marks << mark;
                }
                if (part.startsWith("LST")) {
                    LinkData mark;
                    mark.col = result.length()+1;
                    result += "[LST]";
                    mark.size = result.length() - mark.col - 1;
                    int line = match.captured(7).toInt();
                    QString fName = parentEntry()->lstFileName();
                    DEB() << "captured fileName: " << fName;
                    emit requestTextMark(TextMark::error, fName, line, 0, 0, mark.textMark, parentEntry());
                    marks << mark;
                }
            }
            state = Entering;
            mBeforeErrorExtraction = false;
        } else {
            result = line;
            state = Outside;
        }
    } else {
        if (line.startsWith(" ")) {
            mCurrentErrorHint.second += line;
            state = Inside;
            result = line;
            // TODO(JM) add to description
        } else {
            result = line;
            state = Exiting;
            mBeforeErrorExtraction = true;
        }
    }
    return result;
}

TextMark* FileContext::generateTextMark(TextMark::Type tmType, int line, int column, int size)
{
    TRACE();
    TextMark* res = new TextMark(tmType);
    res->mark(this, line, column, size);
    DEB() << "TextMark: " << int(tmType) << "  pos: " << line << " col " << column << "_" << size;
    mTextMarks.insert(line, res);
    markLink(res);
    return res;
}

void FileContext::markLink(TextMark* mark)
{
    TRACE();
    DEB() << "--> FILE: " << name();
    if (!mEditors.size() || !mark || mark->textCursor().isNull()) return;
    bool mod = document()->isModified();
    QPlainTextEdit *edit = mEditors.first();
    QTextCursor oldCur = QTextCursor(edit->document());
    QTextCursor cur = mark->textCursor();
    DEB() << "Selection: " << cur.selectedText();
    QTextCharFormat oldFormat = cur.charFormat();
    QTextCharFormat newFormat = oldFormat;
    if (mark->type() == TextMark::error) {
        DEB() << int(mark->type());
        newFormat.setUnderlineColor(Qt::red);
        newFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        cur.setCharFormat(newFormat);
        cur.setPosition(mark->textCursor().selectionEnd());
        cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        newFormat.setBackground(QColor(225,200,255));
        newFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        cur.setCharFormat(newFormat);
    }
    if (mark->type() == TextMark::link) {
        DEB() << "Link: " << mark->line() << ", " << cur.selectedText();
        newFormat.setForeground(Qt::blue);
        newFormat.setUnderlineColor(Qt::blue);
        newFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        cur.setCharFormat(newFormat);
    }
//    cur.setPosition(to, QTextCursor::KeepAnchor);
//    cur.setPosition(to);
//    cur.setCharFormat(oldFormat);
    edit->setTextCursor(oldCur);
    document()->setModified(mod);
}

void FileContext::removeTextMarks(TextMark::Type tmType)
{
    removeTextMarks(QSet<TextMark::Type>() << tmType);
}

void FileContext::removeTextMarks(QSet<TextMark::Type> tmTypes)
{
    QHash<int, TextMark*>::iterator i = mTextMarks.begin();
    while (i != mTextMarks.end()) {
        if (tmTypes.contains(i.value()->type()) || tmTypes.contains(TextMark::all)) {
            TextMark* tm = i.value();
            i = mTextMarks.erase(i);
            delete tm;
        } else {
            ++i;
        }
    }
    for (QPlainTextEdit* ed: mEditors) {
        ed->update(); // trigger delayed repaint
    }
}

void FileContext::modificationChanged(bool modiState)
{
    Q_UNUSED(modiState);
    emit changed(id());
}

void FileContext::shareHintForPos(QPlainTextEdit* sender, QPoint pos, QString& hint, QTextCursor& cursor)
{
    // TODO(JM) map pos to row,col
    QTextCursor cur = sender->cursorForPosition(pos);
    for (LinkReference* lr: mLinks) {
        if (cur.block().blockNumber() == lr->line || cur.block().blockNumber() == lr->line-1) {
            GamsErrorHint *eh = mErrHints.value(lr->errCode);
            if (eh) {
                cursor.setPosition(lr->source.anchor() + lr->col-1);
                hint = eh->hint;
                break;
            }
        }
    }
}

void FileContext::updateMarks()
{
    for (TextMark* mark: mTextMarks) {
        mark->updateCursor();
        markLink(mark);
    }
}

void FileContext::onFileChangedExtern(QString filepath)
{
    qDebug() << "changed: " << filepath;
    QFileInfo fi(filepath);
    FileMetrics::ChangeKind changeKind = mMetrics.check(fi);
    if (changeKind == FileMetrics::ckSkip) return;
    if (changeKind == FileMetrics::ckUnchanged) return;
    if (!fi.exists()) {
        // file has been renamed or deleted
        if (document()) document()->setModified();
        emit deletedExtern(id());
    }
    if (changeKind == FileMetrics::ckModified) {
        // file changed externally
        emit modifiedExtern(id());
    }
}

} // namespace studio
} // namespace gams
