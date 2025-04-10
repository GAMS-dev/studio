/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "settings.h"
#include "commonpaths.h"
#include "viewhelper.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "support/solverconfiginfo.h"
#include "editors/navigationhistory.h"
#include "editors/navigationhistorylocator.h"
#include "textfilesaver.h"

#include "encoding.h"
#include <QStringConverter>

#include <QTabWidget>
#include <QFileInfo>
#include <QFile>
#include <QPlainTextDocumentLayout>
#include <QScrollBar>
#include <QApplication>

namespace gams {
namespace studio {

FileMeta::FileMeta(FileMetaRepo *fileRepo, const FileId &id, const QString &location, FileType *knownType)
    : mId(id), mFileRepo(fileRepo), mData(Data(location, knownType))
{
    if (!mFileRepo) EXCEPT() << "FileMetaRepo  must not be null";
    if (mData.type->kind() == FileKind::TxtRO && mData.size < 1024*1024)
        setKind(FileKind::TxtRO);
    mEncoding = Encoding::defaultEncoding(Settings::settings()->toInt(skDefaultCodecMib));
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
        for (QWidget*wid: std::as_const(mEditors)) {
            ViewHelper::setLocation(wid, location);
        }
        mAutoReload = mData.type->autoReload();
    }
}

void FileMeta::updateExtraSelections()
{
    for (QWidget *wid : mEditors) {
        if (AbstractEdit* aEdit = ViewHelper::toAbstractEdit(wid))
            aEdit->updateExtraSelections();
    }
}

bool FileMeta::hasExistingFile(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        if (url.isEmpty()) continue;
        QFileInfo fi(url.toLocalFile());
        if (fi.isFile() && fi.exists()) return true;
    }
    return false;
}

bool FileMeta::hasExistingFolder(const QList<QUrl> &urls) {
    for (const QUrl &url : urls) {
        if (url.isEmpty()) continue;
        QDir d(url.toLocalFile());
        if (d.exists()) return true;
    }
    return false;
}

QStringList FileMeta::pathList(const QList<QUrl> &urls)
{
    QStringList res;
    for (const QUrl &url : urls) {
        if (url.isEmpty()) continue;

        QFileInfo fi(url.toLocalFile());
        if (fi.isFile() && fi.exists())
            res << url.toLocalFile();

        QDir d(url.toLocalFile());
        if (d.exists())
            res << url.toLocalFile();
    }
    return res;
}

void FileMeta::invalidate()
{
    if (kind() == FileKind::Gdx) {
        for (QWidget *wid: std::as_const(mEditors)) {
            if (gdxviewer::GdxViewer *gdxViewer = ViewHelper::toGdxViewer(wid)) {
                gdxViewer->invalidate();
            }
        }
        return;
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
    for (QWidget* widget: std::as_const(mEditors)) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(widget);
        if (edit) {
            QTextCursor cursor = edit->textCursor();
            res << QPoint(cursor.positionInBlock(), cursor.blockNumber());
        }
    }
    return res;
}

void FileMeta::setEditPositions(const QVector<QPoint> &edPositions)
{
    int i = 0;
    for (QWidget* widget: std::as_const(mEditors)) {
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

void FileMeta::updateEditsCompleter()
{
    if (!editors().isEmpty()) {
        for (QWidget *wid : editors()) {
            if (CodeEdit *ce = ViewHelper::toCodeEdit(wid))
                ce->setCompleter(mHighlighter ? mFileRepo->completer() : nullptr);
        }
    }
}

void FileMeta::linkDocument(QTextDocument *doc)
{
    // The very first editor opened for this FileMeta should pass it's document here. It takes over parency
    if (kind() != FileKind::Gdx) {
        if (!doc) doc = new QTextDocument(this);
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
        if (Settings::settings()->toBool(skEdFoldedDcoOnOpen))
            connect(mHighlighter, &syntax::BaseHighlighter::completed, this, [this]() {
                for (QWidget *wid : editors()) {
                    if (CodeEdit *ce = ViewHelper::toCodeEdit(wid))
                        ce->foldAll(true);
                }
            });
        updateEditsCompleter();
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

void FileMeta::refreshType()
{
    QFileInfo f(mLocation);
    FileType* newFT = &FileType::from(f.fileName());
    FileKind oldKind = kind();
    mData = Data(mLocation, newFT); // react to changes in location and extension
    if (mDocument) {
        if (mHighlighter && kind() != FileKind::Gms) {
            mHighlighter->setDocument(nullptr, true);
            mHighlighter->deleteLater();
            mHighlighter = nullptr;
            disconnect(mDocument, &QTextDocument::contentsChange, this, &FileMeta::contentsChange);
            disconnect(mDocument, &QTextDocument::blockCountChanged, this, &FileMeta::blockCountChanged);
            for (QWidget *edit : std::as_const(mEditors)) {
                CodeEdit* scEdit = ViewHelper::toCodeEdit(edit);
                if (scEdit && mHighlighter) {
                    disconnect(scEdit, &CodeEdit::requestSyntaxKind, mHighlighter, &syntax::SyntaxHighlighter::syntaxKind);
                    disconnect(scEdit, &CodeEdit::requestSyntaxKind, mHighlighter, &syntax::SyntaxHighlighter::syntaxKind);
                    disconnect(scEdit, &CodeEdit::scanSyntax, mHighlighter, &syntax::SyntaxHighlighter::scanSyntax);
                    disconnect(scEdit, &CodeEdit::syntaxDocAt, mHighlighter, &syntax::SyntaxHighlighter::syntaxDocAt);
                    disconnect(scEdit, &CodeEdit::syntaxFlagData, mHighlighter, &syntax::SyntaxHighlighter::syntaxFlagData);
                    disconnect(mHighlighter, &syntax::SyntaxHighlighter::needUnfold, scEdit, &CodeEdit::unfold);
                }
            }
        }
        if (kind() == FileKind::Gms && kind() != oldKind) {
            mHighlighter = new syntax::SyntaxHighlighter(mDocument);
            connect(mDocument, &QTextDocument::contentsChange, this, &FileMeta::contentsChange);
            connect(mDocument, &QTextDocument::blockCountChanged, this, &FileMeta::blockCountChanged);
            for (QWidget *edit : std::as_const(mEditors)) {
                CodeEdit* scEdit = ViewHelper::toCodeEdit(edit);
                if (scEdit && mHighlighter) {
                    connect(scEdit, &CodeEdit::requestSyntaxKind, mHighlighter, &syntax::SyntaxHighlighter::syntaxKind);
                    connect(scEdit, &CodeEdit::requestSyntaxKind, mHighlighter, &syntax::SyntaxHighlighter::syntaxKind);
                    connect(scEdit, &CodeEdit::scanSyntax, mHighlighter, &syntax::SyntaxHighlighter::scanSyntax);
                    connect(scEdit, &CodeEdit::syntaxDocAt, mHighlighter, &syntax::SyntaxHighlighter::syntaxDocAt);
                    connect(scEdit, &CodeEdit::syntaxFlagData, mHighlighter, &syntax::SyntaxHighlighter::syntaxFlagData);
                    connect(mHighlighter, &syntax::SyntaxHighlighter::needUnfold, scEdit, &CodeEdit::unfold);
                }
            }
        }
        updateEditsCompleter();
    }
    emit changed(mId);
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

void FileMeta::setKind(FileKind kind)
{
    if (kind == FileKind::TxtRO && mData.size < 1024*1024) {
        mData.type = &FileType::from(FileKind::Txt);
        setReadOnly(true);
    } else {
        mData.type = &FileType::from(kind);
    }
}

FileKind FileMeta::kind() const
{
    return mData.type->kind();
}

QString FileMeta::kindAsStr() const
{
    return mData.type->suffix().constFirst();
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

FontGroup FileMeta::fontGroup(bool forcedAsTextEdit)
{
    const QSet<FileKind> editKind {FileKind::Gms, FileKind::Lst, FileKind::Lxi, FileKind::None, FileKind::Txt, FileKind::TxtRO};
    const QSet<FileKind> forcedTextEditKind {FileKind::Opt, FileKind::GCon};
    if (kind() == FileKind::Log) return FontGroup::fgLog;
    if (forcedAsTextEdit && forcedTextEditKind.contains(kind())) return FontGroup::fgText;
    if (editKind.contains(kind())) return FontGroup::fgText;
    if (document()) return FontGroup::fgText;
    return FontGroup::fgTable;
}

const QWidgetList FileMeta::editors() const
{
    return mEditors;
}

QWidget *FileMeta::topEditor() const
{
    return isOpen() ? mEditors.first() : nullptr;
}

void FileMeta::modificationChanged(bool modiState)
{
    Q_UNUSED(modiState)
    if (kind() == FileKind::Gsp) {
        if (project::ProjectEdit *pro = ViewHelper::toProjectEdit(topEditor())) {
            mName = '['+pro->sharedData()->fieldData(project::ProjectData::name)
                    +pro->sharedData()->fieldData(project::ProjectData::nameExt)+']';
        }
    }
    emit changed(mId);
}

void FileMeta::contentsChange(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsRemoved)
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
    edit->ensureUnfolded(toLine);
//    if (charsAdded) --mChangedLine;
//    if (!column) --mChangedLine;
    if (removedLines > 0)
        mFileRepo->textMarkRepo()->removeMarks(id(), projectId(), QSet<TextMark::Type>()
                                               , fromLine, fromLine+removedLines);
    for (int i = fromLine; i <= toLine; ++i) {
        const QList<TextMark*> marks = mFileRepo->textMarkRepo()->marks(id(), i, projectId());
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
    for (QWidget *w: std::as_const(mEditors)) {
        if (AbstractEdit * ed = ViewHelper::toAbstractEdit(w))
            ed->marksChanged(mDirtyLines);
        if (TextView * tv = ViewHelper::toTextView(w))
            tv->marksChanged(mDirtyLines);
    }
    mDirtyLines.clear();
}

void FileMeta::zoomRequest(qreal delta)
{
    if (!mEditors.size()) return;
    QFont f = mEditors.first()->font();
    if (delta == 0.) return;
    const qreal newSize = f.pointSizeF() + delta;
    if (newSize <= 0) return;
    f.setPointSizeF(newSize);
    emit fontChangeRequest(this, f);
}

void FileMeta::reload()
{
    load(mEncoding, false);
}

void FileMeta::updateView()
{
    for (QWidget *wid: std::as_const(mEditors)) {
        if (TextView* tv = ViewHelper::toTextView(wid)) {
            tv->recalcVisibleLines();
        }
    }
}

void FileMeta::updateSyntaxColors(bool refreshSyntax)
{
    if (mHighlighter) {
        bool changed = mHighlighter->reloadColors() || refreshSyntax;
        if (changed) {
            for (QWidget *w: std::as_const(mEditors)) {
                if (ViewHelper::toCodeEdit(w))
                    mHighlighter->rehighlight();
            }
        }
    }

}

void FileMeta::initEditorColors()
{
    for (QWidget *w: std::as_const(mEditors)) {
        if (reference::ReferenceViewer *rv = ViewHelper::toReferenceViewer(w)) {
            rv->updateStyle();
            continue;
        }
        AbstractEdit *ed = ViewHelper::toAbstractEdit(w);
        if (lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(w))
            ed = lxi->textView()->edit();
        if (TextView *tv = ViewHelper::toTextView(w))
            ed = tv->edit();
        if (!ed) continue;
        if (Theme::color(Theme::Edit_text) == Qt::transparent &&
                Theme::color(Theme::Edit_background) == Qt::transparent) {
            ed->setAutoFillBackground(false);
            ed->setPalette(qApp->palette());
        } else if (ed->palette().windowText().color() != Theme::color(Theme::Edit_text) ||
                ed->palette().window().color() != Theme::color(Theme::Edit_background)) {
            ed->setAutoFillBackground(true);
            QPalette pal = qApp->palette();
            pal.setColor(QPalette::Text, Theme::color(Theme::Edit_text));
            pal.setColor(QPalette::Base, Theme::color(Theme::Edit_background));
            ed->setPalette(pal);
        }
    }
}

void FileMeta::updateEditorColors()
{
    initEditorColors();
    for (QWidget *w: std::as_const(mEditors)) {
        if (AbstractEdit *ce = ViewHelper::toAbstractEdit(w))
            ce->updateExtraSelections();
        if (CodeEdit *ce = ViewHelper::toCodeEdit(w))
            ce->lineNumberArea()->update();
        if (TextView *tv = ViewHelper::toTextView(w))
            tv->updateTheme();
    }
}

void FileMeta::invalidateTheme(bool refreshSyntax)
{
    updateEditorColors();
    updateSyntaxColors(refreshSyntax);
}

bool FileMeta::eventFilter(QObject *sender, QEvent *event)
{
    if (event->type() == QEvent::ApplicationFontChange || event->type() == QEvent::FontChange) {
        const QFont *f = nullptr;
        QList<QWidget*> syncEdits;
        for (QWidget *wid : std::as_const(mEditors)) {
            if (lxiviewer::LxiViewer *lst = ViewHelper::toLxiViewer(wid)) {
                if (lst->textView()->edit() == sender)
                    f = &lst->textView()->edit()->font();
                else
                    syncEdits << lst->textView()->edit();
            } else if (TextView *tv = ViewHelper::toTextView(wid)) {
                if (tv->edit() == sender)
                    f = &tv->edit()->font();
                else
                    syncEdits << tv->edit();
            } else {
                if (wid == sender)
                    f = &wid->font();
                else
                    syncEdits << wid;
            }
        }
        if (f) {
            for (QWidget *wid : std::as_const(syncEdits)) {
                if (!wid->font().isCopyOf(*f))
                    wid->setFont(*f);
            }
        }
    }
    return QObject::eventFilter(sender, event);
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
    initEditorColors();
    ViewHelper::setLocation(edit, location());
    AbstractEdit* aEdit = ViewHelper::toAbstractEdit(edit);

    edit->installEventFilter(this);
    if (aEdit) {
        if (!mDocument)
            linkDocument(aEdit->document());
        else
            aEdit->setDocument(mDocument);
        connect(aEdit, &AbstractEdit::requestLstTexts, mFileRepo->projectRepo(), &ProjectRepo::errorTexts);
        connect(aEdit, &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        connect(aEdit, &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
        connect(aEdit, &AbstractEdit::zoomRequest, this, &FileMeta::zoomRequest);
        connect(aEdit, &AbstractEdit::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
        connect(aEdit, &AbstractEdit::getProjectId, this, [this](NodeId &projectId) { projectId = this->projectId(); });
        connect(aEdit, &AbstractEdit::getFileId, this, [this](FileId &fileId) { fileId = this->id(); });

        CodeEdit* scEdit = ViewHelper::toCodeEdit(edit);
        if (scEdit && mHighlighter) {
            scEdit->setCompleter(mFileRepo->completer());
            connect(scEdit, &CodeEdit::requestSyntaxKind, mHighlighter, &syntax::SyntaxHighlighter::syntaxKind);
            connect(scEdit, &CodeEdit::scanSyntax, mHighlighter, &syntax::SyntaxHighlighter::scanSyntax);
            connect(scEdit, &CodeEdit::syntaxDocAt, mHighlighter, &syntax::SyntaxHighlighter::syntaxDocAt);
            connect(scEdit, &CodeEdit::syntaxFlagData, mHighlighter, &syntax::SyntaxHighlighter::syntaxFlagData);
            connect(mHighlighter, &syntax::SyntaxHighlighter::needUnfold, scEdit, &CodeEdit::unfold);
            if (mEditors.size() > 1)
                updateBreakpoints();
        }

        if (!aEdit->viewport()->hasMouseTracking())
            aEdit->viewport()->setMouseTracking(true);
    }

    if (TextView* tv = ViewHelper::toTextView(edit)) {
        connect(tv->edit(), &AbstractEdit::requestLstTexts, mFileRepo->projectRepo(), &ProjectRepo::errorTexts);
        connect(tv->edit(), &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        connect(tv->edit(), &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
        connect(tv->edit(), &AbstractEdit::zoomRequest, this, &FileMeta::zoomRequest);
        connect(tv->edit(), &AbstractEdit::getProjectId, this, [this](NodeId &projectId) { projectId = this->projectId(); });
        connect(tv->edit(), &AbstractEdit::getFileId, this, [this](FileId &fileId) { fileId = this->id(); });
        if (lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(edit))
            connect(lxi, &lxiviewer::LxiViewer::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
        else
            connect(tv, &TextView::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
        if (aEdit)
            disconnect(aEdit, &AbstractEdit::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
        if (tv->kind() == TextView::FileText)
            tv->setMarks(mFileRepo->textMarkRepo()->marks(mId));
    } else if (project::ProjectEdit* prOp = ViewHelper::toProjectEdit(edit)) {
        connect(prOp, &project::ProjectEdit::modificationChanged, this, &FileMeta::modificationChanged);
        connect(prOp, &project::ProjectEdit::saveProjects, mFileRepo, &FileMetaRepo::saveProjects);
        connect(this, &FileMeta::saveProjects, mFileRepo, &FileMetaRepo::saveProjects, Qt::UniqueConnection);
    } else if (option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(edit)) {
        connect(soEdit, &option::SolverOptionWidget::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (connect::ConnectEditor* gcEdit = ViewHelper::toGamsConnectEditor(edit)) {
        connect(gcEdit, &connect::ConnectEditor::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (option::GamsConfigEditor* gucEdit = ViewHelper::toGamsConfigEditor(edit)) {
        connect(gucEdit, &option::GamsConfigEditor::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (efi::EfiEditor* efi = ViewHelper::toEfiEditor(edit)) {
        connect(efi, &efi::EfiEditor::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (connect::ConnectEditor* cEdit = ViewHelper::toGamsConnectEditor(edit)) {
        connect(cEdit, &connect::ConnectEditor::modificationChanged, this, &FileMeta::modificationChanged);
    }
    if (AbstractView* av = ViewHelper::toAbstractView(edit)) {
        connect(av, &AbstractView::zoomRequest, this, &FileMeta::zoomRequest);
    }

    if (mEditors.size() == 1) emit documentOpened();
    if (aEdit)
        aEdit->setMarks(mFileRepo->textMarkRepo()->marks(mId));
}

void FileMeta::updateBreakpoints()
{
    PExProjectNode *project = mFileRepo->projectRepo()->asProject(mProjectId);
    if (project) {
        SortedIntMap bpLines;
        SortedIntMap abpLines;
        project->breakpoints(mLocation, bpLines, abpLines);
        for (QWidget *wid : mEditors) {
            if (CodeEdit *ce = ViewHelper::toCodeEdit(wid)) {
                ce->breakpointsChanged(bpLines, abpLines);
            }
        }
    }
}

const NodeId &FileMeta::projectId() const
{
    return mProjectId;
}

void FileMeta::setProjectId(const NodeId &newProjectId)
{
    mProjectId = newProjectId;
    if (kind() == FileKind::Gms)
        updateBreakpoints();
}

void FileMeta::editToTop(QWidget *edit)
{
    addEditor(edit);
}

void FileMeta::deleteEdit(QWidget *edit)
{
    int i = mEditors.indexOf(edit);
    if (i < 0) {
        DEB() << "Delete edit failed for " << mLocation;
        return;
    }

    AbstractEdit* aEdit = ViewHelper::toAbstractEdit(edit);
    CodeEdit* scEdit = ViewHelper::toCodeEdit(edit);
    mEditors.removeAt(i);
    edit->removeEventFilter(this);

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
        disconnect(aEdit, &AbstractEdit::zoomRequest, this, &FileMeta::zoomRequest);
        disconnect(aEdit, &AbstractEdit::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
    }

    if (TextView* tv = ViewHelper::toTextView(edit)) {
        tv->edit()->setMarks(nullptr);
        tv->edit()->disconnectTimers();
        disconnect(tv->edit(), &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        disconnect(tv->edit(), &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
        disconnect(tv->edit(), &AbstractEdit::zoomRequest, this, &FileMeta::zoomRequest);
        disconnect(tv, &TextView::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
    } else if (project::ProjectEdit* prOp = ViewHelper::toProjectEdit(edit)) {
       disconnect(prOp, &project::ProjectEdit::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(edit)) {
       disconnect(soEdit, &option::SolverOptionWidget::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (option::GamsConfigEditor* gucEdit = ViewHelper::toGamsConfigEditor(edit)) {
        disconnect(gucEdit, &option::GamsConfigEditor::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (efi::EfiEditor* efi = ViewHelper::toEfiEditor(edit)) {
        disconnect(efi, &efi::EfiEditor::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (connect::ConnectEditor* cEdit = ViewHelper::toGamsConnectEditor(edit)) {
        disconnect(cEdit, &connect::ConnectEditor::modificationChanged, this, &FileMeta::modificationChanged);
    } else if (gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(edit)) {
        gdx->writeState(location());
    }
    if (AbstractView* av = ViewHelper::toAbstractView(edit)) {
        disconnect(av, &AbstractView::zoomRequest, this, &FileMeta::zoomRequest);
    }

    if (mEditors.isEmpty() && kind() != FileKind::Gsp) {
        mFileRepo->textMarkRepo()->removeMarks(id(), QSet<TextMark::Type>() << TextMark::bookmark);
        mFileRepo->watch(this);
    }
    if (scEdit && mHighlighter) {
        scEdit->setCompleter(nullptr);
        disconnect(scEdit, &CodeEdit::requestSyntaxKind, mHighlighter, &syntax::SyntaxHighlighter::syntaxKind);
        disconnect(scEdit, &CodeEdit::scanSyntax, mHighlighter, &syntax::SyntaxHighlighter::scanSyntax);
        disconnect(scEdit, &CodeEdit::syntaxDocAt, mHighlighter, &syntax::SyntaxHighlighter::syntaxDocAt);
        disconnect(scEdit, &CodeEdit::syntaxFlagData, mHighlighter, &syntax::SyntaxHighlighter::syntaxFlagData);
        disconnect(mHighlighter, &syntax::SyntaxHighlighter::needUnfold, scEdit, &CodeEdit::unfold);
    }
    if (gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(edit))
        gdx->saveDelete();
    else
        edit->deleteLater();
    DEB() << "Deleted 1/" << (mEditors.size() + 1) << " editors of " << mLocation;
}

bool FileMeta::hasEditor(QWidget * const &edit) const
{
    return mEditors.contains(edit);
}

void FileMeta::load(QString encoding, bool init)
{
    if (encoding.isEmpty())
        encoding = Encoding::defaultEncoding();
    mEncoding = encoding;
    mData = Data(location(), mData.type);

    if (kind() == FileKind::Gsp) {
        // Project reload not supported
        return;
    }
    if (kind() == FileKind::Gdx) {
        for (QWidget *wid: std::as_const(mEditors)) {
            if (gdxviewer::GdxViewer *gdxViewer = ViewHelper::toGdxViewer(wid)) {
                gdxViewer->setHasChanged(init);
                gdxViewer->reload(encoding);
            }
        }
        return;
    }
    if (kind() == FileKind::TxtRO || kind() == FileKind::Lst) {
        for (QWidget *wid: std::as_const(mEditors)) {
            if (TextView *tView = ViewHelper::toTextView(wid)) {
                tView->loadFile(location(), encoding, init);
            }
            if (kind() == FileKind::Lst) {
                lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(wid);
                if (lxi) lxi->loadLxi();
            }
        }
        return;
    }
    if (kind() == FileKind::Ref) {
        for (QWidget *wid: std::as_const(mEditors)) {
            reference::ReferenceViewer *refViewer = ViewHelper::toReferenceViewer(wid);
            if (refViewer) refViewer->reloadFile(encoding);
        }
        return;
    }
    if (kind() == FileKind::Opt || kind() == FileKind::Pf) {
        bool done = false;
        for (QWidget *wid : std::as_const(mEditors)) {
            if (option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(wid)) {
                done = true;
                so->on_reloadSolverOptionFile(encoding);
            }
        }
        if (done) return;
    }
    if (kind() == FileKind::GCon) {
        bool done = false;
        for (QWidget *wid : std::as_const(mEditors)) {
            if (connect::ConnectEditor *cyaml = ViewHelper::toGamsConnectEditor(wid)) {
                done = true;
                cyaml->on_reloadConnectFile();
            }
        }
        if (done) return;
    }
    if (kind() == FileKind::Guc) {
        bool done = false;
        for (QWidget *wid : std::as_const(mEditors)) {
            if (option::GamsConfigEditor *cfge = ViewHelper::toGamsConfigEditor(wid)) {
                done = true;
                cfge->on_reloadGamsUserConfigFile();
            }
        }
        if (done) return;
    }
    if (kind() == FileKind::Efi) {
        bool done = false;
        for (QWidget *wid : std::as_const(mEditors)) {
            if (efi::EfiEditor *ee = ViewHelper::toEfiEditor(wid)) {
                done = true;
                ee->load(location());
            }
        }
        if (done) return;
    }

    QFile file(location());
    bool canOpen = true;
    emit editableFileSizeCheck(file, canOpen);
    if (!canOpen)
        EXCEPT() << "FileMeta" << '\t' << "Size for editable files exceeded: " << file.fileName();

    if (!mDocument) {
        linkDocument();
    }
    if (!file.fileName().isEmpty() && file.exists()) {
        if (!file.open(QFile::ReadOnly | QFile::Text))
            EXCEPT() << "Error opening file " << location();

        const QByteArray data(file.readAll());
        QStringDecoder decoder = Encoding::createDecoder(encoding);
        if (!decoder.isValid()) {
            if (auto encode = decoder.encodingForData(data))
                decoder = QStringDecoder(*encode);
        }

        if (decoder.isValid()) {
            QString msg;
            QString text = decoder.decode(data);
            if (decoder.hasError()) {
                msg = "Errors when decoding with '" + encoding + "'";
                if (auto encode = decoder.encodingForData(data))
                    msg += " Decoding with '" + QString(QStringConverter::nameForEncoding(*encode)) + "' might work";
                    //decoder = QStringDecoder(*encode);

            }
            QVector<QPoint> edPos = getEditPositions();
            mLoading = true;
            if (mHighlighter) mHighlighter->pause();
            document()->setPlainText(text);
            if (mHighlighter) mHighlighter->resume();
            setEditPositions(edPos);
            mLoading = false;
            if (!msg.isEmpty()) DEB() << msg;
        } else {
            SysLogLocator::systemLog()->append("Invalid encoding " + encoding, LogMsgType::Info);
        }
        file.close();
        setModified(false);
        return;
    }
    return;
}

///
/// \brief FileMeta::save Saves the data to a file
/// \param newLocation the filepath of the new or replaced file
/// \return
///
bool FileMeta::save(const QString &newLocation)
{
    QString location = newLocation.isEmpty() ? mLocation : newLocation;
    QFile file(location);
    if (location == mLocation && !isModified()) return true;

    if (location.isEmpty() || location.startsWith('['))
        EXCEPT() << "Can't save file '" << location << "'";

    bool res = true;
    mFileRepo->unwatch(this);
    mFileRepo->unwatch(newLocation);
    if (document()) {
        TextFileSaver saver;
        if (!saver.open(location))
            EXCEPT() << "Can't save " << location;
        QStringEncoder enc = QStringEncoder(mEncoding.toLatin1());
        saver.write(enc(document()->toPlainText()));
        res = saver.close();
    } else if (kind() == FileKind::Gsp) {
        project::ProjectEdit* PEd = ViewHelper::toProjectEdit(mEditors.first());
        if (PEd) res = PEd->save();
        if (res) {
            saveProjects();
            PExProjectNode *project = mFileRepo->projectRepo()->asProject(projectId());
            if (project && project->needSave())
                res = false;
        }
    } else if (kind() == FileKind::Opt || kind() == FileKind::Pf) {
        option::SolverOptionWidget* solverOptionWidget = ViewHelper::toSolverOptionEdit( mEditors.first() );
        if (solverOptionWidget) res = solverOptionWidget->saveOptionFile(location);

    } else if (kind() == FileKind::Guc) {
        option::GamsConfigEditor* gucEditor = ViewHelper::toGamsConfigEditor( mEditors.first() );
        if (gucEditor) res = gucEditor->saveConfigFile(location);
    } else if (kind() == FileKind::Efi) {
        efi::EfiEditor* efi = ViewHelper::toEfiEditor( mEditors.first() );
        if (efi) {
            PExProjectNode *project = mFileRepo->projectRepo()->asProject(projectId());
            if (project) {
                if (!project->runnableGms()) {
                    efi->setWarnText("Warning: project contains no runnable GAMS file ");
                } else {
                    QFileInfo gmsFi(project->runnableGms()->location());
                    QFileInfo thisFi(location);
                    if (gmsFi.completeBaseName().compare(thisFi.completeBaseName(), FileType::fsCaseSense()) != 0) {
                        QString text = "Warning: only " + gmsFi.completeBaseName()
                                + ".efi is used for " + thisFi.completeBaseName();
                        efi->setWarnText(text);
                    }
                }
            }
            res = efi->save(location);
        }
    } else if (kind() == FileKind::GCon) {
        connect::ConnectEditor* gconEditor = ViewHelper::toGamsConnectEditor( mEditors.first() );
        if (gconEditor) res = gconEditor->saveConnectFile(location);

    } else { // no document, e.g. lst
        QFile old(mLocation);
        if (file.exists()) QFile::remove(file.fileName());

        if (!old.copy(location)) EXCEPT() << "Can't save " << location;
    }
    setLocation(location);
    refreshType();
    if (res)
        setModified(false);

    if (kind() != FileKind::Gsp) {
        // reactivate file watcher
        QTimer::singleShot(500, this, [this]() {
            mFileRepo->watch(this);
        });
    }
    return res;
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

FileMeta::FileDifferences FileMeta::compare(const QString &fileName)
{
    FileDifferences res;
    Data other(fileName.isEmpty() ? location() : fileName);
    res = mData.compare(other);
    if (!fileName.isEmpty() && !FileMetaRepo::equals(QFileInfo(fileName), QFileInfo(location())))
        res.setFlag(FdName);
    return res;
}

bool FileMeta::refreshMetaData()
{
    Data data(location(), mData.type);
    bool res = (mData.compare(data) != FdEqual);
    mData = data;
    return res;
}

void FileMeta::jumpTo(const NodeId &groupId, bool focus, int line, int column, int length)
{
    mFileRepo->openFile(this, groupId, focus, encoding());
    if (!mEditors.size()) return;

    AbstractEdit* edit = ViewHelper::toAbstractEdit(mEditors.first());
    if (edit && line < edit->document()->blockCount()) {
        QTextBlock block = edit->document()->findBlockByNumber(line);
        NavigationHistoryLocator::navigationHistory()->stopRecord();
        edit->jumpTo(line, qMin(column, block.length()-1));
        QTextCursor tc = edit->textCursor();
        tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, length);
        NavigationHistoryLocator::navigationHistory()->startRecord();
        edit->setTextCursor(tc);

        // center line vertically
        qreal lines = qreal(edit->rect().height()) / edit->cursorRect().height();
        qreal line = qreal(edit->cursorRect().bottom()) / edit->cursorRect().height();

        int mv = int(line - lines/2);
        if (qAbs(mv)+1 > lines / 3) // centeres if the cursor is in upper or lower visible-lines/6
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

syntax::SyntaxHighlighter *FileMeta::highlighter() const
{
    return mHighlighter;
}

void FileMeta::marksChanged(const QSet<int> &lines)
{
    QMutexLocker mx(&mDirtyLinesMutex);
    mDirtyLines.unite(lines);
    if (lines.isEmpty()) mDirtyLinesUpdater.start(0);
    else if (!mDirtyLinesUpdater.isActive()) mDirtyLinesUpdater.start(500);
}

void FileMeta::reloadDelayed()
{
    for (QWidget *wid: std::as_const(mEditors)) {
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
    } else if (kind() == FileKind::Gsp) {
        for (QWidget *wid: mEditors) {
            project::ProjectEdit *prOp = ViewHelper::toProjectEdit(wid);
            if (prOp) return prOp->isModified();
        }
    } else if (kind() == FileKind::Opt || kind() == FileKind::Pf) {
        for (QWidget *wid: mEditors) {
            option::SolverOptionWidget *solverOptionWidget = ViewHelper::toSolverOptionEdit(wid);
            if (solverOptionWidget)
                return solverOptionWidget->isModified();
        }
    } else if (kind() == FileKind::Guc) {
        for (QWidget *wid: mEditors) {
            option::GamsConfigEditor* gucEditor = ViewHelper::toGamsConfigEditor(wid);
            if (gucEditor)
                return gucEditor->isModified();
        }
    } else if (kind() == FileKind::Efi) {
        for (QWidget *wid: mEditors) {
            if (efi::EfiEditor *efi = ViewHelper::toEfiEditor(wid))
                return efi->isModified();
        }
    } else if (kind() == FileKind::GCon) {
        for (QWidget *wid: mEditors) {
            connect::ConnectEditor* gconEditor = ViewHelper::toGamsConnectEditor(wid);
            if (gconEditor)
                return gconEditor->isModified();
        }
    }
    return false;
}

bool FileMeta::isReadOnly() const
{
    AbstractEdit* edit = mEditors.isEmpty() ? nullptr : ViewHelper::toAbstractEdit(mEditors.first());
    if (edit) return edit->isReadOnly();
    if (mForceReadOnly) return true;

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

void FileMeta::setReadOnly(bool readOnly)
{
    if (mForceReadOnly == readOnly) return;
    // This adds a readonly to usually editable editors (currently only for CodeEdit)
    mForceReadOnly = readOnly;
    if (document()) { // There is a text editor
        for (QWidget *wid : editors()) {
            AbstractEdit *aEdit = ViewHelper::toAbstractEdit(wid);
            if (aEdit) {
                aEdit->setReadOnly(mForceReadOnly);
                aEdit->setTextInteractionFlags(aEdit->textInteractionFlags() | Qt::TextSelectableByKeyboard);
            }
        }
    }
}

bool FileMeta::isAutoReload() const
{
    bool autoReload = mAutoReload || mTempAutoReloadTimer.isActive();
    if (!mAutoReload) {
        QStringList autoSuffix = Settings::settings()->toString(skAutoReloadTypes)
                .split(QRegularExpression("\\h*,\\h*"), Qt::SkipEmptyParts);
        QFileInfo fi(mLocation);
        autoReload = autoSuffix.contains(fi.suffix(), Qt::CaseInsensitive);
    }
    return autoReload;
}

void FileMeta::setAutoReload()
{
    mAutoReload = true;
}

void FileMeta::resetTempReloadState()
{
    mTempAutoReloadTimer.start(1500);
}

void FileMeta::setModified(bool modified)
{
    if (document()) {
        document()->setModified(modified);
    } else if (kind() == FileKind::GCon) {
        for (QWidget *e : std::as_const(mEditors)) {
             connect::ConnectEditor *ce = ViewHelper::toGamsConnectEditor(e);
             if (ce) ce->setModified(modified);
        }
    } else if (kind() == FileKind::Opt || kind() == FileKind::Pf) {
          for (QWidget *e : std::as_const(mEditors)) {
               option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(e);
               if (so) so->setModified(modified);
          }
    } else if (kind() == FileKind::Guc) {
        for (QWidget *e : std::as_const(mEditors)) {
             option::GamsConfigEditor *gco = ViewHelper::toGamsConfigEditor(e);
             if (gco) gco->setModified(modified);
        }
    } else if (kind() == FileKind::Efi) {
        for (QWidget *e : std::as_const(mEditors)) {
             if (efi::EfiEditor *efi = ViewHelper::toEfiEditor(e)) efi->setModified(modified);
        }
    }
    emit modifiedChanged(mId, modified);
}

bool FileMeta::isPinnable()
{
    QSet<FileKind> suppressedKinds {FileKind::Guc, FileKind::Opt, FileKind::Pf, FileKind::GCon, };
    return !suppressedKinds.contains(kind());
}

void FileMeta::updateTabName(QTabWidget *tabWidget, int index)
{
    if (kind() == FileKind::Gsp) {
        if (project::ProjectEdit *opt = ViewHelper::toProjectEdit(tabWidget->widget(index)))
            tabWidget->setTabText(index, opt->tabName());
    } else
        tabWidget->setTabText(index, name());
    if (isPinnable())
        tabWidget->setTabToolTip(index, "<p style='white-space:pre'>"+QDir::toNativeSeparators(location()) +
                                 "<br>- Pin right <b>Ctrl+Click</b><br>- Pin below <b>Shift+Ctrl+Click</b></p>");
    else
        tabWidget->setTabToolTip(index, "<p style='white-space:pre'>"+QDir::toNativeSeparators(location())+"</p>");
}

QTextDocument *FileMeta::document() const
{
    return mDocument;
}

void FileMeta::setEncoding(const QString &encoding)
{
    mEncoding = encoding;
}

bool FileMeta::exists(bool ckeckNow) const
{
    if (ckeckNow) return QFileInfo::exists(location());
    return mData.exist;
}

bool FileMeta::isOpen() const
{
    return !mEditors.isEmpty();
}

QWidget* FileMeta::createEdit(QWidget *parent, PExProjectNode *project, const QFont &font, QString encoding, bool forcedAsTextEdit)
{
    QWidget* res = nullptr;
    if (encoding.isEmpty()) encoding = FileMeta::encoding();
    if (encoding.isEmpty()) encoding = Encoding::defaultEncoding();

    if (kind() == FileKind::Gsp) {
        project::ProjectData *sharedData = nullptr;
        project::ProjectEdit *otherProp = nullptr;
        if (editors().size()) {
            otherProp = ViewHelper::toProjectEdit(topEditor());
            sharedData = otherProp->sharedData();
        } else {
            sharedData = new project::ProjectData(project);
        }
        project::ProjectEdit *prop = new project::ProjectEdit(sharedData, parent);
        project->setProjectEditFileMeta(this);
        res = ViewHelper::initEditorType(prop);
    } else if (kind() == FileKind::Gdx) {
        gdxviewer::GdxViewer *gdx = new gdxviewer::GdxViewer(location(), CommonPaths::systemDir(), mEncoding, parent);
        res = ViewHelper::initEditorType(gdx);
        QVariantMap states = Settings::settings()->toMap(skGdxStates);
        if (states.contains(location())) {
            QVariantMap map = states.value(location()).toMap();
            gdx->readState(map);
        }
    } else if (kind() == FileKind::Ref && !forcedAsTextEdit) {
        reference::ReferenceViewer *rv = new reference::ReferenceViewer(location(), mEncoding, parent);
        res = ViewHelper::initEditorType( rv );
        connect(rv, &reference::ReferenceViewer::processState, project, &PExProjectNode::processState);
    } else if (kind() == FileKind::Log) {
        LogParser *parser = new LogParser(mEncoding);
        connect(parser, &LogParser::hasFile, project, &PExProjectNode::hasFile);
        connect(parser, &LogParser::setErrorText, project, &PExProjectNode::setErrorText);
        TextView* tView = ViewHelper::initEditorType(new TextView(TextView::MemoryText, parent), EditorType::log);
        tView->setDebugMode(mFileRepo->debugMode());
        connect(tView, &TextView::hasHRef, project, &PExProjectNode::hasHRef);
        connect(tView, &TextView::jumpToHRef, project, &PExProjectNode::jumpToHRef);
        connect(tView, &TextView::createMarks, project, &PExProjectNode::createMarks);
        connect(tView, &TextView::switchLst, project, &PExProjectNode::switchLst);
        connect(tView, &TextView::registerGeneratedFile, project, &PExProjectNode::registerGeneratedFile);

        tView->setLogParser(parser);
        res = tView;
    } else if (kind() == FileKind::TxtRO || kind() == FileKind::Lst) {
        EditorType type = kind() == FileKind::TxtRO ? EditorType::txtRo : EditorType::lxiLstChild;
        TextView* tView = ViewHelper::initEditorType(new TextView(TextView::FileText, parent), type);
        tView->setDebugMode(mFileRepo->debugMode());
        res = tView;
        tView->loadFile(location(), mEncoding, true);
        if (kind() == FileKind::Lst)
            res = ViewHelper::initEditorType(new lxiviewer::LxiViewer(tView, location(), parent));
    } else if (kind() == FileKind::Guc && !forcedAsTextEdit) {
            // Guc Editor encoding is fixed to UTF-8
            res = ViewHelper::initEditorType(new option::GamsConfigEditor(QFileInfo(name()).completeBaseName(), location(),
                                                                         id(), parent));
    } else if (kind() == FileKind::GCon && !forcedAsTextEdit) {
        res =  ViewHelper::initEditorType(new connect::ConnectEditor(location(), id(), parent));
    } else if ((kind() == FileKind::Opt || kind() == FileKind::Pf) && !forcedAsTextEdit) {
        QFileInfo fileInfo(name());
        support::SolverConfigInfo solverConfigInfo;
        QString defFileName =  kind() == FileKind::Opt ? solverConfigInfo.solverOptDefFileName(fileInfo.baseName())
                                                       : "optgams.def";
        if (!defFileName.isEmpty() && QFileInfo(CommonPaths::systemDir(),defFileName).exists()) {
            res =  ViewHelper::initEditorType(new option::SolverOptionWidget(QFileInfo(name()).completeBaseName(), location(), defFileName, kind(),
                                                                             id(), mEncoding, parent));
        } else if ( QFileInfo(CommonPaths::systemDir(),QString("opt%1.def").arg(fileInfo.baseName().toLower())).exists() &&
                    QString::compare(fileInfo.baseName().toLower(),"gams", Qt::CaseInsensitive)!=0 ) {
            res =  ViewHelper::initEditorType(new option::SolverOptionWidget(QFileInfo(name()).completeBaseName(), location(), QString("opt%1.def").arg(fileInfo.baseName().toLower()), kind(),
                                                                             id(), mEncoding, parent));
        } else {
            SysLogLocator::systemLog()->append(QString("Cannot find  solver option definition file for %1. Open %1 in text editor.").arg(fileInfo.fileName()), LogMsgType::Error);
            forcedAsTextEdit = true;
        }
    } else if (kind() == FileKind::Efi && !forcedAsTextEdit) {
        efi::EfiEditor *ee = ViewHelper::initEditorType(new efi::EfiEditor(parent));
        ee->setWorkingDir(project->workDir());
        ee->setModelName(project->mainModelName());
        ee->load(location());
        res = ee;
    } else {
        forcedAsTextEdit = true;
    }

    if (forcedAsTextEdit) {
        AbstractEdit *edit = nullptr;
        CodeEdit *codeEdit = nullptr;
        codeEdit = new CodeEdit(parent);
        codeEdit->setReadOnly(mForceReadOnly);
        codeEdit->setTextInteractionFlags(codeEdit->textInteractionFlags() | Qt::TextSelectableByKeyboard);

        edit = (kind() == FileKind::Txt || kind() == FileKind::Efi) ? ViewHelper::initEditorType(codeEdit, EditorType::txt)
                                                                    : ViewHelper::initEditorType(codeEdit);
        edit->setLineWrapMode(Settings::settings()->toBool(skEdLineWrapEditor) ? QPlainTextEdit::WidgetWidth
                                                                               : QPlainTextEdit::NoWrap);
        edit->setTabChangesFocus(false);
        res = edit;
        if (kind() == FileKind::Log) {
            edit->setReadOnly(true);
            edit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        } else {
            connect(codeEdit, &CodeEdit::hasHRef, project, &PExProjectNode::hasHRef);
            connect(codeEdit, &CodeEdit::jumpToHRef, project, &PExProjectNode::jumpToHRef);
            connect(codeEdit, &CodeEdit::addBreakpoint, this, [this](int line) {
                PExProjectNode *pro = mFileRepo->projectRepo()->asProject(mProjectId);
                if (!pro) return;
                pro->addBreakpoint(mLocation, line);
                updateBreakpoints();
            });
            connect(codeEdit, &CodeEdit::delBreakpoint, this, [this](int line) {
                PExProjectNode *pro = mFileRepo->projectRepo()->asProject(mProjectId);
                if (!pro) return;
                pro->delBreakpoint(mLocation, line);
                updateBreakpoints();
            });
            connect(codeEdit, &CodeEdit::delAllBreakpoints, this, [this]() {
                PExProjectNode *pro = mFileRepo->projectRepo()->asProject(mProjectId);
                if (!pro) return;
                pro->clearBreakpoints();
                updateBreakpoints();
            });
            connect(codeEdit, &CodeEdit::getProjectHasErrors, this, [this](bool *hasErrors) {
                if (!hasErrors) return;
                *hasErrors = false;
                PExProjectNode *pro = mFileRepo->projectRepo()->asProject(mProjectId);
                if (!pro) return;
                for (PExFileNode *node: pro->listFiles()) {
                    const LineMarks *marks = pro->textMarkRepo()->marks(node->file()->id());
                    if (!marks->isEmpty()) {
                        *hasErrors = true;
                        return;
                    }
                }
            });
            connect(codeEdit, &CodeEdit::delAllProjectErrors, this, [this]() {
                PExProjectNode *pro = mFileRepo->projectRepo()->asProject(mProjectId);
                if (!pro) return;
                pro->clearTextMarks(QSet<TextMark::Type>() << TextMark::error << TextMark::link << TextMark::target);
            });
        }
    }
    setProjectId(project->id());
    ViewHelper::setLocation(res, location());
    addEditor(res);
    res->setFont(font);
    DEB() << "Added editor " << mEditors.count() << " to " << mLocation;
    return res;
}

int FileMeta::addToTab(QTabWidget *tabWidget, QWidget *edit, NewTabStrategy tabStrategy)
{
    int atIndex = tabWidget->count();
    switch (tabStrategy) {
    case tabAtStart: atIndex = 0; break;
    case tabBeforeCurrent: atIndex = tabWidget->currentIndex() < 0 ? 0 : tabWidget->currentIndex(); break;
    case tabAfterCurrent: atIndex = tabWidget->currentIndex()+1; break;
    case tabAtEnd: break;
    }
    int i = tabWidget->insertTab(atIndex, edit, name(NameModifier::editState));
    updateTabName(tabWidget, i);
    if (mEditors.size() == 1 && kind() != FileKind::Log && kind() != FileKind::Gsp && ViewHelper::toAbstractEdit(edit)) {
        try {
            load(mEncoding);
        } catch (Exception &e) {
            if (mEditors.size() > 0) {
                tabWidget->removeTab(tabWidget->currentIndex()+1);
                deleteEdit(mEditors.first());
            }
            e.raise();
        }
    }
    return i;
}

FileMeta::Data::Data(QString location, FileType *knownType)
{
    if (knownType == &FileType::from("-"))
        knownType = nullptr;
    if (location.contains('\\'))
        location = QDir::fromNativeSeparators(location);

    QFileInfo fi(location);
    if (fi.fileName().startsWith('[')) {
        int len = fi.fileName().indexOf(']')-1;
        QString loc = (len > 0) ? fi.fileName().mid(1, len) : "-";
        type = knownType ? knownType : &FileType::from( QString::compare(loc, fi.completeBaseName())==0 ? loc : "-" );
    } else {
        exist = fi.exists();
        size = fi.size();
        created = fi.birthTime();
        modified = fi.lastModified();
        type = (knownType ? knownType : &FileType::from(fi.fileName()));
    }
}

FileMeta::FileDifferences FileMeta::Data::compare(const FileMeta::Data &other)
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
