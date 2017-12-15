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

enum MouseOverLinkType {
    molNone = 0,
    molErrIcon = TextMark::error,
    molLinkIcon = TextMark::link,
    molBookmarkIcon = TextMark::bookmark,
    molText = TextMark::all+1,
};

FileContext::FileContext(int id, QString name, QString location, ContextType type)
    : FileSystemContext(id, name, location, type)
{
    mMetrics = FileMetrics(QFileInfo(location));
    if (mMetrics.fileType() == FileType::Gms || mMetrics.fileType() == FileType::Txt)
        mSyntaxHighlighter = new SyntaxHighlighter(this, &mMarks);
    else if (mMetrics.fileType() != FileType::Gdx) {
        mSyntaxHighlighter = new ErrorHighlighter(this, &mMarks);
    }
}

QList<QPlainTextEdit*>&FileContext::editorList()
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
    if (!edit)
        return;
    if (mEditors.contains(edit)) {
        mEditors.move(mEditors.indexOf(edit), 0);
        return;
    }

    mEditors.prepend(edit);
    if (mEditors.size() == 1) {
        document()->setParent(this);
        connect(document(), &QTextDocument::modificationChanged, this, &FileContext::modificationChanged, Qt::UniqueConnection);
        if (mSyntaxHighlighter && mSyntaxHighlighter->document() != document())
            mSyntaxHighlighter->setDocAndConnect(document());
        QTimer::singleShot(50, &mMarks, &TextMarkList::updateMarks);
    } else {
        edit->setDocument(document());
    }
    // TODO(JM) getMouseMove and -click for editor to enable link-clicking
    if (!edit->viewport()->hasMouseTracking()) {
        edit->viewport()->setMouseTracking(true);
    }
    edit->viewport()->installEventFilter(this);
    edit->installEventFilter(this);

    setFlag(FileSystemContext::cfActive);
}

void FileContext::addEditor(CodeEditor* edit)
{
    addEditor(static_cast<QPlainTextEdit*>(edit));
    connect(edit, &CodeEditor::requestMarkHash, &mMarks, &TextMarkList::shareMarkHash);
    connect(edit, &CodeEditor::requestMarksEmpty, &mMarks, &TextMarkList::textMarksEmpty);
}

void FileContext::editToTop(QPlainTextEdit* edit)
{
    addEditor(edit);
}

void FileContext::removeEditor(QPlainTextEdit* edit)
{
    int i = mEditors.indexOf(edit);
    if (i < 0)
        return;
    bool wasModified = isModified();
    mEditors.removeAt(i);
    if (mEditors.isEmpty()) {
        if (mSyntaxHighlighter && !document()) {
            mSyntaxHighlighter->setDocAndConnect(nullptr);
        }
        // After removing last editor: paste document-parency back to editor
        edit->document()->setParent(edit);
        disconnect(edit->document(), &QTextDocument::modificationChanged, this, &FileContext::modificationChanged);
        unsetFlag(FileSystemContext::cfActive);
        if (wasModified) emit changed(id());
    } else {
        edit->setDocument(document()->clone(edit));
    }
    edit->viewport()->removeEventFilter(this);
    edit->removeEventFilter(this);
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
    if (mEditors.isEmpty())
        return nullptr;
    return mEditors.first()->document();
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
//        QTimer::singleShot(50, &mMarks, &TextMarkList::updateMarks);
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
        QPlainTextEdit* edit = mEditors.first();
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

void FileContext::showToolTip(const TextMark& mark)
{
    if (mEditors.size() && !mark.textCursor().isNull()) {
        QPlainTextEdit* edit = mEditors.first();
        QTextCursor cursor(mark.textCursor());
        cursor.setPosition(cursor.anchor());
        QPoint pos = edit->cursorRect(cursor).bottomLeft();
        QString tip;
        emit requestErrorHint(mark.value(), tip);

        QToolTip::showText(edit->mapToGlobal(pos), tip, edit);
    }
}

void FileContext::rehighlightAt(int pos)
{
    if (document() && mSyntaxHighlighter) mSyntaxHighlighter->rehighlightBlock(document()->findBlock(pos));
}

void FileContext::updateMarks()
{
    mMarks.updateMarks();
}

TextMark* FileContext::generateTextMark(TextMark::Type tmType, int value, int line, int column, int size)
{
    TextMark* mark = mMarks.generateTextMark(this, tmType, value, line, column, size);
    return mark;
}

void FileContext::removeTextMarks(TextMark::Type tmType)
{
    removeTextMarks(QSet<TextMark::Type>() << tmType);
}

void FileContext::removeTextMarks(QSet<TextMark::Type> tmTypes)
{
    mMarks.removeTextMarks(tmTypes);
    if (mSyntaxHighlighter) mSyntaxHighlighter->rehighlight();
    for (QPlainTextEdit* ed: mEditors) {
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


    if (evCheckKey.contains(event->type())) {
        QKeyEvent *keyEv = static_cast<QKeyEvent*>(event);
        if (keyEv->modifiers() & Qt::ControlModifier) {
            mEditors.first()->viewport()->setCursor(mMarksAtMouse.isEmpty() ? Qt::IBeamCursor : Qt::PointingHandCursor);
        } else {
            mEditors.first()->viewport()->setCursor(Qt::IBeamCursor);
        }
        return FileSystemContext::eventFilter(watched, event);
    }

    if (evCheckMouse.contains(event->type())) {
        QPlainTextEdit* edit = mEditors.first();
        QPoint pos = (event->type() == QEvent::ToolTip) ? static_cast<QHelpEvent*>(event)->pos()
                                                        : static_cast<QMouseEvent*>(event)->pos();
        QTextCursor cursor = edit->cursorForPosition(pos);
        CodeEditor* codeEdit = dynamic_cast<CodeEditor*>(edit);
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
            if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->cursorShape(&shape, true);
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
            if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->showToolTip();
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
