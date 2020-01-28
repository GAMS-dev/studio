/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "settingslocator.h"
#include "studiosettings.h"
#include "commonpaths.h"
#include "editors/viewhelper.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "support/solverconfiginfo.h"

#include <QTabWidget>
#include <QFileInfo>
#include <QFile>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>
#include <QScrollBar>

namespace gams {
namespace studio {

FileMeta::FileMeta(FileMetaRepo *fileRepo, FileId id, QString location, FileType *knownType)
    : mId(id), mFileRepo(fileRepo), mData(Data(location, knownType))
{
    if (!mFileRepo) EXCEPT() << "FileMetaRepo  must not be null";
    mCodec = QTextCodec::codecForLocale();
    if (location.contains('\\'))
        location = QDir::fromNativeSeparators(location);
    setLocation(location);
    mTempAutoReloadTimer.setSingleShot(true); // only for measurement
    mReloadTimer.setSingleShot(true);
    connect(&mReloadTimer, &QTimer::timeout, this, &FileMeta::reload);
    mDirtyLinesUpdater.setSingleShot(true);
    connect(&mDirtyLinesUpdater, &QTimer::timeout, this, &FileMeta::updateMarks);
}

void FileMeta::setLocation(QString location)
{
    if (location.contains('\\'))
        location = QDir::fromNativeSeparators(location);
    if (mLocation != location) {
        QString oldLocation = mLocation;
        mFileRepo->unwatch(this);
        mLocation = location;
        mFileRepo->updateRenamed(this, oldLocation);
        mFileRepo->watch(this);
        mName = mData.type->kind() == FileKind::Log ? '['+QFileInfo(mLocation).completeBaseName()+']'
                                                    : QFileInfo(mLocation).fileName();
        for (QWidget*wid: mEditors) {
            ViewHelper::setLocation(wid, location);
            ViewHelper::setFileId(wid, id());
        }
    }
}

void FileMeta::takeEditsFrom(FileMeta *other)
{
    if (mDocument) return;
    mEditors = other->mEditors;
    mDocument = other->mDocument;
    other->mDocument = nullptr;
    other->mEditors.clear();
    for (QWidget *wid: mEditors) {
        ViewHelper::setLocation(wid, location());
        ViewHelper::setFileId(wid, id());
    }
}

FileMeta::~FileMeta()
{
    if (mDocument) unlinkAndFreeDocument();
    mFileRepo->textMarkRepo()->removeMarks(id());
    mFileRepo->removeFile(this);
}

QVector<QPoint> FileMeta::getEditPositions()
{
    QVector<QPoint> res;
    for (QWidget* widget: mEditors) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(widget);
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
    for (QWidget* widget: mEditors) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(widget);
        if (edit) {
            QPoint pos = (i < edPositions.size()) ? edPositions.at(i) : QPoint(0, 0);
            QTextCursor cursor(document()->findBlockByNumber(qMin(pos.y(), document()->blockCount()-1)));
            if (cursor.positionInBlock() < pos.x())
                cursor.setPosition(cursor.position() + qMin(cursor.block().length()-1, pos.x()));
            edit->setTextCursor(cursor);
        }
        i++;
    }
}

bool FileMeta::checkActivelySavedAndReset()
{
    bool res = mActivelySaved;
    mActivelySaved = false;
    return res;
}

void FileMeta::linkDocument(QTextDocument *doc)
{
    // The very first editor opened for this FileMeta should pass it's document here. It takes over parency
    if (kind() != FileKind::Gdx) {
        mDocument = doc;
        doc->setParent(this);
        mDocument->setDocumentLayout(new QPlainTextDocumentLayout(mDocument));
        mDocument->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        connect(mDocument, &QTextDocument::modificationChanged, this, &FileMeta::modificationChanged);
    }

    if (kind() == FileKind::Gms) {
        mHighlighter = new syntax::SyntaxHighlighter(mDocument);
        connect(mDocument, &QTextDocument::contentsChange, this, &FileMeta::contentsChange);
        connect(mDocument, &QTextDocument::blockCountChanged, this, &FileMeta::blockCountChanged);
    }
}

void FileMeta::unlinkAndFreeDocument()
{
    if (!mDocument) return;
    disconnect(mDocument, &QTextDocument::modificationChanged, this, &FileMeta::modificationChanged);
    if (kind() == FileKind::Gms) {
        disconnect(mDocument, &QTextDocument::contentsChange, this, &FileMeta::contentsChange);
        disconnect(mDocument, &QTextDocument::blockCountChanged, this, &FileMeta::blockCountChanged);
    }

    if (mHighlighter) {
        mHighlighter->setDocument(nullptr);
        mHighlighter->deleteLater();
        mHighlighter = nullptr;
    }
    mDocument->deleteLater();
    mDocument = nullptr;
}

FileId FileMeta::id() const
{
    return mId;
}

QString FileMeta::location() const
{
    return mLocation;
}

QStringList FileMeta::suffix() const
{
    return mData.type->suffix();
}

void FileMeta::setKind(const QString &suffix)
{
    mData.type = &FileType::from(suffix);
}

FileKind FileMeta::kind() const
{
    return mData.type->kind();
}

QString FileMeta::kindAsStr() const
{
    return mData.type->suffix().first();
}

QString FileMeta::name(NameModifier mod)
{
    switch (mod) {
    case NameModifier::editState:
        return mName + (isModified() ? "*" : "");
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

void FileMeta::contentsChange(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsRemoved);
    if (!mDocument) return;
    if (!isOpen()) return;
    if (mLoading) return;
    AbstractEdit *edit = ViewHelper::toAbstractEdit(topEditor());
    if (!edit) return;
    QTextCursor cursor(mDocument);
    cursor.setPosition(from);
    int column = cursor.positionInBlock();
    int fromLine = cursor.blockNumber();
    cursor.setPosition(from+charsAdded);
    int toLine = cursor.blockNumber();
    int removedLines = mLineCount-mDocument->lineCount() + toLine-fromLine;
    mChangedLine = fromLine;
//    if (charsAdded) --mChangedLine;
//    if (!column) --mChangedLine;
    if (removedLines > 0)
        mFileRepo->textMarkRepo()->removeMarks(id(), edit->groupId(), QSet<TextMark::Type>()
                                               , fromLine, fromLine+removedLines);
    for (int i = fromLine; i <= toLine; ++i) {
        QList<TextMark*> marks = mFileRepo->textMarkRepo()->marks(id(), i, edit->groupId());
        for (TextMark *mark: marks) {
            if (mark->blockEnd() >= column)
                mark->flatten();
        }
    }
}

void FileMeta::blockCountChanged(int newBlockCount)
{
    if (mLineCount != newBlockCount) {
        mFileRepo->textMarkRepo()->shiftMarks(id(), mChangedLine, newBlockCount-mLineCount);
        mLineCount = newBlockCount;
    }
}

void FileMeta::updateMarks()
{
    QMutexLocker mx(&mDirtyLinesMutex);

    // update changed editors
    for (QWidget *w: mEditors) {
        if (AbstractEdit * ed = ViewHelper::toAbstractEdit(w))
            ed->marksChanged(mDirtyLines);
        if (TextView * tv = ViewHelper::toTextView(w))
            tv->marksChanged(mDirtyLines);
    }
    mDirtyLines.clear();
}

void FileMeta::reload()
{
    load(mCodec->mibEnum(), false);
}

void FileMeta::invalidate()
{
    for (QWidget *wid: mEditors) {
        if (TextView* tv = ViewHelper::toTextView(wid)) {
            tv->invalidate();
        }
    }
}

void FileMeta::invalidateScheme()
{
    if (mHighlighter) {
        mHighlighter->reloadColors();
        for (QWidget *w: mEditors) {
            if (CodeEdit *ce = ViewHelper::toCodeEdit(w)) {
                mHighlighter->rehighlight();
                ce->updateExtraSelections();
                ce->lineNumberArea()->repaint();
            }
        }
    }
}

void FileMeta::addEditor(QWidget *edit)
{
    if (!edit) return;
    if (mEditors.contains(edit)) {
        mEditors.move(mEditors.indexOf(edit), 0);
        return;
    }
    if (ViewHelper::editorType(edit) == EditorType::undefined)
        EXCEPT() << "Type assignment missing for this editor/viewer";

    mEditors.prepend(edit);
    ViewHelper::setLocation(edit, location());
    ViewHelper::setFileId(edit, id());
    AbstractEdit* aEdit = ViewHelper::toAbstractEdit(edit);
    option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(edit);

    if (aEdit) {
        if (!mDocument)
            linkDocument(aEdit->document());
        else
            aEdit->setDocument(mDocument);
        connect(aEdit, &AbstractEdit::requestLstTexts, mFileRepo->projectRepo(), &ProjectRepo::errorTexts);
        connect(aEdit, &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        connect(aEdit, &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);

        CodeEdit* scEdit = ViewHelper::toCodeEdit(edit);
        if (scEdit && mHighlighter)
            connect(scEdit, &CodeEdit::requestSyntaxKind, mHighlighter, &syntax::SyntaxHighlighter::syntaxKind);

        if (!aEdit->viewport()->hasMouseTracking())
            aEdit->viewport()->setMouseTracking(true);

    }
    if (TextView* tv = ViewHelper::toTextView(edit)) {
        connect(tv->edit(), &AbstractEdit::requestLstTexts, mFileRepo->projectRepo(), &ProjectRepo::errorTexts);
        connect(tv->edit(), &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        connect(tv->edit(), &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
        if (tv->kind() == TextView::FileText)
            tv->setMarks(mFileRepo->textMarkRepo()->marks(mId));
    }
    if (soEdit) {
        connect(soEdit, &option::SolverOptionWidget::modificationChanged, this, &FileMeta::modificationChanged);
    }
    if (mEditors.size() == 1) emit documentOpened();
    if (aEdit)
        aEdit->setMarks(mFileRepo->textMarkRepo()->marks(mId));
}

void FileMeta::editToTop(QWidget *edit)
{
    addEditor(edit);
}

void FileMeta::removeEditor(QWidget *edit)
{
    int i = mEditors.indexOf(edit);
    if (i < 0) return;

    AbstractEdit* aEdit = ViewHelper::toAbstractEdit(edit);
    CodeEdit* scEdit = ViewHelper::toCodeEdit(edit);
    option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(edit);
    mEditors.removeAt(i);

    if (aEdit) {
        aEdit->setMarks(nullptr);
        aEdit->disconnectTimers();
        QTextDocument *doc = new QTextDocument(aEdit);
        doc->setDocumentLayout(new QPlainTextDocumentLayout(doc)); // w/o layout the setDocument() fails
        aEdit->setDocument(doc);

        if (mEditors.isEmpty()) {
            emit documentClosed();
            if (kind() != FileKind::Log) {
                unlinkAndFreeDocument();
            }
        }
        disconnect(aEdit, &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        disconnect(aEdit, &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
    }
    if (TextView* tv = ViewHelper::toTextView(edit)) {
        tv->edit()->disconnectTimers();
        disconnect(tv->edit(), &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        disconnect(tv->edit(), &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
    }
    if (soEdit) {
       disconnect(soEdit, &option::SolverOptionWidget::modificationChanged, this, &FileMeta::modificationChanged);
    }

    if (mEditors.isEmpty()) {
        mFileRepo->textMarkRepo()->removeMarks(id(), QSet<TextMark::Type>() << TextMark::bookmark);
    }
    if (scEdit && mHighlighter) {
        disconnect(scEdit, &CodeEdit::requestSyntaxKind, mHighlighter, &syntax::SyntaxHighlighter::syntaxKind);
    }
}

bool FileMeta::hasEditor(QWidget * const &edit) const
{
    return mEditors.contains(edit);
}

void FileMeta::load(int codecMib, bool init)
{
    if (codecMib == -1) codecMib = QTextCodec::codecForLocale()->mibEnum();
    mData = Data(location(), mData.type);

    if (kind() == FileKind::Gdx) {
        for (QWidget *wid: mEditors) {
            if (gdxviewer::GdxViewer *gdxViewer = ViewHelper::toGdxViewer(wid)) {
                mCodec = QTextCodec::codecForMib(codecMib);
                gdxViewer->reload(mCodec);
            }
        }
        return;
    }
    if (kind() == FileKind::TxtRO || kind() == FileKind::Lst) {
        for (QWidget *wid: mEditors) {
            if (TextView *tView = ViewHelper::toTextView(wid))
                tView->loadFile(location(), codecMib, init);
            if (kind() == FileKind::Lst) {
                lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(wid);
                if (lxi) lxi->loadLxi();
            }
        }
        return;
    }
    if (kind() == FileKind::Ref) {
        for (QWidget *wid: mEditors) {
            reference::ReferenceViewer *refViewer = ViewHelper::toReferenceViewer(wid);
            mCodec = QTextCodec::codecForMib(codecMib);
            if (refViewer) refViewer->on_referenceFileChanged(mCodec);
        }
        return;
    }
    if (kind() == FileKind::Opt) {
        bool textOptEditor = true;
        for (QWidget *wid : mEditors) {
            option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(wid);
            if (so) {
                textOptEditor = false;
                mCodec = QTextCodec::codecForMib(codecMib);
                so->on_reloadSolverOptionFile(mCodec);
            }
        }
        if (!textOptEditor)
            return;
    }

    QFile file(location());
    bool canOpen = true;
    emit editableFileSizeCheck(file, canOpen);
    if (!canOpen)
        EXCEPT() << "FileMeta" << '\t' << "Size for editable files exceeded: " << file.fileName();

    if (!mDocument) {
        QTextDocument *doc = new QTextDocument(this);
        linkDocument(doc);
    }
    if (!file.fileName().isEmpty() && file.exists()) {
        if (!file.open(QFile::ReadOnly | QFile::Text))
            EXCEPT() << "Error opening file " << location();

        const QByteArray data(file.readAll());
        QTextCodec *codec = nullptr;
        QString invalidCodecs;
        QTextCodec::ConverterState state;
        codec = QTextCodec::codecForMib(codecMib);
        if (codec) {
            QString text = codec->toUnicode(data.constData(), data.size(), &state);
            if (state.invalidChars != 0) {
                invalidCodecs += (invalidCodecs.isEmpty() ? "" : ", ") + codec->name();
            }
            QVector<QPoint> edPos = getEditPositions();
            mLoading = true;
            document()->setPlainText(text);
            setEditPositions(edPos);
            mLoading = false;
            mCodec = codec;
            if (!invalidCodecs.isEmpty()) {
                DEB() << " can't be encoded to " + invalidCodecs + ". Encoding used: " + codec->name();
            }
        } else {
            SysLogLocator::systemLog()->append("System doesn't contain codec for MIB " + QString::number(codecMib), LogMsgType::Info);
        }
        file.close();
        setModified(false);
        return;
    }
    return;
}

void FileMeta::save(const QString &newLocation)
{
    QString location = newLocation.isEmpty() ? mLocation : newLocation;
    QFile file(location);
    if (location == mLocation && !isModified()) return;

    if (location.isEmpty() || location.startsWith('['))
        EXCEPT() << "Can't save file '" << location << "'";

    if (document()) {
        mActivelySaved = true;
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            EXCEPT() << "Can't save " << location;
        QTextStream out(&file);
        if (mCodec) out.setCodec(mCodec);
        out << document()->toPlainText();
        out.flush();
        file.close();

    } else if (kind() == FileKind::Opt) {
        mActivelySaved = true;
        option::SolverOptionWidget* solverOptionWidget = ViewHelper::toSolverOptionEdit( mEditors.first() );
        if (solverOptionWidget) solverOptionWidget->saveOptionFile(location);

    } else { // no document, e.g. lst
        mActivelySaved = true;
        QFile old(mLocation);
        if (file.exists()) QFile::remove(file.fileName());

        if (!old.copy(location)) EXCEPT() << "Can't save " << location;
    }
    setLocation(location);
    QFileInfo f(file);
    FileType* newFT = &FileType::from(f.suffix());
    mData = Data(location, newFT); // react to changes in location and extension
    setModified(false);
    mFileRepo->watch(this);
}

void FileMeta::renameToBackup()
{
    const int MAX_BACKUPS = 3;
    mFileRepo->unwatch(this);

    QString filename = location();
    QFile file(filename);

    // find oldest backup file
    int last = 1;
    while (QFile(filename + "." + QString::number(last) + ".bak").exists()) {
        if (last == MAX_BACKUPS) break; // dont exceed MAX_BACKUPS
        last++;
    }
    if (last == MAX_BACKUPS) { // delete if maximum reached
        QFile(filename + "." + QString::number(last) + ".bak").remove();
        last--; // last is now one less
    }

    // move up all by 1, starting last
    for (int i = last; i > 0; i--) {
        QFile(filename + "." + QString::number(i) + ".bak") // from
                .rename(filename + "." + QString::number(i + 1) + ".bak"); // to
    }
    //rename to 1
    file.rename(filename + ".1.bak");

}

FileMeta::FileDifferences FileMeta::compare(QString fileName)
{
    FileDifferences res;
    Data other(fileName.isEmpty() ? location() : fileName);
    res = mData.compare(other);
    if (!fileName.isEmpty() && !FileMetaRepo::equals(QFileInfo(fileName), QFileInfo(location())))
        res.setFlag(FdName);
    return res;
}

void FileMeta::jumpTo(NodeId groupId, bool focus, int line, int column, int length)
{
    emit mFileRepo->openFile(this, groupId, focus, codecMib());
    if (!mEditors.size()) return;

    AbstractEdit* edit = ViewHelper::toAbstractEdit(mEditors.first());
    if (edit && line < edit->document()->blockCount()) {
        QTextBlock block = edit->document()->findBlockByNumber(line);
        QTextCursor tc = QTextCursor(block);

        tc.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, qMin(column, block.length()-1));
        tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, length);
        edit->setTextCursor(tc);

        // center line vertically
        qreal lines = qreal(edit->rect().height()) / edit->cursorRect().height();
        qreal line = qreal(edit->cursorRect().bottom()) / edit->cursorRect().height();
        int mv = int(line - lines/2);
        if (qAbs(mv) > lines/3)
            edit->verticalScrollBar()->setValue(edit->verticalScrollBar()->value()+mv);
        return;
    }
    if (TextView *tv = ViewHelper::toTextView(mEditors.first())) {
        tv->jumpTo(line, column, length, focus);
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

syntax::SyntaxHighlighter *FileMeta::highlighter() const
{
    return mHighlighter;
}

void FileMeta::marksChanged(QSet<int> lines)
{
    QMutexLocker mx(&mDirtyLinesMutex);
    mDirtyLines.unite(lines);
    if (lines.isEmpty()) mDirtyLinesUpdater.start(0);
    else if (!mDirtyLinesUpdater.isActive()) mDirtyLinesUpdater.start(500);
}

void FileMeta::reloadDelayed()
{
    for (QWidget *wid: mEditors) {
        if (TextView *tv = ViewHelper::toTextView(wid)) {
            tv->reset();
        }
    }
    mReloadTimer.start(100);
}

bool FileMeta::isModified() const
{
    if (mDocument) {
        return  mDocument->isModified();
    } else if (kind() == FileKind::Opt) {
        for (QWidget *wid: mEditors) {
            option::SolverOptionWidget *solverOptionWidget = ViewHelper::toSolverOptionEdit(wid);
            if (solverOptionWidget)
                return solverOptionWidget->isModified();
        }
    }
    return false;
}

bool FileMeta::isReadOnly() const
{
    AbstractEdit* edit = mEditors.isEmpty() ? nullptr : ViewHelper::toAbstractEdit(mEditors.first());
    if (edit) return edit->isReadOnly();

    if (kind() == FileKind::TxtRO
            || kind() == FileKind::Lst
            || kind() == FileKind::Lxi
            || kind() == FileKind::Gdx
            || kind() == FileKind::Ref
            || kind() == FileKind::Log)
        return true;
    else
        return false;
}

bool FileMeta::isAutoReload() const
{
    return mData.type->autoReload() || mTempAutoReloadTimer.isActive();
}

void FileMeta::resetTempReloadState()
{
    mTempAutoReloadTimer.start(1500);
}

void FileMeta::setModified(bool modified)
{
    if (document()) {
        document()->setModified(modified);
    } else if (kind() == FileKind::Opt) {
          for (QWidget *e : mEditors) {
               option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(e);
               if (so) so->setModified(modified);
          }
    }
}

QTextDocument *FileMeta::document() const
{
    return mDocument;
}

int FileMeta::codecMib() const
{
    return mCodec ? mCodec->mibEnum() : QTextCodec::codecForLocale()->mibEnum();
}

void FileMeta::setCodecMib(int mib)
{
    QTextCodec *codec = QTextCodec::codecForMib(mib);
    if (!codec) {
        DEB() << "TextCodec not found for MIB " << mib;
        return;
    }
    if (!isReadOnly() && codec != mCodec) {
       setModified(true);
    }
    setCodec(codec);
}

QTextCodec *FileMeta::codec() const
{
    return mCodec;
}

void FileMeta::setCodec(QTextCodec *codec)
{
    if (!codec) {
        if (!mCodec) {
            mCodec = QTextCodec::codecForLocale();
            DEB() << "Encoding parameter invalid, initialized to " << mCodec->name();
        } else {
            DEB() << "Encoding parameter invalid, left unchanged at " << mCodec->name();
        }
    } else {
        mCodec = codec;
    }
}

bool FileMeta::exists(bool ckeckNow) const
{
    if (ckeckNow) return QFileInfo(location()).exists();
    return mData.exist;
}

bool FileMeta::isOpen() const
{
    return !mEditors.isEmpty();
}

QWidget* FileMeta::createEdit(QTabWidget *tabWidget, ProjectRunGroupNode *runGroup, int codecMib, bool forcedAsTextEdit)
{
    QWidget* res = nullptr;
    if (codecMib == -1) codecMib = FileMeta::codecMib();
    if (codecMib == -1) codecMib = QTextCodec::codecForLocale()->mibEnum();
    mCodec = QTextCodec::codecForMib(codecMib);
    if (kind() == FileKind::Gdx) {
        res = ViewHelper::initEditorType(new gdxviewer::GdxViewer(location(), CommonPaths::systemDir(), mCodec, tabWidget));
    } else if (kind() == FileKind::Ref && !forcedAsTextEdit) {
        res = ViewHelper::initEditorType(new reference::ReferenceViewer(location(), mCodec, tabWidget));
    } else if (kind() == FileKind::Log) {
        LogParser *parser = new LogParser(mCodec);
        connect(parser, &LogParser::hasFile, runGroup, &ProjectRunGroupNode::hasFile);
        connect(parser, &LogParser::setErrorText, runGroup, &ProjectRunGroupNode::setErrorText);
        TextView* tView = ViewHelper::initEditorType(new TextView(TextView::MemoryText, tabWidget), EditorType::log);
        tView->setDebugMode(mFileRepo->debugMode());
        connect(tView, &TextView::hasHRef, runGroup, &ProjectRunGroupNode::hasHRef);
        connect(tView, &TextView::jumpToHRef, runGroup, &ProjectRunGroupNode::jumpToHRef);
        connect(tView, &TextView::createMarks, runGroup, &ProjectRunGroupNode::createMarks);
        tView->setLogParser(parser);
        res = tView;
    } else if (kind() == FileKind::TxtRO || kind() == FileKind::Lst) {
        EditorType type = kind() == FileKind::TxtRO ? EditorType::txtRo : EditorType::lxiLst;
        TextView* tView = ViewHelper::initEditorType(new TextView(TextView::FileText, tabWidget), type);
        tView->setDebugMode(mFileRepo->debugMode());
        res = tView;
        tView->loadFile(location(), codecMib, true);
        if (kind() == FileKind::Lst)
            res = ViewHelper::initEditorType(new lxiviewer::LxiViewer(tView, location(), tabWidget));
    } else if (kind() == FileKind::Opt && !forcedAsTextEdit) {
            QFileInfo fileInfo(name());
            support::SolverConfigInfo solverConfigInfo;
            QString defFileName = solverConfigInfo.solverOptDefFileName(fileInfo.baseName());
            if (!defFileName.isEmpty() && QFileInfo(CommonPaths::systemDir(),defFileName).exists()) {
                 res =  ViewHelper::initEditorType(new option::SolverOptionWidget(QFileInfo(name()).completeBaseName(), location(), defFileName,
                                                                                  id(), mCodec, tabWidget));
            } else if ( QFileInfo(CommonPaths::systemDir(),QString("opt%1.def").arg(fileInfo.baseName().toLower())).exists() &&
                        QString::compare(fileInfo.baseName().toLower(),"gams", Qt::CaseInsensitive)!=0 ) {
                        res =  ViewHelper::initEditorType(new option::SolverOptionWidget(QFileInfo(name()).completeBaseName(), location(), QString("opt%1.def").arg(fileInfo.baseName().toLower()),
                                                                                         id(), mCodec, tabWidget));
            } else {
                    SysLogLocator::systemLog()->append(QString("Cannot find  solver option definition file for %1. Open %1 in text editor.").arg(fileInfo.fileName()), LogMsgType::Error);
                    forcedAsTextEdit = true;
            }
    } else {
        forcedAsTextEdit = true;
    }

    if (forcedAsTextEdit) {
        AbstractEdit *edit = nullptr;
        CodeEdit *codeEdit = nullptr;
        codeEdit  = new CodeEdit(tabWidget);
        edit = (kind() == FileKind::Txt) ? ViewHelper::initEditorType(codeEdit, EditorType::txt)
                                         : ViewHelper::initEditorType(codeEdit);
        edit->setLineWrapMode(SettingsLocator::settings()->lineWrapEditor() ? QPlainTextEdit::WidgetWidth
                                                                            : QPlainTextEdit::NoWrap);
        edit->setTabChangesFocus(false);
        res = edit;
        if (kind() == FileKind::Log) {
            edit->setReadOnly(true);
            edit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        }
    }
    ViewHelper::setFileId(res, id());
    ViewHelper::setGroupId(res, runGroup->id());
    ViewHelper::setLocation(res, location());
    int i = tabWidget->insertTab(tabWidget->currentIndex()+1, res, name(NameModifier::editState));
    tabWidget->setTabToolTip(i, QDir::toNativeSeparators(location()));
    addEditor(res);
    if (mEditors.size() == 1 && kind() != FileKind::Log && ViewHelper::toAbstractEdit(res)) {
        try {
            load(codecMib);
        } catch (Exception &e) {
            Q_UNUSED(e)
            if (mEditors.size() > 0) {
                tabWidget->removeTab(tabWidget->currentIndex()+1);
                removeEditor(mEditors.first());
            }
            e.raise();
        }
    }
    return res;
}

FileMeta::Data::Data(QString location, FileType *knownType)
{
    if (knownType == &FileType::from(""))
        knownType = nullptr;
    if (location.contains('\\'))
        location = QDir::fromNativeSeparators(location);

    if (location.startsWith('[')) {
        int len = location.indexOf(']')-2;
        type = knownType ? knownType
                         : &FileType::from((len > 0) ? location.mid(1, len) : "");
    } else {
        QFileInfo fi(location);
        exist = fi.exists();
        size = fi.size();
        created = fi.birthTime();
        modified = fi.lastModified();
        type = (knownType ? knownType : &FileType::from(fi.suffix()));
    }
}

FileMeta::FileDifferences FileMeta::Data::compare(FileMeta::Data other)
{
    FileDifferences res;
    if (!other.exist) res.setFlag(FdMissing);
    else {
        if (modified != other.modified) res.setFlag(FdTime);
        if (created != other.created) res.setFlag(FdTime);
        if (size != other.size) res.setFlag(FdSize);
        if (type != other.type) res.setFlag(FdType);
    }
    return res;
}

} // namespace studio
} // namespace gams
