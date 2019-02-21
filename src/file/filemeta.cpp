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
#include "locators/settingslocator.h"
#include "studiosettings.h"
#include "commonpaths.h"
#include "editors/viewhelper.h"
#include "locators/sysloglocator.h"
#include "locators/abstractsystemlogger.h"

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
    setLocation(location);
    mTempAutoReloadTimer.setSingleShot(true); // only for measurement
    mReloadTimer.setSingleShot(true);
    connect(&mReloadTimer, &QTimer::timeout, this, &FileMeta::reload);
    mDirtyLinesUpdater.setSingleShot(true);
    connect(&mDirtyLinesUpdater, &QTimer::timeout, this, &FileMeta::updateMarks);
}

void FileMeta::setLocation(const QString &location)
{
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
        EXCEPT() << "Can't save " << location;
    QTextStream out(&file);
    if (mCodec) out.setCodec(mCodec);
    mActivelySaved = true;
    out << document()->toPlainText();
    out.flush();
    file.close();
    mData = Data(location);
    document()->setModified(false);
    mFileRepo->watch(this);
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
        mHighlighter = new SyntaxHighlighter(mDocument);
        connect(mDocument, &QTextDocument::contentsChange, this, &FileMeta::contentsChange);
        connect(mDocument, &QTextDocument::blockCountChanged, this, &FileMeta::blockCountChanged);
    } else if (kind() != FileKind::Gdx) {
        mHighlighter = new ErrorHighlighter(mDocument);
    }
    if (mHighlighter)
        mHighlighter->setMarks(mFileRepo->textMarkRepo()->marks(mId));
}

void FileMeta::unlinkAndFreeDocument()
{
    if (!mDocument) return;
    if (mHighlighter) {
        mHighlighter->setDocument(nullptr);
        mHighlighter->deleteLater();
        mHighlighter = nullptr;
    }
    disconnect(mDocument, &QTextDocument::modificationChanged, this, &FileMeta::modificationChanged);
    if (kind() == FileKind::Gms) {
        disconnect(mDocument, &QTextDocument::contentsChange, this, &FileMeta::contentsChange);
        disconnect(mDocument, &QTextDocument::blockCountChanged, this, &FileMeta::blockCountChanged);
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

void FileMeta::setKind(FileKind fk)
{
    mData.type = &FileType::from(fk);
}

FileKind FileMeta::kind() const
{
    return mData.type->kind();
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
    mChangedLine = toLine;
    if (charsAdded) {
        --mChangedLine;
        if (!column) --mChangedLine;
    }
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
        mFileRepo->textMarkRepo()->shiftMarks(id(), mChangedLine+1, newBlockCount-mLineCount);
        mLineCount = newBlockCount;
    }
}

void FileMeta::updateMarks()
{
    QMutexLocker mx(&mDirtyLinesMutex);

    // update changed editors
    for (QWidget *w: mEditors) {
        AbstractEdit *edit = nullptr;
        if (AbstractEdit * ed = ViewHelper::toAbstractEdit(w)) {
            edit = ed;
            ed->marksChanged(mDirtyLines);
        }
        if (TextView * tv = ViewHelper::toTextView(w)) {
            edit = tv->edit();
            tv->marksChanged(mDirtyLines);
        }
        if (edit && mHighlighter) {
            mHighlighter->rehighlight();
        }
    }
    mDirtyLines.clear();
}

void FileMeta::reload()
{
    load(mCodec->mibEnum(), false);
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
    CodeEdit* scEdit = ViewHelper::toCodeEdit(edit);

    if (aEdit) {
        if (!mDocument)
            linkDocument(aEdit->document());
        else
            aEdit->setDocument(mDocument);
        connect(aEdit, &AbstractEdit::requestLstTexts, mFileRepo->projectRepo(), &ProjectRepo::lstTexts);
        connect(aEdit, &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        connect(aEdit, &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
        if (scEdit) {
            connect(scEdit, &CodeEdit::requestSyntaxState, mHighlighter, &ErrorHighlighter::syntaxState);
        }
        if (!aEdit->viewport()->hasMouseTracking()) {
            aEdit->viewport()->setMouseTracking(true);
        }
    }
    if (TextView* tv = ViewHelper::toTextView(edit)) {
        connect(tv->edit(), &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        connect(tv->edit(), &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
        tv->setMarks(mFileRepo->textMarkRepo()->marks(mId));
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
    mEditors.removeAt(i);

    if (aEdit) {
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
        disconnect(tv->edit(), &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        disconnect(tv->edit(), &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
    }

    if (mEditors.isEmpty()) {
        mFileRepo->textMarkRepo()->removeMarks(id(), QSet<TextMark::Type>() << TextMark::bookmark);
    }
    if (scEdit && mHighlighter) {
        disconnect(scEdit, &CodeEdit::requestSyntaxState, mHighlighter, &ErrorHighlighter::syntaxState);
    }
}

bool FileMeta::hasEditor(QWidget * const &edit) const
{
    return mEditors.contains(edit);
}

void FileMeta::load(int codecMib, bool init)
{
    // TODO(JM) Later, this method should be moved to the new DataWidget
    if (codecMib == -1) codecMib = QTextCodec::codecForLocale()->mibEnum();

    mData = Data(location());
    // TODO(JM) Later, this method should be moved to the new DataWidget
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


    QFile file(location());
    qint64 maxSize = SettingsLocator::settings()->editableMaxSizeMB() *1024*1024;
    if (file.exists() && file.size() > maxSize) {
        SysLogLocator::systemLog()->append("File size of " + QString::number(qreal(maxSize)/1024/1024, 'f', 1)
                                           + " MB exeeded by " + location());
        EXCEPT() << ("File size of " + QString::number(qreal(maxSize)/1024/1024, 'f', 1)
                     + " MB exeeded by " + location());
//        return;
    }
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
        document()->setModified(false);
        return;
    }
    return;
}

void FileMeta::save()
{
    if (!isModified()) return;
    if (location().isEmpty() || location().startsWith('['))
        EXCEPT() << "Can't save file '" << location() << "'";
    internalSave(location());
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

FileDifferences FileMeta::compare(QString fileName)
{
    FileDifferences res;
    Data other(fileName.isEmpty() ? location() : fileName);
    res = mData.compare(other);
    if (!fileName.isEmpty() && !FileMetaRepo::equals(QFileInfo(fileName), QFileInfo(location())))
        res.setFlag(FdName);
    return res;
}

void FileMeta::jumpTo(NodeId groupId, bool focus, int line, int column)
{
    emit mFileRepo->openFile(this, groupId, focus, codecMib());
    if (!mEditors.size()) return;
    AbstractEdit* edit = ViewHelper::toAbstractEdit(mEditors.first());
    if (edit && line < edit->document()->blockCount()) {
        QTextBlock block = edit->document()->findBlockByNumber(line);
        QTextCursor tc = QTextCursor(block);
        tc.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, qMin(column, block.length()-1));
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
        tv->jumpTo(line, column);
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

void FileMeta::marksChanged(QSet<int> lines)
{
    QMutexLocker mx(&mDirtyLinesMutex);
    mDirtyLines.unite(lines);
    if (lines.isEmpty()) mDirtyLinesUpdater.start(0);
    else if (!mDirtyLinesUpdater.isActive()) mDirtyLinesUpdater.start(500);
}

void FileMeta::reloadDelayed()
{
    mReloadTimer.start(100);
}

bool FileMeta::isModified() const
{
    return mDocument ? mDocument->isModified() : false;
}

bool FileMeta::isReadOnly() const
{
    AbstractEdit* edit = mEditors.isEmpty() ? nullptr : ViewHelper::toAbstractEdit(mEditors.first());
    if (!edit) return true;
    return edit->isReadOnly();
}

bool FileMeta::isAutoReload() const
{
    return mData.type->autoReload() || mTempAutoReloadTimer.isActive();
}

void FileMeta::resetTempReloadState()
{
    mTempAutoReloadTimer.start(1500);
}

QTextDocument *FileMeta::document() const
{
    return mDocument;
}

int FileMeta::codecMib() const
{
    return mCodec ? mCodec->mibEnum() : -1;
}

void FileMeta::setCodecMib(int mib)
{
    QTextCodec *codec = QTextCodec::codecForMib(mib);
    if (!codec) {
        DEB() << "TextCodec not found for MIB " << mib;
        return;
    }
    if (document() && !isReadOnly() && codec != mCodec) {
        document()->setModified();
        setCodec(codec);
    }
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

QWidget* FileMeta::createEdit(QTabWidget *tabWidget, ProjectRunGroupNode *runGroup, int codecMib)
{
    QWidget* res = nullptr;
    if (codecMib == -1) codecMib = QTextCodec::codecForLocale()->mibEnum();
    mCodec = QTextCodec::codecForMib(codecMib);
    if (kind() == FileKind::Gdx) {
        res = ViewHelper::initEditorType(new gdxviewer::GdxViewer(location(), CommonPaths::systemDir(), mCodec, tabWidget));
    } else if (kind() == FileKind::Ref) {
        // TODO: multiple ReferenceViewers share one Reference Object of the same file
        //       instead of holding individual Reference Object
        res = ViewHelper::initEditorType(new reference::ReferenceViewer(location(), mCodec, tabWidget));
    } else if (kind() == FileKind::TxtRO || kind() == FileKind::Lst) {
        TextView* tView = ViewHelper::initEditorType(new TextView(tabWidget));
        res = tView;
//        tView->loadFile(location());
//        QTimer::singleShot(1, tView, &PagingTextView::reorganize);
        if (kind() == FileKind::Lst) {
            res = ViewHelper::initEditorType(new lxiviewer::LxiViewer(tView, location(), tabWidget));
        }
        if (kind() == FileKind::Log || kind() == FileKind::Lst || kind() == FileKind::TxtRO) {
//            tView->setReadOnly(true);
//            tView->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        }
    } else {
        AbstractEdit *edit = nullptr;
        CodeEdit *codeEdit = nullptr;
        if (kind() == FileKind::Log) {
            edit = ViewHelper::initEditorType(new ProcessLogEdit(tabWidget));
        } else {
            codeEdit  = new CodeEdit(tabWidget);
            edit = (kind() == FileKind::Txt) ? ViewHelper::initEditorType(codeEdit, EditorType::txt)
                                             : ViewHelper::initEditorType(codeEdit);
        }
        edit->setLineWrapMode(SettingsLocator::settings()->lineWrapEditor() ? QPlainTextEdit::WidgetWidth
                                                                            : QPlainTextEdit::NoWrap);
        edit->setTabChangesFocus(false);
        res = edit;
//        if (kind() == FileKind::Lst) {
//            res = ViewHelper::initEditorType(new lxiviewer::LxiViewer(codeEdit, location(), tabWidget));
//        }
        if (kind() == FileKind::Log || /*kind() == FileKind::Lst ||*/ kind() == FileKind::TxtRO) {
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
    if (mEditors.size() == 1 && kind() != FileKind::Log && (ViewHelper::toAbstractEdit(res) || ViewHelper::toTextView(res)))
        load(codecMib);
    return res;
}

FileMeta::Data::Data(QString location, FileType *knownType)
{
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

FileDifferences FileMeta::Data::compare(FileMeta::Data other)
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
