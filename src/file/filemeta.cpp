/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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

#include <QTabWidget>
#include <QFileInfo>
#include <QFile>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>
#include <QScrollBar>
#include <QApplication>

namespace gams {
namespace studio {

FileMeta::FileMeta(FileMetaRepo *fileRepo, FileId id, QString location, FileType *knownType)
    : mId(id), mFileRepo(fileRepo), mData(Data(location, knownType))
{
    if (!mFileRepo) EXCEPT() << "FileMetaRepo  must not be null";
    mCodec = QTextCodec::codecForMib(Settings::settings()->toInt(skDefaultCodecMib));
    if (!mCodec) mCodec = QTextCodec::codecForLocale();
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
        for (QWidget*wid: qAsConst(mEditors)) {
            ViewHelper::setLocation(wid, location);
        }
        mAutoReload = mData.type->autoReload();
    }
}

bool FileMeta::hasExistingFile(QList<QUrl> urls)
{
    for (const QUrl &url : urls) {
        if (url.isEmpty()) continue;
        QFileInfo fi(url.toLocalFile());
        if (fi.isFile() && fi.exists()) return true;
    }
    return false;
}

bool FileMeta::hasExistingFolder(QList<QUrl> urls) {
    for (const QUrl &url : urls) {
        if (url.isEmpty()) continue;
        QDir d(url.toLocalFile());
        if (d.exists()) return true;
    }
    return false;
}

QStringList FileMeta::pathList(QList<QUrl> urls)
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
        for (QWidget *wid: qAsConst(mEditors)) {
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
    for (QWidget* widget: qAsConst(mEditors)) {
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
    for (QWidget* widget: qAsConst(mEditors)) {
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
            for (QWidget *edit : qAsConst(mEditors)) {
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
            for (QWidget *edit : qAsConst(mEditors)) {
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
    mData.type = &FileType::from(kind);
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

FontGroup FileMeta::fontGroup()
{
    const QSet<FileKind> editKind {FileKind::Gms, FileKind::Lst, FileKind::Lxi, FileKind::None, FileKind::Txt, FileKind::TxtRO};
    if (kind() == FileKind::Log) return FontGroup::fgLog;
    if (editKind.contains(kind())) return FontGroup::fgText;
    if (document()) return FontGroup::fgText;
    return FontGroup::fgTable;
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
    Q_UNUSED(modiState)
    if (!modiState && kind() == FileKind::PrO) {
        if (project::ProjectOptions *pro = ViewHelper::toProjectOptions(topEditor())) {
            mName = '['+pro->sharedData()->fieldData(project::ProjectData::name)+']';
        }
    }
    emit changed(id());
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
    for (QWidget *w: qAsConst(mEditors)) {
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
    if (kind() != FileKind::PrO)
        load(mCodec->mibEnum(), false);
}

void FileMeta::updateView()
{
    for (QWidget *wid: qAsConst(mEditors)) {
        if (TextView* tv = ViewHelper::toTextView(wid)) {
            tv->recalcVisibleLines();
        }
    }
}

void FileMeta::updateSyntaxColors()
{
    if (mHighlighter) {
        mHighlighter->reloadColors();
        for (QWidget *w: qAsConst(mEditors)) {
            if (ViewHelper::toCodeEdit(w)) {
                mHighlighter->rehighlight();
            }
        }
    }

}

void FileMeta::initEditorColors()
{
    for (QWidget *w: qAsConst(mEditors)) {
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
            ed->setPalette(QPalette());
        } else if (ed->palette().windowText().color() != Theme::color(Theme::Edit_text) ||
                ed->palette().window().color() != Theme::color(Theme::Edit_background)) {
            ed->setAutoFillBackground(true);
            QPalette pal = ed->palette();
            pal.setColor(QPalette::Text, Theme::color(Theme::Edit_text));
            pal.setColor(QPalette::Base, Theme::color(Theme::Edit_background));
            ed->setPalette(pal);
        }
    }
}

void FileMeta::updateEditorColors()
{
    initEditorColors();
    for (QWidget *w: qAsConst(mEditors)) {
        if (AbstractEdit *ce = ViewHelper::toAbstractEdit(w))
            ce->updateExtraSelections();
        if (CodeEdit *ce = ViewHelper::toCodeEdit(w))
            ce->lineNumberArea()->update();
        if (TextView *tv = ViewHelper::toTextView(w))
            tv->updateTheme();
    }
}

void FileMeta::invalidateTheme()
{
    updateEditorColors();
    updateSyntaxColors();
}

bool FileMeta::eventFilter(QObject *sender, QEvent *event)
{
    if (event->type() == QEvent::ApplicationFontChange || event->type() == QEvent::FontChange) {
        const QFont *f = nullptr;
        QList<QWidget*> syncEdits;
        for (QWidget *wid : mEditors) {
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
            for (QWidget *wid : syncEdits) {
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
        }

        if (!aEdit->viewport()->hasMouseTracking())
            aEdit->viewport()->setMouseTracking(true);
    }

    if (TextView* tv = ViewHelper::toTextView(edit)) {
        connect(tv->edit(), &AbstractEdit::requestLstTexts, mFileRepo->projectRepo(), &ProjectRepo::errorTexts);
        connect(tv->edit(), &AbstractEdit::toggleBookmark, mFileRepo, &FileMetaRepo::toggleBookmark);
        connect(tv->edit(), &AbstractEdit::jumpToNextBookmark, mFileRepo, &FileMetaRepo::jumpToNextBookmark);
        connect(tv->edit(), &AbstractEdit::zoomRequest, this, &FileMeta::zoomRequest);
        if (lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(edit))
            connect(lxi, &lxiviewer::LxiViewer::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
        else
            connect(tv, &TextView::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
        if (aEdit)
            disconnect(aEdit, &AbstractEdit::scrolled, mFileRepo, &FileMetaRepo::scrollSynchronize);
        if (tv->kind() == TextView::FileText)
            tv->setMarks(mFileRepo->textMarkRepo()->marks(mId));
    } else if (project::ProjectOptions* prOp = ViewHelper::toProjectOptions(edit)) {
        connect(prOp, &project::ProjectOptions::modificationChanged, this, &FileMeta::modificationChanged);
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

const NodeId &FileMeta::projectId() const
{
    return mProjectId;
}

void FileMeta::setProjectId(const NodeId &newProjectId)
{
    mProjectId = newProjectId;
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
    } else if (project::ProjectOptions* prOp = ViewHelper::toProjectOptions(edit)) {
       disconnect(prOp, &project::ProjectOptions::modificationChanged, this, &FileMeta::modificationChanged);
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

    if (mEditors.isEmpty()) {
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
}

bool FileMeta::hasEditor(QWidget * const &edit) const
{
    return mEditors.contains(edit);
}

void FileMeta::load(int codecMib, bool init)
{
    if (codecMib == -1) {
        codecMib = Settings::settings()->toInt(skDefaultCodecMib);
    }
    mCodec = QTextCodec::codecForMib(codecMib);
    mData = Data(location(), mData.type);

    if (kind() == FileKind::Gdx) {
        for (QWidget *wid: qAsConst(mEditors)) {
            if (gdxviewer::GdxViewer *gdxViewer = ViewHelper::toGdxViewer(wid)) {
                gdxViewer->setHasChanged(init);
                gdxViewer->reload(mCodec);
            }
        }
        return;
    }
    if (kind() == FileKind::TxtRO || kind() == FileKind::Lst) {
        for (QWidget *wid: qAsConst(mEditors)) {
            if (TextView *tView = ViewHelper::toTextView(wid)) {
                tView->loadFile(location(), codecMib, init);
            }
            if (kind() == FileKind::Lst) {
                lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(wid);
                if (lxi) lxi->loadLxi();
            }
        }
        return;
    }
    if (kind() == FileKind::Ref) {
        for (QWidget *wid: qAsConst(mEditors)) {
            reference::ReferenceViewer *refViewer = ViewHelper::toReferenceViewer(wid);
            if (refViewer) refViewer->on_referenceFileChanged(mCodec);
        }
        return;
    }
    if (kind() == FileKind::Opt) {
        bool textOptEditor = true;
        for (QWidget *wid : qAsConst(mEditors)) {
            if (option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(wid)) {
                textOptEditor = false;
                so->on_reloadSolverOptionFile(mCodec);
            }
        }
        if (!textOptEditor)
            return;
    }
    if (kind() == FileKind::GCon) {
        bool textEditor = true;
        for (QWidget *wid : qAsConst(mEditors)) {
            if (connect::ConnectEditor *cyaml = ViewHelper::toGamsConnectEditor(wid)) {
                textEditor = false;
                cyaml->on_reloadConnectFile(mCodec);
            }
        }
        if (!textEditor)
            return;
    }
    if (kind() == FileKind::Guc) {
        bool textOptEditor = true;
        for (QWidget *wid : qAsConst(mEditors)) {
            if (option::GamsConfigEditor *cfge = ViewHelper::toGamsConfigEditor(wid)) {
                textOptEditor = false;
                cfge->on_reloadGamsUserConfigFile(mCodec);
            }
        }
        if (!textOptEditor)
            return;
    }
    if (kind() == FileKind::Efi) {
        bool textOptEditor = true;
        for (QWidget *wid : qAsConst(mEditors)) {
            if (efi::EfiEditor *ee = ViewHelper::toEfiEditor(wid)) {
                textOptEditor = false;
                ee->load(location());
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
        QString invalidCodecs;
        QTextCodec::ConverterState state;
        QTextCodec *codec = QTextCodec::codecForMib(codecMib);
        if (codec) {
            QString text = codec->toUnicode(data.constData(), data.size(), &state);
            if (state.invalidChars != 0) {
                invalidCodecs += (invalidCodecs.isEmpty() ? "" : ", ") + codec->name();
            }
            QVector<QPoint> edPos = getEditPositions();
            mLoading = true;
            if (mHighlighter) mHighlighter->pause();
            document()->setPlainText(text);
            if (mHighlighter) mHighlighter->resume();
            setEditPositions(edPos);
            mLoading = false;
            mCodec = codec;
            if (!invalidCodecs.isEmpty()) {
                DEB() << " can't be encoded to " + invalidCodecs + ". Encoding used: " + codec->name();
            }
        } else {
            SysLogLocator::systemLog()->append("System doesn't contain codec for MIB "
                                               + QString::number(codecMib), LogMsgType::Info);
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

    mFileRepo->unwatch(this);
    if (document()) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            EXCEPT() << "Can't save " << location;
        QTextStream out(&file);
        if (mCodec) out.setCodec(mCodec);
        out << document()->toPlainText();
        out.flush();
        file.close();
    } else if (kind() == FileKind::PrO) {
        project::ProjectOptions* prOp = ViewHelper::toProjectOptions(mEditors.first());
        if (prOp) prOp->save();
    } else if (kind() == FileKind::Opt) {
        option::SolverOptionWidget* solverOptionWidget = ViewHelper::toSolverOptionEdit( mEditors.first() );
        if (solverOptionWidget) solverOptionWidget->saveOptionFile(location);

    } else if (kind() == FileKind::Guc) {
        option::GamsConfigEditor* gucEditor = ViewHelper::toGamsConfigEditor( mEditors.first() );
        if (gucEditor) gucEditor->saveConfigFile(location);
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
            efi->save(location);
        }
    } else if (kind() == FileKind::GCon) {
        connect::ConnectEditor* gconEditor = ViewHelper::toGamsConnectEditor( mEditors.first() );
        if (gconEditor) gconEditor->saveConnectFile(location);

    } else { // no document, e.g. lst
        QFile old(mLocation);
        if (file.exists()) QFile::remove(file.fileName());

        if (!old.copy(location)) EXCEPT() << "Can't save " << location;
    }
    setLocation(location);
    refreshType();
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

bool FileMeta::refreshMetaData()
{
    Data data(location(), mData.type);
    bool res = (mData.compare(data) != FdEqual);
    mData = data;
    return res;
}

void FileMeta::jumpTo(NodeId groupId, bool focus, int line, int column, int length)
{
    mFileRepo->openFile(this, groupId, focus, codecMib());
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

void FileMeta::marksChanged(QSet<int> lines)
{
    QMutexLocker mx(&mDirtyLinesMutex);
    mDirtyLines.unite(lines);
    if (lines.isEmpty()) mDirtyLinesUpdater.start(0);
    else if (!mDirtyLinesUpdater.isActive()) mDirtyLinesUpdater.start(500);
}

void FileMeta::reloadDelayed()
{
    for (QWidget *wid: qAsConst(mEditors)) {
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
    } else if (kind() == FileKind::PrO) {
        for (QWidget *wid: mEditors) {
            project::ProjectOptions *prOp = ViewHelper::toProjectOptions(wid);
            if (prOp) return prOp->isModified();
        }
    } else if (kind() == FileKind::Opt) {
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
        for (QWidget *e : qAsConst(mEditors)) {
             connect::ConnectEditor *ce = ViewHelper::toGamsConnectEditor(e);
             if (ce) ce->setModified(modified);
        }
    } else if (kind() == FileKind::Opt) {
          for (QWidget *e : qAsConst(mEditors)) {
               option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(e);
               if (so) so->setModified(modified);
          }
    } else if (kind() == FileKind::Guc) {
        for (QWidget *e : qAsConst(mEditors)) {
             option::GamsConfigEditor *gco = ViewHelper::toGamsConfigEditor(e);
             if (gco) gco->setModified(modified);
        }
    } else if (kind() == FileKind::Efi) {
        for (QWidget *e : qAsConst(mEditors)) {
             if (efi::EfiEditor *efi = ViewHelper::toEfiEditor(e)) efi->setModified(modified);
        }
    }
}

bool FileMeta::isPinnable()
{
    QSet<FileKind> suppressedKinds {FileKind::Guc, FileKind::Opt, FileKind::GCon, };
    return !suppressedKinds.contains(kind());
}

void FileMeta::updateTabName(QTabWidget *tabWidget, int index)
{
    tabWidget->setTabText(index, name(NameModifier::raw));
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

int FileMeta::codecMib() const
{
    return mCodec ? mCodec->mibEnum() : Settings::settings()->toInt(skDefaultCodecMib);
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
            mCodec = QTextCodec::codecForMib(Settings::settings()->toInt(skDefaultCodecMib));
            DEB() << "Encoding parameter invalid, initialized to " << mCodec->name();
        } else {
            DEB() << "Encoding parameter invalid, left unchanged at " << mCodec->name();
        }
    } else {
        mCodec = codec;
    }
    for (QWidget *wid: qAsConst(mEditors)) {
        if (TextView *tv = ViewHelper::toTextView(wid)) {
            if (tv->logParser())
                tv->logParser()->setCodec(mCodec);
        }
    }
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

QWidget* FileMeta::createEdit(QWidget *parent, PExProjectNode *project, int codecMib, bool forcedAsTextEdit)
{
    QWidget* res = nullptr;
    if (codecMib == -1) codecMib = FileMeta::codecMib();
    if (codecMib == -1) codecMib = Settings::settings()->toInt(skDefaultCodecMib);
    mCodec = QTextCodec::codecForMib(codecMib);
    if (kind() == FileKind::PrO) {
        project::ProjectData *sharedData = nullptr;
        project::ProjectOptions *otherProp = nullptr;
        if (editors().size()) {
            otherProp = ViewHelper::toProjectOptions(topEditor());
            sharedData = otherProp->sharedData();
        } else {
            sharedData = new project::ProjectData(project);
        }
        project::ProjectOptions *prop = new project::ProjectOptions(sharedData, parent);
        res = ViewHelper::initEditorType(prop);
    } else if (kind() == FileKind::Gdx) {
        gdxviewer::GdxViewer *gdx = new gdxviewer::GdxViewer(location(), CommonPaths::systemDir(), mCodec, parent);
        res = ViewHelper::initEditorType(gdx);
        QVariantMap states = Settings::settings()->toMap(skGdxStates);
        if (states.contains(location())) {
            QVariantMap map = states.value(location()).toMap();
            gdx->readState(map);
        }
    } else if (kind() == FileKind::Ref && !forcedAsTextEdit) {
        res = ViewHelper::initEditorType(new reference::ReferenceViewer(location(), mCodec, parent));
    } else if (kind() == FileKind::Log) {
        LogParser *parser = new LogParser(mCodec);
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
        tView->loadFile(location(), codecMib, true);
        if (kind() == FileKind::Lst)
            res = ViewHelper::initEditorType(new lxiviewer::LxiViewer(tView, location(), parent));
    } else if (kind() == FileKind::Guc && !forcedAsTextEdit) {
            // Guc Editor ignore other encoding scheme than UTF-8
            mCodec = QTextCodec::codecForName("utf-8");
            res = ViewHelper::initEditorType(new option::GamsConfigEditor( QFileInfo(name()).completeBaseName(), location(),
                                                                         id(), parent));
    } else if (kind() == FileKind::GCon && !forcedAsTextEdit) {
        res =  ViewHelper::initEditorType(new connect::ConnectEditor(location(), id(), mCodec, parent ));
    } else if (kind() == FileKind::Opt && !forcedAsTextEdit) {
        QFileInfo fileInfo(name());
        support::SolverConfigInfo solverConfigInfo;
        QString defFileName = solverConfigInfo.solverOptDefFileName(fileInfo.baseName());
        if (!defFileName.isEmpty() && QFileInfo(CommonPaths::systemDir(),defFileName).exists()) {
            res =  ViewHelper::initEditorType(new option::SolverOptionWidget(QFileInfo(name()).completeBaseName(), location(), defFileName,
                                                                             id(), mCodec, parent));
        } else if ( QFileInfo(CommonPaths::systemDir(),QString("opt%1.def").arg(fileInfo.baseName().toLower())).exists() &&
                    QString::compare(fileInfo.baseName().toLower(),"gams", Qt::CaseInsensitive)!=0 ) {
            res =  ViewHelper::initEditorType(new option::SolverOptionWidget(QFileInfo(name()).completeBaseName(), location(), QString("opt%1.def").arg(fileInfo.baseName().toLower()),
                                                                             id(), mCodec, parent));
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
        }
    }
    setProjectId(project->id());
    ViewHelper::setLocation(res, location());

    addEditor(res);
    return res;
}

int FileMeta::addToTab(QTabWidget *tabWidget, QWidget *edit, int codecMib, NewTabStrategy tabStrategy)
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
    if (mEditors.size() == 1 && kind() != FileKind::Log && kind() != FileKind::PrO && ViewHelper::toAbstractEdit(edit)) {
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
        if (type->kind() == FileKind::PrO) {
            if (fi.exists() && fi.isFile()) {
                type = &FileType::from(FileKind::Txt);
            }
        }
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
