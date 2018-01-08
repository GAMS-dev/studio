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

FileContext::FileContext(FileId fileId, QString name, QString location, ContextType type)
    : FileSystemContext(fileId, name, location, type)
{
    mMetrics = FileMetrics(QFileInfo(location));
    if (mMetrics.fileType() == FileType::Gms || mMetrics.fileType() == FileType::Txt)
        mSyntaxHighlighter = new SyntaxHighlighter(this, &mMarks);
    else if (mMetrics.fileType() != FileType::Gdx) {
        mSyntaxHighlighter = new ErrorHighlighter(this, &mMarks);
    }
}

QWidgetList& FileContext::editorList()
{
    return mEditors;
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

bool FileContext::isModified()
{
    return document() && document()->isModified();
}

void FileContext::save()
{
    if (isModified())
        save(location());
}

void FileContext::save(QString filePath)
{
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

const FileMetrics& FileContext::metrics()
{
    return mMetrics;
}

const QWidgetList FileContext::editors() const
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
    if (mMetrics.fileType() == FileType::Gdx)
        return QIcon(":/img/database");
    return QIcon(":/img/file-alt");
}

void FileContext::addEditor(QWidget* edit)
{
    if (!edit) return;
    if (mEditors.contains(edit)) {
        mEditors.move(mEditors.indexOf(edit), 0);
        return;
    }
    if (FileSystemContext::editorType(edit) == FileSystemContext::etUndefined)
        EXCEPT() << "Type assignment missing for this editor/viewer";
    mEditors.prepend(edit);
    QPlainTextEdit* ptEdit = FileSystemContext::toPlainEdit(edit);
    CodeEditor* scEdit = FileSystemContext::toCodeEdit(edit);

    if (mEditors.size() == 1) {
        if (ptEdit) {
            ptEdit->document()->setParent(this);
            connect(document(), &QTextDocument::modificationChanged, this, &FileContext::modificationChanged, Qt::UniqueConnection);
            if (mSyntaxHighlighter && mSyntaxHighlighter->document() != document())
                mSyntaxHighlighter->setDocAndConnect(document());
            QTimer::singleShot(50, this, &FileContext::updateMarks);
        }
    } else if (ptEdit) {
        ptEdit->setDocument(document());
    }
    // TODO(JM) getMouseMove and -click for editor to enable link-clicking
    if (ptEdit) {
        if (!ptEdit->viewport()->hasMouseTracking()) {
            ptEdit->viewport()->setMouseTracking(true);
        }
        ptEdit->viewport()->installEventFilter(this);
        ptEdit->installEventFilter(this);
    }
    if (scEdit) {
        connect(scEdit, &CodeEditor::requestMarkHash, &mMarks, &TextMarkList::shareMarkHash);
        connect(scEdit, &CodeEditor::requestMarksEmpty, &mMarks, &TextMarkList::textMarksEmpty);
    }
    setFlag(FileSystemContext::cfActive);
}

void FileContext::editToTop(QWidget* edit)
{
    addEditor(edit);
}

void FileContext::removeEditor(QWidget* edit)
{
    int i = mEditors.indexOf(edit);
    if (i < 0)
        return;
    bool wasModified = isModified();
    QPlainTextEdit* ptEdit = FileSystemContext::toPlainEdit(edit);
    if (ptEdit && mEditors.size() == 1) {
        // On removing last editor: paste document-parency back to editor
        ptEdit->document()->setParent(ptEdit);
        disconnect(ptEdit->document(), &QTextDocument::modificationChanged, this, &FileContext::modificationChanged);
        if (mSyntaxHighlighter && type() != FileSystemContext::Log) mSyntaxHighlighter->setDocAndConnect(nullptr);
    }
    mEditors.removeAt(i);
    if (mEditors.isEmpty()) {
        unsetFlag(FileSystemContext::cfActive);
        if (wasModified) emit changed(id());
    } else if (ptEdit) {
        ptEdit->setDocument(document()->clone(ptEdit));
    }
    if (ptEdit) {
        ptEdit->viewport()->removeEventFilter(this);
        ptEdit->removeEventFilter(this);
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

bool FileContext::hasEditor(QWidget* edit)
{
    return mEditors.contains(edit);
}

QTextDocument*FileContext::document()
{
    if (mEditors.isEmpty())
        return nullptr;
    QPlainTextEdit* ed = FileSystemContext::toPlainEdit(mEditors.first());
    return ed ? ed->document() : nullptr;
}

void FileContext::load(QString codecName)
{
//    TRACETIME();
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
                    break;
                }
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
        QTimer::singleShot(50, this, &FileContext::updateMarks);
    }
    if (!mWatcher) {
        mWatcher = new QFileSystemWatcher(this);
        connect(mWatcher, &QFileSystemWatcher::fileChanged, this, &FileContext::onFileChangedExtern);
        mWatcher->addPath(location());
    }
}

void FileContext::jumpTo(const QTextCursor &cursor, bool focus, int altLine, int altColumn)
{
    emit openFileContext(this, focus);
    if (mEditors.size()) {
        QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mEditors.first());
        if (!edit) return;
        QTextCursor tc(cursor.isNull() ? QTextCursor(edit->document()->findBlockByNumber(altLine)) : cursor);
        if (cursor.isNull()) tc.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, altColumn);
        tc.clearSelection();
        edit->setTextCursor(tc);
        // center line vertically
        int lines = edit->rect().bottom() / edit->cursorRect().height();
        int line = edit->cursorRect().bottom() / edit->cursorRect().height();
        int mv = line - lines/2;
        edit->verticalScrollBar()->setValue(edit->verticalScrollBar()->value()+mv);
    }
}

void FileContext::showToolTip(const QList<TextMark*> marks)
{
    if (mEditors.size() && marks.size() > 0 &&  !marks.first()->textCursor().isNull()) {
        QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mEditors.first());
        if (!edit) return;
        QTextCursor cursor(marks.first()->textCursor());
        cursor.setPosition(cursor.anchor());
        QPoint pos = edit->cursorRect(cursor).bottomLeft();
        QString tip = parentEntry()->lstErrorText(marks.first()->value());
        QToolTip::showText(edit->mapToGlobal(pos), tip, edit);
    }
}

void FileContext::rehighlightAt(int pos)
{
    if (document() && mSyntaxHighlighter) mSyntaxHighlighter->rehighlightBlock(document()->findBlock(pos));
}

void FileContext::updateMarks()
{
    // TODO(JM) Perform a large-file-test if this should have an own thread
    mMarks.updateMarks();
    if (mMarksEnhanced) return;
    QRegularExpression rex("\\*{4}((\\s+)\\$([0-9,]+)(.*)|\\s{1,3}([0-9]{1,3})\\s+(.*)|\\s\\s+(.*)|\\s(.*))");
    if (mMetrics.fileType() == FileType::Lst && document()) {
        for (TextMark* mark: mMarks.marks()) {
            QList<int> errNrs;
            int lineNr = mark->line();
            QTextBlock block = document()->findBlockByNumber(lineNr).next();
            QStringList errText;
            while (block.isValid()) {
                QRegularExpressionMatch match = rex.match(block.text());
                if (!match.hasMatch()) break;
                if (match.capturedLength(3)) { // first line with error numbers and indent
                    for (QString nrText: match.captured(3).split(",")) errNrs << nrText.toInt();
                    if (match.capturedLength(4)) errText << match.captured(4);
                } else if (match.capturedLength(5)) { // line with error number and description
                    errText << match.captured(5)+"\t"+match.captured(6);
                } else if (match.capturedLength(7)) { // indented follow-up line for error description
                    errText << "\t"+match.captured(7);
                } else if (match.capturedLength(8)) { // non-indented line for additional error description
                    errText << match.captured(8);
                }
                block = block.next();
            }
            parentEntry()->setLstErrorText(lineNr, errText.join("\n"));
        }
        mMarksEnhanced = true;
    }
}

TextMark* FileContext::generateTextMark(TextMark::Type tmType, int value, int line, int column, int size)
{
    TextMark* mark = mMarks.generateTextMark(this, tmType, value, line, column, size);
    return mark;
}

ErrorHighlighter *FileContext::highlighter()
{
    return mSyntaxHighlighter;
}

void FileContext::removeTextMarks(TextMark::Type tmType)
{
    removeTextMarks(QSet<TextMark::Type>() << tmType);
}

void FileContext::removeTextMarks(QSet<TextMark::Type> tmTypes)
{
    mMarks.removeTextMarks(tmTypes);
    if (mSyntaxHighlighter) mSyntaxHighlighter->rehighlight();
    for (QWidget* ed: mEditors) {
        ed->update(); // trigger delayed repaint
    }
}

bool FileContext::eventFilter(QObject* watched, QEvent* event)
{
    static QSet<QEvent::Type> evCheckMouse {QEvent::MouseButtonPress, QEvent::MouseButtonRelease, QEvent::MouseMove, QEvent::ToolTip};
    static QSet<QEvent::Type> evCheckKey {QEvent::KeyPress, QEvent::KeyRelease};

//    if (!mEditors.size() || (watched != mEditors.first() && watched != mEditors.first()->viewport()))
//        return FileSystemContext::eventFilter(watched, event);

    // For events MouseButtonPress, MouseButtonRelease, MouseMove,
    // - when send by viewport -> content
    // - when send by CodeEdit -> lineNumberArea
    // For event ToolTip
    // - always two events occur: one for viewport and one for CodeEdit

    // TODO(JM) use updateLinkDisplay

    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mEditors.first());
    if (!edit) FileSystemContext::eventFilter(watched, event);

    // TODO(JM) FileType of Log should be set to Log
    if (event->type() == QEvent::MouseButtonDblClick && mMetrics.fileType() == FileType::None /*FileType::Log*/) {
        QPoint pos = static_cast<QMouseEvent*>(event)->pos();
        int line = edit->cursorForPosition(pos).blockNumber();
        TextMark* linkMark = nullptr;
        for (TextMark *mark: mMarks.marks()) {
            if (mark->type() == TextMark::link && mark->refFileKind() == FileType::Lst) {
                if (mark->line() < line)
                    linkMark = mark;
                else if (!linkMark)
                    linkMark = mark;
                else if (qAbs(linkMark->line()-line) < qAbs(mark->line()-line))
                    break;
                else {
                    linkMark = mark;
                    break;
                }
            }
        }
        if (linkMark) linkMark->jumpToRefMark(true);

    } else if (evCheckKey.contains(event->type())) {

        QKeyEvent *keyEv = static_cast<QKeyEvent*>(event);
        if (keyEv->modifiers() & Qt::ControlModifier) {
            edit->viewport()->setCursor(mMarksAtMouse.isEmpty() ? Qt::IBeamCursor : Qt::PointingHandCursor);
        } else {
            edit->viewport()->setCursor(Qt::IBeamCursor);
        }
        return FileSystemContext::eventFilter(watched, event);

    } else if (evCheckMouse.contains(event->type())) {

        QPoint pos = (event->type() == QEvent::ToolTip) ? static_cast<QHelpEvent*>(event)->pos()
                                                        : static_cast<QMouseEvent*>(event)->pos();
        QTextCursor cursor = edit->cursorForPosition(pos);
        CodeEditor* codeEdit = FileSystemContext::toCodeEdit(edit);
        mMarksAtMouse = mMarks.findMarks(cursor);
        bool isValidLink = false;

        // if in CodeEditors lineNumberArea
        if (codeEdit && watched == codeEdit && event->type() != QEvent::ToolTip) {
            Qt::CursorShape shape = Qt::ArrowCursor;
            if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->cursorShape(&shape, true);
            codeEdit->lineNumberArea()->setCursor(shape);
            isValidLink = mMarksAtMouse.isEmpty() ? false : mMarksAtMouse.first()->isValidLink(true);
        } else {
            Qt::CursorShape shape = Qt::IBeamCursor;
            if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->cursorShape(&shape);
            edit->viewport()->setCursor(shape);
            isValidLink = mMarksAtMouse.isEmpty() ? false : mMarksAtMouse.first()->isValidLink();
        }

        if (!mMarksAtMouse.isEmpty() && event->type() == QEvent::MouseButtonPress) {
            mClickPos = pos;
        } else if (!mMarksAtMouse.isEmpty() && event->type() == QEvent::MouseButtonRelease) {
            if ((mClickPos-pos).manhattanLength() < 4 && isValidLink) {
                if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->jumpToRefMark();
                return !mMarksAtMouse.isEmpty();
            }
        } else if (event->type() == QEvent::ToolTip) {
            if (!mMarksAtMouse.isEmpty()) showToolTip(mMarksAtMouse);
            return !mMarksAtMouse.isEmpty();
        }
    }
    return FileSystemContext::eventFilter(watched, event);
}

bool FileContext::mouseOverLink()
{
    return !mMarksAtMouse.isEmpty();
}

void FileContext::modificationChanged(bool modiState)
{
    Q_UNUSED(modiState);
    emit changed(id());
}

void FileContext::onFileChangedExtern(QString filepath)
{
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
    mWatcher->addPath(location());
}

} // namespace studio
} // namespace gams
