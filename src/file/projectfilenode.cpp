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
#include "projectfilenode.h"
#include "projectgroupnode.h"
#include "exception.h"
#include "editors/codeedit.h"
#include "logger.h"
#include <QScrollBar>
#include <QToolTip>
#include <QTextCodec>

namespace gams {
namespace studio {

const QList<int> ProjectFileNode::mDefaulsCodecs {0, 1, 108};
        // << "Utf-8" << "GB2312" << "Shift-JIS" << "System" << "Windows-1250" << "Latin-1";

ProjectFileNode::ProjectFileNode(FileId fileId, QString name, QString location, ContextType type)
    : ProjectAbstractNode(fileId, name, location, type)
{
    mMetrics = FileMetrics(QFileInfo(location));
    if (mMetrics.fileType() == FileType::Gms || mMetrics.fileType() == FileType::Txt)
        mSyntaxHighlighter = new SyntaxHighlighter(this);
    else if (mMetrics.fileType() != FileType::Gdx) {
        mSyntaxHighlighter = new ErrorHighlighter(this);
    }
}

QWidgetList& ProjectFileNode::editorList()
{
    return mEditors;
}

ProjectFileNode::~ProjectFileNode()
{
    if (mMarks) mMarks->unbind();

//    setParentEntry(nullptr);
    removeAllEditors();
}

void ProjectFileNode::setParentEntry(ProjectGroupNode* parent)
{
    ProjectAbstractNode::setParentEntry(parent);
    if (!parent) {
        if (mMarks) mMarks->unbind();
        mMarks = nullptr;
    } else if (mMarks != parent->marks(location())) {
        mMarks = parent->marks(location());
        if (mMarks) connect(this, &ProjectFileNode::documentOpened, mMarks, &TextMarkList::documentOpened);
    }
}

int ProjectFileNode::codecMib() const
{
    return mCodec ? mCodec->mibEnum() : -1;
}

void ProjectFileNode::setCodecMib(int mib)
{
    QTextCodec *codec = QTextCodec::codecForMib(mib);
    if (!codec)
        EXCEPT() << "TextCodec not found for MIB " << mib;
    if (document() && !isReadOnly() && !isModified() && codec != mCodec) {
        document()->setModified();
        mCodec = codec;
    }
    // TODO(JM) changing the codec must trigger conversion (not necessarily HERE)
}

const QString ProjectFileNode::caption()
{
    return name() + (isModified() ? "*" : "");
}

bool ProjectFileNode::isModified()
{
    return document() && document()->isModified();
}

void ProjectFileNode::save()
{
    if (isModified())
        save(location());
}

void ProjectFileNode::save(QString filePath)
{
    if (filePath == "")
        EXCEPT() << "Can't save without file name";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        EXCEPT() << "Can't open the file";
    mMetrics = FileMetrics();
    QTextStream out(&file);
    if (mCodec) out.setCodec(mCodec);
    out << document()->toPlainText();
    out.flush();
    file.close();
    mMetrics = FileMetrics(QFileInfo(file));
    document()->setModified(false);
}

const FileMetrics& ProjectFileNode::metrics() const
{
    return mMetrics;
}

const QWidgetList ProjectFileNode::editors() const
{
    return mEditors;
}

void ProjectFileNode::setLocation(const QString& _location)
{
    if (_location.isEmpty())
        EXCEPT() << "File can't be set to an empty location.";
    QFileInfo newLoc(_location);
    if (QFileInfo(location()) == newLoc)
        return; // nothing to do

    // TODO(JM) adapt parent group
    if (document())
        document()->setModified(true);
    ProjectAbstractNode::setLocation(_location);
    mMetrics = FileMetrics(newLoc);
}

QIcon ProjectFileNode::icon()
{
    QString runMark = (location() == parentEntry()->runnableGms()) ? "-run" : "";
    if (mMetrics.fileType() == FileType::Gms)
        return QIcon(":/img/gams-w"+runMark);
    if (mMetrics.fileType() == FileType::Gdx)
        return QIcon(":/img/database");
    return QIcon(":/img/file-alt"+runMark);
}

void ProjectFileNode::addEditor(QWidget* edit)
{
    if (!edit) return;
    if (mEditors.contains(edit)) {
        mEditors.move(mEditors.indexOf(edit), 0);
        return;
    }
    if (ProjectAbstractNode::editorType(edit) == ProjectAbstractNode::etUndefined)
        EXCEPT() << "Type assignment missing for this editor/viewer";
    bool newlyOpen = !document();
    mEditors.prepend(edit);
    AbstractEdit* ptEdit = ProjectFileNode::toAbstractEdit(edit);
    CodeEdit* scEdit = ProjectFileNode::toCodeEdit(edit);

    if (mEditors.size() == 1) {
        if (ptEdit) {
            ptEdit->document()->setParent(this);
            connect(document(), &QTextDocument::modificationChanged, this, &ProjectFileNode::modificationChanged, Qt::UniqueConnection);
            if (mSyntaxHighlighter && mSyntaxHighlighter->document() != document()) {
                mSyntaxHighlighter->setDocument(document());
                if (scEdit) connect(scEdit, &CodeEdit::requestSyntaxState, mSyntaxHighlighter, &ErrorHighlighter::syntaxState);
            }
            if (newlyOpen) emit documentOpened();
            QTimer::singleShot(50, this, &ProjectFileNode::updateMarks);
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
    if (scEdit && mMarks) {
        connect(scEdit, &CodeEdit::requestMarkHash, mMarks, &TextMarkList::shareMarkHash);
        connect(scEdit, &CodeEdit::requestMarksEmpty, mMarks, &TextMarkList::textMarkIconsEmpty);
        connect(scEdit->document(), &QTextDocument::contentsChange, scEdit, &CodeEdit::afterContentsChanged);
    }
    setFlag(ProjectAbstractNode::cfActive);
}

void ProjectFileNode::editToTop(QWidget* edit)
{
    addEditor(edit);
}

void ProjectFileNode::removeEditor(QWidget* edit)
{
    int i = mEditors.indexOf(edit);
    if (i < 0)
        return;
    bool wasModified = isModified();
    AbstractEdit* ptEdit = ProjectFileNode::toAbstractEdit(edit);
    CodeEdit* scEdit = ProjectFileNode::toCodeEdit(edit);

    if (ptEdit && mEditors.size() == 1) {
        // On removing last editor: paste document-parency back to editor
        ptEdit->document()->setParent(ptEdit);
        disconnect(ptEdit->document(), &QTextDocument::modificationChanged, this, &ProjectFileNode::modificationChanged);
    }
    mEditors.removeAt(i);
    if (mEditors.isEmpty()) {
        unsetFlag(ProjectAbstractNode::cfActive);
        if (wasModified) emit changed(id());
    } else if (ptEdit) {
        ptEdit->setDocument(document()->clone(ptEdit));
    }
    if (ptEdit) {
        ptEdit->viewport()->removeEventFilter(this);
        ptEdit->removeEventFilter(this);
    }
    if (scEdit && mSyntaxHighlighter) {
        disconnect(scEdit, &CodeEdit::requestSyntaxState, mSyntaxHighlighter, &ErrorHighlighter::syntaxState);
    }
}

void ProjectFileNode::removeAllEditors()
{
    auto editors = mEditors;
    for (auto editor : editors) {
        removeEditor(editor);
    }
    mEditors = editors;
}

bool ProjectFileNode::hasEditor(QWidget* edit)
{
    return mEditors.contains(edit);
}

QTextDocument*ProjectFileNode::document() const
{
    if (mEditors.isEmpty())
        return nullptr;
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mEditors.first());
    return edit ? edit->document() : nullptr;
}

bool ProjectFileNode::isReadOnly()
{
    AbstractEdit* edit = nullptr;
    if (mEditors.size()) {
        edit = toAbstractEdit(mEditors.first());
    }
    return edit && edit->isReadOnly();
}

void ProjectFileNode::load(QList<int> codecMibs, bool keepMarks)
{
//    TRACETIME();
    if (!document())
        EXCEPT() << "There is no document assigned to the file " << location();

    QList<int> mibs = codecMibs.isEmpty() ? mDefaulsCodecs : codecMibs;

    QFile file(location());
    if (!file.fileName().isEmpty() && file.exists()) {
        if (!file.open(QFile::ReadOnly | QFile::Text))
            EXCEPT() << "Error opening file " << location();
        mMetrics = FileMetrics();
        const QByteArray data(file.readAll());
        QString text;
        QTextCodec *codec = nullptr;
        for (int mib: mibs) {
            QTextCodec::ConverterState state;
            codec = QTextCodec::codecForMib(mib);
            if (codec) {
                text = codec->toUnicode(data.constData(), data.size(), &state);
                if (state.invalidChars == 0) {
                    break;
                }
            } else {
                DEB() << "System doesn't contain codec for MIB " << mib;
            }
        }
        if (codec) {
            if (mMarks && keepMarks)
                disconnect(document(), &QTextDocument::contentsChange, mMarks, &TextMarkList::documentChanged);
            QVector<QPoint> edPos = getEditPositions();
            document()->setPlainText(text);
            setEditPositions(edPos);
            if (mMarks && keepMarks)
                connect(document(), &QTextDocument::contentsChange, mMarks, &TextMarkList::documentChanged);
            mCodec = codec;
        }
        file.close();
        document()->setModified(false);
        mMetrics = FileMetrics(QFileInfo(file));
        QTimer::singleShot(50, this, &ProjectFileNode::updateMarks);
    }
    if (!mWatcher) {
        mWatcher = new QFileSystemWatcher(this);
        connect(mWatcher, &QFileSystemWatcher::fileChanged, this, &ProjectFileNode::onFileChangedExtern);
        mWatcher->addPath(location());
    }
}

void ProjectFileNode::load(int codecMib, bool keepMarks)
{
    load(QList<int>() << codecMib, keepMarks);
}

void ProjectFileNode::jumpTo(const QTextCursor &cursor, int altLine, int altColumn)
{
    emit openFileNode(this, true);
    if (!mEditors.size())
        return;
    CodeEdit* edit = ProjectAbstractNode::toCodeEdit(mEditors.first());
    if (edit)
        edit->jumpTo(cursor, altLine, altColumn);
}

void ProjectFileNode::showToolTip(const QVector<TextMark*> marks)
{
    if (mEditors.size() && marks.size() > 0) {
        QTextCursor cursor(marks.first()->textCursor());
        if (cursor.isNull()) return;
        AbstractEdit* edit = ProjectAbstractNode::toAbstractEdit(mEditors.first());
        if (!edit) return;
        cursor.setPosition(cursor.anchor());
        QPoint pos = edit->cursorRect(cursor).bottomLeft();
        QString tip = parentEntry()->lstErrorText(marks.first()->value());
        QToolTip::showText(edit->mapToGlobal(pos), tip, edit);
    }
}

void ProjectFileNode::rehighlightAt(int pos)
{
    if (pos < 0) return;
    if (document() && mSyntaxHighlighter) mSyntaxHighlighter->rehighlightBlock(document()->findBlock(pos));
}

void ProjectFileNode::rehighlightBlock(QTextBlock block, QTextBlock endBlock)
{
    if (!document() || !mSyntaxHighlighter) return;
    while (block.isValid()) {
        mSyntaxHighlighter->rehighlightBlock(block);
        if (!endBlock.isValid() || block == endBlock) break;
        block = block.next();
    }
}

void ProjectFileNode::updateMarks()
{
    // TODO(JM) Perform a large-file-test if this should have an own thread
    if (!mMarks) return;
    mMarks->updateMarks();
    if (mMarksEnhanced) return;
    QRegularExpression rex("\\*{4}((\\s+)\\$([0-9,]+)(.*)|\\s{1,3}([0-9]{1,3})\\s+(.*)|\\s\\s+(.*)|\\s(.*))");
    if (mMetrics.fileType() == FileType::Lst && document()) {
        for (TextMark* mark: mMarks->marks()) {
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

TextMark* ProjectFileNode::generateTextMark(TextMark::Type tmType, int value, int line, int column, int size)
{
    if (!mMarks || !parentEntry())
        EXCEPT() << "Marks can only be set if FileContext is linked to a Group";
    TextMark* mark = mMarks->generateTextMark(tmType, value, line, column, size);
    return mark;
}

TextMark*ProjectFileNode::generateTextMark(QString fileName, TextMark::Type tmType, int value, int line, int column, int size)
{
    if (!parentEntry())
        EXCEPT() << "Marks can only be set if FileContext is linked to a Group";
    TextMark* mark = parentEntry()->marks(fileName)->generateTextMark(tmType, value, line, column, size);
    return mark;
}

ErrorHighlighter *ProjectFileNode::highlighter()
{
    return mSyntaxHighlighter;
}

void ProjectFileNode::removeTextMarks(TextMark::Type tmType, bool rehighlight)
{
    removeTextMarks(QSet<TextMark::Type>() << tmType, rehighlight);
}

void ProjectFileNode::removeTextMarks(QSet<TextMark::Type> tmTypes, bool rehighlight)
{
    if (!mMarks) return;
    mMarks->removeTextMarks(tmTypes, rehighlight);
}

void ProjectFileNode::addFileWatcherForGdx()
{
    if (!mWatcher) {
        mWatcher = new QFileSystemWatcher(this);
        connect(mWatcher, &QFileSystemWatcher::fileChanged, this, &ProjectFileNode::onFileChangedExtern);
        mWatcher->addPath(location());
    }
}

void ProjectFileNode::unwatch()
{
    if (mWatcher) {
        mWatcher->removePath(location());
        mWatcher->deleteLater();
        mWatcher = nullptr;
    }
}

QString ProjectFileNode::tooltip()
{
    return location();
}

int ProjectFileNode::textMarkCount(QSet<TextMark::Type> tmTypes)
{
    return mMarks->textMarkCount(tmTypes);
}

bool ProjectFileNode::eventFilter(QObject* watched, QEvent* event)
{
    static QSet<QEvent::Type> evCheckMouse {QEvent::MouseButtonDblClick, QEvent::MouseButtonPress, QEvent::MouseButtonRelease, QEvent::MouseMove};
    static QSet<QEvent::Type> evCheckKey {QEvent::KeyPress, QEvent::KeyRelease};

//    if (!mEditors.size() || (watched != mEditors.first() && watched != mEditors.first()->viewport()))
//        return FileSystemContext::eventFilter(watched, event);

    // For events MouseButtonPress, MouseButtonRelease, MouseMove,
    // - when send by viewport -> content
    // - when send by CodeEdit -> lineNumberArea
    // For event ToolTip
    // - always two events occur: one for viewport and one for CodeEdit

    // TODO(JM) use updateLinkDisplay

    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mEditors.first());
    if (!edit) ProjectAbstractNode::eventFilter(watched, event);

    QMouseEvent* mouseEvent = (evCheckMouse.contains(event->type())) ? static_cast<QMouseEvent*>(event) : nullptr;
    QHelpEvent* helpEvent = (event->type() == QEvent::ToolTip)  ? static_cast<QHelpEvent*>(event) : nullptr;
    QKeyEvent *keyEvent = (evCheckKey.contains(event->type())) ? static_cast<QKeyEvent*>(event) : nullptr;

    // TODO(JM) FileType of Log should be set to Log
    if (mMetrics.fileType() == FileType::Log
        && (event->type() == QEvent::MouseButtonDblClick
            || (event->type() == QEvent::MouseButtonRelease && mouseEvent->modifiers()==Qt::ControlModifier)) ) {
        QPoint pos = mouseEvent->pos();
        QTextCursor cursor = edit->cursorForPosition(pos);
        if (mMarks && (mMarks->marksForBlock(cursor.block(), TextMark::error).isEmpty()
                       || mouseEvent->modifiers()==Qt::ControlModifier)) {
            int line = cursor.blockNumber();
            TextMark* linkMark = nullptr;
            for (TextMark *mark: mMarks->marks()) {
                if (mark->type() == TextMark::link && mark->refFileKind() == FileType::Lst) {
                    if (mark->line() < line)
                        linkMark = mark;
                    else if (!linkMark)
                        linkMark = mark;
                    else if (line+1 < mark->line()+mark->spread())
                        break;
                    else if (qAbs(linkMark->line()-line) < qAbs(line-mark->line()))
                        break;
                    else {
                        linkMark = mark;
                        break;
                    }
                }
            }
            if (linkMark) linkMark->jumpToRefMark(true);
        }

    } else if (keyEvent) {
        if (keyEvent->modifiers() & Qt::ControlModifier) {
            edit->viewport()->setCursor(mMarksAtMouse.isEmpty() ? Qt::IBeamCursor : Qt::PointingHandCursor);
        } else {
            edit->viewport()->setCursor(Qt::IBeamCursor);
        }
        return ProjectAbstractNode::eventFilter(watched, event);

    } else if (mouseEvent || helpEvent) {
        static QPoint ttPos;

        QPoint pos = mouseEvent ? mouseEvent->pos() : helpEvent->pos();
        QTextCursor cursor = edit->cursorForPosition(pos);
        CodeEdit* codeEdit = ProjectAbstractNode::toCodeEdit(edit);
        mMarksAtMouse = mMarks ? mMarks->findMarks(cursor) : QVector<TextMark*>();
        bool isValidLink = false;
        if (QToolTip::isVisible() && (ttPos-pos).manhattanLength() > 3) {
            QToolTip::hideText();
            ttPos = QPoint();
        }

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
            ttPos = pos;
            return !mMarksAtMouse.isEmpty();
        }
    }
    return ProjectAbstractNode::eventFilter(watched, event);
}

bool ProjectFileNode::mouseOverLink()
{
    return !mMarksAtMouse.isEmpty();
}

QVector<QPoint> ProjectFileNode::getEditPositions()
{
    QVector<QPoint> res;
    foreach (QWidget* widget, mEditors) {
        AbstractEdit* edit = ProjectFileNode::toAbstractEdit(widget);
        if (edit) {
            QTextCursor cursor = edit->textCursor();
            res << QPoint(cursor.positionInBlock(), cursor.blockNumber());
        } else {
            res << QPoint(0, 0);
        }
    }
    return res;
}

void ProjectFileNode::setEditPositions(QVector<QPoint> edPositions)
{
    int i = 0;
    foreach (QWidget* widget, mEditors) {
        AbstractEdit* edit = ProjectFileNode::toAbstractEdit(widget);
        QPoint pos = (i < edPositions.size()) ? edPositions.at(i) : QPoint(0, 0);
        if (edit) {
            QTextCursor cursor(edit->document());
            if (cursor.blockNumber() < pos.y())
                cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, qMin(edit->blockCount()-1, pos.y()));
            if (cursor.positionInBlock() < pos.x())
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, qMin(cursor.block().length()-1, pos.x()));
            edit->setTextCursor(cursor);
        }

    }
}

void ProjectFileNode::modificationChanged(bool modiState)
{
    Q_UNUSED(modiState);
    emit changed(id());
}

void ProjectFileNode::onFileChangedExtern(QString filepath)
{
    if (!mEditors.size())
        return;
    QFileInfo fi(filepath);

    gdxviewer::GdxViewer* gdxViewer = toGdxViewer(mEditors.first());
    // we have a GDX Viewer
    if (gdxViewer) {

        if (!fi.exists()) {
            // file has been renamed or deleted
            //TODO: implement
            this->removeEditor(gdxViewer);
        } else {
            // file changed externally
            gdxViewer->setHasChanged(true);
        }
    }
    // we have a normal document
    else {
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
    mWatcher->addPath(location());
}

} // namespace studio
} // namespace gams
