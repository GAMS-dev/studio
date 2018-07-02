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
#include "filemeta.h"
#include "filemetarepo.h"
#include "projectrepo.h"
#include "filetype.h"
#include "editors/codeedit.h"
#include "exception.h"
#include "logger.h"
#include "studiosettings.h"
#include "commonpaths.h"
#include <QFileInfo>
#include <QFile>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>
#include <QScrollBar>

namespace gams {
namespace studio {

const QList<int> FileMeta::mDefaulsCodecs {0, 1, 108};
        // << "Utf-8" << "GB2312" << "Shift-JIS" << "System" << "Windows-1250" << "Latin-1";

FileMeta::FileMeta(FileMetaRepo *fileRepo, FileId id, QString location)
    : mId(id), mFileRepo(fileRepo), mLocation(location), mData(Data(location))
{
    if (!mFileRepo) EXCEPT() << "FileMetaRepo  must not be null";
    bool symbolic = mLocation.startsWith('[');
    int braceEnd = mLocation.indexOf(']');
    if (braceEnd <= 0) braceEnd = mLocation.size();
    mName = symbolic ? mLocation.left(braceEnd) : QFileInfo(mLocation).fileName();

    if (kind() != FileKind::Gdx) {
        mDocument = new QTextDocument(this);
        mDocument->setDocumentLayout(new QPlainTextDocumentLayout(mDocument));
        mDocument->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        connect(mDocument, &QTextDocument::modificationChanged, this, &FileMeta::modificationChanged);
    }

    if (kind() == FileKind::Gms || kind() == FileKind::Txt) {
        mHighlighter = new SyntaxHighlighter(id, mFileRepo->textMarkRepo());
        mHighlighter->setDocument(document());
    } else if (kind() != FileKind::Gdx) {
        mHighlighter = new ErrorHighlighter(id, mFileRepo->textMarkRepo());
        mHighlighter->setDocument(document());
    }

}

FileMeta::~FileMeta()
{
    mFileRepo->removedFile(this);
}

QVector<QPoint> FileMeta::getEditPositions()
{
    QVector<QPoint> res;
    foreach (QWidget* widget, mEditors) {
        AbstractEdit* edit = toAbstractEdit(widget);
        if (edit) {
            QTextCursor cursor = edit->textCursor();
            res << QPoint(cursor.positionInBlock(), cursor.blockNumber());
        }
    }
    return res;
}

void FileMeta::setEditPositions(QVector<QPoint> edPositions)
{
    int i = 0;
    foreach (QWidget* widget, mEditors) {
        AbstractEdit* edit = toAbstractEdit(widget);
        if (edit) {
            QPoint pos = (i < edPositions.size()) ? edPositions.at(i) : QPoint(0, 0);
            QTextCursor cursor(document());
            if (cursor.blockNumber() < pos.y())
                cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, qMin(edit->blockCount()-1, pos.y()));
            if (cursor.positionInBlock() < pos.x())
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, qMin(cursor.block().length()-1, pos.x()));
            edit->setTextCursor(cursor);
        }
        i++;
    }

}

void FileMeta::internalSave(const QString &location)
{
    QFile file(location);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        EXCEPT() << "Can't open the file";
    QTextStream out(&file);
    if (mCodec) out.setCodec(mCodec);
    out << document()->toPlainText();
    out.flush();
    file.close();
    mData = Data(location);
    document()->setModified(false);
}

FileId FileMeta::id() const
{
    return mId;
}

QString FileMeta::location() const
{
    return mLocation;
}

FileKind FileMeta::kind()
{
    return mData.type->kind();
}

QString FileMeta::name(NameModifier mod)
{
    switch (mod) {
    case NameModifier::editState:
        return mName + (isModified() ? "*" : "");
        break;
    default:
        break;
    }
    return mName;
}

QWidgetList FileMeta::editors() const
{
    return mEditors;
}

QWidget *FileMeta::topEditor() const
{
    return isOpen() ? mEditors.first() : nullptr;
}

void FileMeta::modificationChanged(bool modiState)
{
    Q_UNUSED(modiState);
    emit changed(id());
}

void FileMeta::updateMarks()
{
    // This gathers additional error information from lst-content

    // TODO(JM) Perform a large-file-test if this should have an own thread

//    if (!mMarks) return;
//    mMarks->updateMarks();
//    if (mMarksEnhanced) return;
//    QRegularExpression rex("\\*{4}((\\s+)\\$([0-9,]+)(.*)|\\s{1,3}([0-9]{1,3})\\s+(.*)|\\s\\s+(.*)|\\s(.*))");
//    if (mMetrics.fileType() == FileType::Lst && document()) {
//        for (TextMark* mark: mMarks->marks()) {
//            QList<int> errNrs;
//            int lineNr = mark->line();
//            QTextBlock block = document()->findBlockByNumber(lineNr).next();
//            QStringList errText;
//            while (block.isValid()) {
//                QRegularExpressionMatch match = rex.match(block.text());
//                if (!match.hasMatch()) break;
//                if (match.capturedLength(3)) { // first line with error numbers and indent
//                    for (QString nrText: match.captured(3).split(",")) errNrs << nrText.toInt();
//                    if (match.capturedLength(4)) errText << match.captured(4);
//                } else if (match.capturedLength(5)) { // line with error number and description
//                    errText << match.captured(5)+"\t"+match.captured(6);
//                } else if (match.capturedLength(7)) { // indented follow-up line for error description
//                    errText << "\t"+match.captured(7);
//                } else if (match.capturedLength(8)) { // non-indented line for additional error description
//                    errText << match.captured(8);
//                }
//                block = block.next();
//            }
//            parentEntry()->setLstErrorText(lineNr, errText.join("\n"));
//        }
//        mMarksEnhanced = true;
//    }

}

void FileMeta::addEditor(QWidget *edit)
{
    if (!edit) return;
    if (mEditors.contains(edit)) {
        mEditors.move(mEditors.indexOf(edit), 0);
        return;
    }
    if (editorType(edit) == EditorType::undefined)
        EXCEPT() << "Type assignment missing for this editor/viewer";

    mEditors.prepend(edit);
    AbstractEdit* ptEdit = toAbstractEdit(edit);
    CodeEdit* scEdit = toCodeEdit(edit);

    if (ptEdit) {
        ptEdit->setDocument(document());
        if (scEdit) {
            connect(scEdit, &CodeEdit::requestSyntaxState, mHighlighter, &ErrorHighlighter::syntaxState);
        }
        if (!ptEdit->viewport()->hasMouseTracking()) {
            ptEdit->viewport()->setMouseTracking(true);
        }
        ptEdit->viewport()->installEventFilter(this);
        ptEdit->installEventFilter(this);
    }
    if (mEditors.size() == 1) emit documentOpened();

//    if (scEdit && mMarks) {
//        // TODO(JM) Should be bound directly to a sublist in TextMarkRepo
//        connect(scEdit, &CodeEditor::requestMarkHash, this, &ProjectFileNode::shareMarkHash);
//        connect(scEdit, &CodeEditor::requestMarksEmpty, this, &ProjectFileNode::textMarkIconsEmpty);
//        connect(scEdit->document(), &QTextDocument::contentsChange, scEdit, &CodeEditor::afterContentsChanged);
//    }
}

void FileMeta::editToTop(QWidget *edit)
{
    addEditor(edit);
}

void FileMeta::removeEditor(QWidget *edit, bool suppressCloseSignal)
{
    int i = mEditors.indexOf(edit);
    if (i < 0) return;

    AbstractEdit* ptEdit = toAbstractEdit(edit);
    CodeEdit* scEdit = toCodeEdit(edit);
    mEditors.removeAt(i);

    if (ptEdit) {
        ptEdit->viewport()->removeEventFilter(this);
        ptEdit->removeEventFilter(this);
        ptEdit->setDocument(new QTextDocument(ptEdit));

        if (mEditors.isEmpty()) {
            if (!suppressCloseSignal) emit documentClosed();
            if (kind() != FileKind::Log) {
                mDocument->clear();
                mDocument->clearUndoRedoStacks();
                mDocument->setModified(false);
            }
        }
    }
    if (scEdit && mHighlighter) {
        disconnect(scEdit, &CodeEdit::requestSyntaxState, mHighlighter, &ErrorHighlighter::syntaxState);
    }
}

bool FileMeta::hasEditor(QWidget * const &edit) const
{
    return mEditors.contains(edit);
}

void FileMeta::load(int codecMib)
{
    load(codecMib==-1 ? QList<int>() : QList<int>() << codecMib);
}

void FileMeta::load(QList<int> codecMibs)
{
    // TODO(JM) Later, this method should be moved to the new DataWidget
    if (!document() && kind() != FileKind::Gdx)
        EXCEPT() << "There is no document assigned to the file " << location();
    QList<int> mibs = codecMibs.isEmpty() ? mDefaulsCodecs : codecMibs;

    QFile file(location());
    if (!file.fileName().isEmpty() && file.exists()) {
        if (!file.open(QFile::ReadOnly | QFile::Text))
            EXCEPT() << "Error opening file " << location();

        // TODO(JM) Read in lines to enable progress information
        // !! For paging: reading must ensure being at the start of a line - not in the middle of a unicode-character
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
//            if (mMarks && keepMarks)
//                disconnect(document(), &QTextDocument::contentsChange, mMarks, &TextMarkRepo::documentChanged);
            QVector<QPoint> edPos = getEditPositions();
            document()->setPlainText(text);
            setEditPositions(edPos);
//            if (mMarks && keepMarks)
//                connect(document(), &QTextDocument::contentsChange, mMarks, &TextMarkRepo::documentChanged);
            mCodec = codec;
        }
        file.close();
        mData = Data(location());
        document()->setModified(false);
//        QTimer::singleShot(50, this, &ProjectFileNode::updateMarks);
    }
}

void FileMeta::save()
{
    if (!isModified()) return;
    if (location().isEmpty() || location().startsWith('['))
        EXCEPT() << "Can't save file '" << location() << "'";
    internalSave(location());
}

void FileMeta::saveAs(const QString &location)
{
    if (kind() == FileKind::Log) {
        internalSave(location);
        return;
    }
    if (location.isEmpty() || location.startsWith('['))
        EXCEPT() << "Can't save file '" << location << "'";
    if (location == mLocation) return;

    // remember nodes that should be switched to the new FileMeta
    QVector<ProjectFileNode*> nodes;
    FileMeta* existingFM = mFileRepo->fileMeta(location);
    if (existingFM == this) existingFM = nullptr;
    if (existingFM)
        nodes = mFileRepo->projectRepo()->fileNodes(existingFM->id());

    // write the content
    mLocation = location;
    mData = Data(location);
    emit changed(mId);
    internalSave(mLocation);

    // if there were nodes on the existingFM assign them to this
    if (existingFM) {
        for (ProjectFileNode* node: nodes) {
            node->replaceFile(this);
        }
        for (QWidget* wid : existingFM->editors()) {
            existingFM->removeEditor(wid, true);
            addEditor(wid);
        }
        delete existingFM;
    }
}

void FileMeta::jumpTo(FileId runId, bool focus, int line, int column)
{
    emit mFileRepo->openFile(this, runId, focus, codecMib());

    AbstractEdit* edit = mEditors.size() ? toAbstractEdit(mEditors.first()) : nullptr;
    if (edit && edit->document()->blockCount()-1 < line) {
        QTextBlock block = edit->document()->findBlockByNumber(line);
        QTextCursor tc = QTextCursor(block);
        tc.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, qMin(column, block.length()-1));
        edit->setTextCursor(tc);
        // center line vertically
        qreal lines = qreal(edit->rect().height()) / edit->cursorRect().height();
        qreal line = qreal(edit->cursorRect().bottom()) / edit->cursorRect().height();
        int mv = line - lines/2;
        if (qAbs(mv) > lines/3)
            edit->verticalScrollBar()->setValue(edit->verticalScrollBar()->value()+mv);
    }
}

void FileMeta::rehighlight(int line)
{
    if (line < 0) return;
    if (document() && mHighlighter) mHighlighter->rehighlightBlock(document()->findBlockByNumber(line));
}

void FileMeta::rehighlightBlock(QTextBlock block, QTextBlock endBlock)
{
    if (!document() || !mHighlighter) return;
    while (block.isValid()) {
        mHighlighter->rehighlightBlock(block);
        if (!endBlock.isValid() || block == endBlock) break;
        block = block.next();
    }
}

ErrorHighlighter *FileMeta::highlighter() const
{
    return mHighlighter;
}

bool FileMeta::isModified() const
{
    return mDocument ? mDocument->isModified() : false;
}

bool FileMeta::isReadOnly() const
{
    AbstractEdit* edit = mEditors.isEmpty() ? nullptr : toAbstractEdit(mEditors.first());
    if (!edit) return true;
    return edit->isReadOnly();
}

bool FileMeta::isAutoReload() const
{
    return mData.type->autoReload();
}

QTextDocument *FileMeta::document() const
{
    return mDocument;
}

int FileMeta::codecMib() const
{
    return mCodec ? mCodec->mibEnum() : -1;
}

bool FileMeta::exists() const
{
    return mData.exist;
}

bool FileMeta::isOpen() const
{
    return !mEditors.isEmpty();
}

QWidget* FileMeta::createEdit(QTabWidget *tabWidget, ProjectRunGroupNode *runGroup, QList<int> codecMibs)
{
    Q_UNUSED(codecMibs)
    QWidget* res = nullptr;
    if (kind() != FileKind::Gdx) {
        CodeEdit *codeEdit = new CodeEdit(tabWidget);
        codeEdit->setSettings(mFileRepo->settings());
        codeEdit->setFileId(id());
        codeEdit->setTabChangesFocus(false);
        codeEdit->setGroupId(runGroup ? runGroup->id() : NodeId());
        initEditorType(codeEdit);
        codeEdit->setFont(QFont(mFileRepo->settings()->fontFamily(), mFileRepo->settings()->fontSize()));
        QFontMetrics metric(codeEdit->font());
        codeEdit->setTabStopDistance(8*metric.width(' '));
        res = codeEdit;
        if (kind() == FileKind::Lst) {
            lxiviewer::LxiViewer* lxiViewer = new lxiviewer::LxiViewer(codeEdit, this, runGroup, tabWidget);
            initEditorType(lxiViewer);
            res = lxiViewer;
        }

        // TODO(JM) load should be unbound from createEdit
//        if (!mEditors.size())
//            triggerLoad(codecMibs);

        if (kind() == FileKind::Log ||
                kind() == FileKind::Lst ||
                kind() == FileKind::Ref) {
            codeEdit->setReadOnly(true);
            codeEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        }
    } else {
        gdxviewer::GdxViewer* gdxView = new gdxviewer::GdxViewer(location(), CommonPaths::systemDir(), tabWidget);
        initEditorType(gdxView);
        res = gdxView;
    }
    addEditor(res);
    return res;
}

FileMeta::Data::Data(QString location)
{
    if (location.startsWith('[')) {
        int len = location.indexOf(']')-2;
        type = (len > 0) ? &FileType::from(location.mid(1, len)) : &FileType::from("");
    } else {
        QFileInfo fi(location);
        exist = fi.exists();
        size = fi.size();
        created = fi.birthTime();
        modified = fi.lastModified();
        type = &FileType::from(fi.suffix());
    }
}

} // namespace studio
} // namespace gams
