/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "findadapter.h"
#include "viewhelper.h"
#include "logger.h"
#include "editors/codeedit.h"
#include "editors/textview.h"
#include "lxiviewer/lxiviewer.h"

#include <QTextBrowser>

#ifdef QWEBENGINE
#include <QWebEngineView>
#endif

namespace gams {
namespace studio {
namespace find {

const int  CMaxParallelFind = 1000;

//  -------------------------- FindAdapter

FindAdapter *FindAdapter::createAdapter(QWidget *widget)
{
    FindAdapter *res = nullptr;

    if (CodeEdit *ce = ViewHelper::toCodeEdit(widget))
        res = new EditFindAdapter(ce);

    if (TextView *tv = ViewHelper::toTextView(widget))
        res = new ViewFindAdapter(tv);

    if (lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(widget))
        res = new ViewFindAdapter(lxi->textView());

    if (QTextBrowser *browser = qobject_cast<QTextBrowser*>(widget))
        res = new ChangelogFindAdapter(browser);

#ifdef QWEBENGINE
    if (QWebEngineView *browser = qobject_cast<QWebEngineView*>(widget))
        res = new WebViewFindAdapter(browser);
#endif

    if (res)
        connect(widget, &QWidget::destroyed, res, &FindAdapter::widgetDestroyed);
    return res;
}

QWidget *FindAdapter::widget() const
{
    return nullptr;
}

void FindAdapter::setFocus()
{
    if (widget())
        widget()->setFocus();
}

bool FindAdapter::canReplace() const
{
    return false;
}

void FindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
    Q_UNUSED(rex)
    mCurrentOptions = options;
}

void FindAdapter::findText(const QString &text, FindOptions options)
{
    if (text.isEmpty()) return;

    QString filter;
    QRegularExpression::WildcardConversionOptions opt = QRegularExpression::NonPathWildcardConversion;
    if (!options.testFlag(foExactMatch))
        opt.setFlag(QRegularExpression::UnanchoredWildcardConversion);
    filter = QRegularExpression::wildcardToRegularExpression(text, opt);
    if (options.testFlag(foExactMatch))
        filter = "\\b"+text+"\\b";
    QRegularExpression rex = QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption);
    findText(rex, options);
}

int FindAdapter::findReplaceAll(const QRegularExpression &rex, FindOptions options, const QString &replacement)
{
    Q_UNUSED(rex)
    Q_UNUSED(options)
    Q_UNUSED(replacement)
    return 0;
}

bool FindAdapter::findReplace(const QString &replacement)
{
    Q_UNUSED(replacement)
    return false;
}

void FindAdapter::widgetDestroyed()
{
    delete this;
}

void FindAdapter::stopFind()
{
    mCurrentFindId = -1;
}

void FindAdapter::emitFindDone(bool found)
{
    if (!found)
        invalidateSelection();
    emit findDone(found);
}

QString FindAdapter::lastMatch() const
{
    return mLastMatch;
}

FindOptions FindAdapter::currentOptions() const
{
    return mCurrentOptions;
}

void FindAdapter::setLastMatch(const QString &text)
{
    mLastMatch = text;
}

FindAdapter::FindAdapter(QWidget *widget)
    : QObject{widget}
{

}

QTextDocument::FindFlags FindAdapter::findFlags(FindOptions options)
{
    QTextDocument::FindFlags res = QTextDocument::FindFlags();
    if (options.testFlag(foBackwards))
        res |= QTextDocument::FindBackward;
    if (options.testFlag(foExactMatch))
        res |= QTextDocument::FindWholeWords;
    if (options.testFlag(foCaseSense))
        res |= QTextDocument::FindCaseSensitively;
    return res;
}

//  -------------------------- EditFindAdapter

EditFindAdapter::EditFindAdapter(CodeEdit *edit)
    : FindAdapter(edit), mEdit(edit)
{
    qRegisterMetaType<FindResult>("FindResult");
    mWorkerThread = new QThread(this);
    mWorker = new FindWorker();
    mWorker->moveToThread(mWorkerThread);

    connect(this, &EditFindAdapter::requestSearch, mWorker, &FindWorker::findText);
    connect(mWorker, &FindWorker::done, this, &EditFindAdapter::handleNextResult);
    connect(mWorkerThread, &QThread::finished, mWorker, &QObject::deleteLater);

    connect(edit, &CodeEdit::allowReplaceChanged, this, &FindAdapter::allowReplaceChanged);
    connect(edit, &CodeEdit::endFind, this, &FindAdapter::endFind);
    edit->updateExtraSelections();
    mWorkerThread->start();
}

EditFindAdapter::~EditFindAdapter()
{
    mWorker->activeFindId.store(-1);
    disconnect(mWorker, &FindWorker::done, this, &EditFindAdapter::handleNextResult);
    mWorkerThread->quit();
    if (!mWorkerThread->wait(500)) {
        mWorkerThread->terminate();
        mWorkerThread->wait();
    }
}

void EditFindAdapter::emitFindDone(bool found)
{
    FindAdapter::findDone(found);
    mEdit->updateFindScrollMarkers(true);
}

void EditFindAdapter::handleNextResult(const FindResult &res)
{
    if (res.aborted || res.id != mCurrentFindId) return;

    if (res.success) {
        QTextCursor c(mEdit->document());
        c.setPosition(res.pos);
        c.setPosition(res.pos + res.len, QTextCursor::KeepAnchor);
        mEdit->setTextCursor(c);
        mEdit->ensureCursorVisible();
        if (res.wrapped) emit showStatusMessage(tr("Suche am Anfang/Ende fortgesetzt"));
        emitFindDone(true);
    } else {
        emitFindDone(false);
    }
}

void EditFindAdapter::updateCache()
{
    int rev = mEdit->document()->revision();
    if (rev != mCachedRevision) {
        mCachedText = mEdit->document()->toPlainText();
        mCachedRevision = rev;
    }
}

QWidget *EditFindAdapter::widget() const
{
    return mEdit;
}

bool EditFindAdapter::canReplace() const
{
    return mEdit && !mEdit->isReadOnly();
}

bool EditFindAdapter::hasSelectedFind() const
{
    return mEdit->hasSelectedFind();
}

void EditFindAdapter::setFindTerm(const QRegularExpression &rex, FindOptions options)
{
    mEdit->setFindTerm(rex, findFlags(options));
    emitFindDone(false);
}

bool EditFindAdapter::hasFindTerm()
{
    return mEdit->findTerm();
}

void EditFindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
    mCurrentFindId = (mCurrentFindId + 1) % CMaxParallelFind;
    mWorker->activeFindId.store(mCurrentFindId);
    updateCache();

    QRegularExpression::PatternOptions patternOptions = rex.patternOptions() | QRegularExpression::UseUnicodePropertiesOption;
    if (!options.testFlag(foCaseSense))
         patternOptions |=  QRegularExpression::CaseInsensitiveOption;
    else patternOptions &= ~QRegularExpression::CaseInsensitiveOption;
    QRegularExpression finalRex = rex;
    finalRex.setPatternOptions(patternOptions);
    if (options.testFlag(foExactMatch)) {
        QString pattern = finalRex.pattern();
        if (!pattern.startsWith(QLatin1String("\\b")) && !pattern.endsWith(QLatin1String("\\b"))) {
            finalRex.setPattern(QStringLiteral("\\b%1\\b").arg(pattern));
        }
    }
    QTextDocument::FindFlags qtOptions = findFlags(options);
    mEdit->setFindTerm(finalRex, qtOptions);
    bool backward = options.testFlag(foBackwards);
    QTextCursor c = mEdit->textCursor();
    int startPos = options.testFlag(foContinued) ? (backward ? c.selectionStart() - 1 : c.selectionEnd())
                                                 : c.selectionStart();
    emit requestSearch(mCachedText, finalRex, qBound(0, startPos, mCachedText.length()), backward, mCurrentFindId);
}

bool EditFindAdapter::findReplace(const QString &replacement)
{
    if (!mEdit->findReplace(replacement))
        return false;
    QTextCursor cursor = mEdit->textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, replacement.length());
    mEdit->setTextCursor(cursor);
    return true;
}

int EditFindAdapter::findReplaceAll(const QRegularExpression &rex, FindOptions options, const QString &replacement)
{
    return mEdit->findReplaceAll(rex, findFlags(options), replacement);
}

QString EditFindAdapter::currentFindSelection(bool &isCurrentWord)
{
    return mEdit->currentFindSelection(false, isCurrentWord);
}

void EditFindAdapter::invalidateSelection()
{
    mEdit->clearSelectedFind();
    QTextCursor cur = mEdit->textCursor();
    if (cur.hasSelection()) {
        cur.setPosition(cur.anchor());
        mEdit->setTextCursor(cur);
    }
}


// -------------------------- ViewFindAdapter

ViewFindAdapter::ViewFindAdapter(TextView *view)
    : FindAdapter(view), mView(view)
{
    CodeEdit* edit = static_cast<CodeEdit*>(view->edit());
    connect(edit, &CodeEdit::endFind, this, &FindAdapter::endFind);
    view->updateExtraSelections();
}

ViewFindAdapter::~ViewFindAdapter()
{}

QWidget *ViewFindAdapter::widget() const
{
    return mView;
}

bool ViewFindAdapter::hasSelectedFind() const
{
    return mView->hasSelectedFind();
}

void ViewFindAdapter::setFindTerm(const QRegularExpression &rex, FindOptions options)
{
    mView->setFindTerm(rex, findFlags(options));
}

bool ViewFindAdapter::hasFindTerm()
{
    return static_cast<CodeEdit*>(mView->edit())->findTerm();
}

void ViewFindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
    FindAdapter::findText(rex, options);
    if (rex.pattern().isEmpty()) {
        mView->setFindTerm(rex, findFlags(options));
        emitFindDone(false);
    }
    if (!options.testFlag(foContinued) && mView->anchor().y() == mView->position().y()) {
        QPoint pos = mView->position();
        QPoint anc = mView->anchor();
        if (pos.x() < anc.x())
            qSwap(pos, anc);
        if (!options.testFlag(foBackwards))
            qSwap(pos, anc);
        mView->jumpTo(pos.y(), pos.x());
    }
    bool res = mView->findText(rex, findFlags(options), options.testFlag(foContinued));
    if (!res && options.testFlag(foBackwards)) {
        mView->jumpToEnd();
        res = mView->findText(rex, findFlags(options), options.testFlag(foContinued));
    }
    emitFindDone(res);
}

QString ViewFindAdapter::currentFindSelection(bool &isCurrentWord)
{
    return mView->currentFindSelection(isCurrentWord);
}

void ViewFindAdapter::invalidateSelection()
{
    mView->clearSelectedFind();
}


// -------------------------- ChangelogFindAdapter

ChangelogFindAdapter::ChangelogFindAdapter(QTextBrowser *view)
    : FindAdapter(view), mView(view)
{
    // add handler for endFind (to close on ESC), F3, and Shift+F3
    mView->installEventFilter(this);
    mView->viewport()->installEventFilter(this);

    connect(mView, &QTextBrowser::selectionChanged, this, [this](){
        mSelection = mTakeSelection ? mView->textCursor().selectedText() : QString();
        mTakeSelection = false;
    });

    connect(mView->verticalScrollBar(), &QScrollBar::sliderMoved, this, [this](){
        calcExtraSelections();
    });
}

ChangelogFindAdapter::~ChangelogFindAdapter()
{
    mView->removeEventFilter(this);
    mView->viewport()->removeEventFilter(this);
}

QWidget *ChangelogFindAdapter::widget() const
{
    return mView;
}

bool ChangelogFindAdapter::hasSelectedFind() const
{
    return !mSelection.isEmpty();
}

void ChangelogFindAdapter::setFindTerm(const QRegularExpression &rex, FindOptions options)
{
    if (mRex)
        delete mRex;
    mRex = rex.isValid() && !rex.pattern().isEmpty() ? new QRegularExpression(rex) : nullptr;
    mOptions = options;

    calcExtraSelections();
}

bool ChangelogFindAdapter::hasFindTerm()
{
    return mRex && mRex->isValid() && !mRex->pattern().isEmpty();
}

void ChangelogFindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
    FindAdapter::findText(rex, options);
    if (rex.pattern().isEmpty()) {
        invalidateSelection();
        setFindTerm(rex, options);
        QTextCursor cur = mView->textCursor();
        cur.clearSelection();
        mView->setTextCursor(cur);
        emitFindDone(false);
        return;
    }
    int pos = mView->textCursor().hasSelection() ? mView->textCursor().anchor()
                                                 : mView->textCursor().position();
    QTextDocument::FindFlags docOpt = findFlags(options);
    if (options.testFlag(foContinued))
        pos += docOpt.testFlag(QTextDocument::FindBackward) ? -1 : 1;
    QTextCursor cur = mView->textCursor();
    if (!options.testFlag(foSkipFind))
        cur = mView->document()->find(rex, pos, docOpt);
    if (cur.isNull()) {
        pos = docOpt.testFlag(QTextDocument::FindBackward) ? mView->document()->characterCount()-1 : 0;
        cur = mView->document()->find(rex, pos, docOpt);
    }
    if (cur.isNull()) {
        mView->textCursor().clearSelection();
    } else {
        mTakeSelection = true;
        mView->setTextCursor(cur);
    }
    setFindTerm(rex, options);
    emitFindDone(!cur.isNull());
}

QString ChangelogFindAdapter::currentFindSelection(bool &isCurrentWord)
{
    isCurrentWord = false;
    if (!mSelection.isEmpty())
        return mSelection;

    if (!mView->textCursor().hasSelection()) {
        QTextCursor cur = mView->textCursor();
        cur.movePosition(QTextCursor::EndOfWord);
        cur.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
        if (mView->textCursor().position() >= cur.position() && mView->textCursor().position() <= cur.anchor()) {
            mTakeSelection = true;
            isCurrentWord = true;
            mView->setTextCursor(cur);
        }
    }
    if (mView->textCursor().hasSelection()) {
        QTextCursor anc = mView->textCursor();
        anc.setPosition(anc.anchor());
        if (mView->textCursor().blockNumber() == anc.blockNumber())
            return mView->textCursor().selectedText();
    }
    return QString();
}

void ChangelogFindAdapter::invalidateSelection()
{
    mSelection.clear();
}

void ChangelogFindAdapter::calcExtraSelections()
{
    if (!hasFindTerm()) {
        mView->setExtraSelections({});
        return;
    }

    // calculate the extraSelections
    QTextCursor curFrom = mView->cursorForPosition({0,0});
    QTextCursor curTo = mView->cursorForPosition({mView->viewport()->size().width(),
                                                  mView->viewport()->size().height()});
    QList<QTextEdit::ExtraSelection> selections;
    QTextBlock block = curFrom.block();
    while (block.isValid() && block.blockNumber() <= curTo.blockNumber()) {
        QRegularExpressionMatchIterator i = mRex->globalMatch(block.text());

        while (i.hasNext()) {
            QRegularExpressionMatch m = i.next();
            QTextEdit::ExtraSelection selection;
            QTextCursor tc(mView->document());
            tc.setPosition(block.position() + int(m.capturedStart(0)));
            tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, int(m.capturedLength(0)));

            selection.cursor = tc;
            selection.format.setForeground(Qt::white);
            selection.format.setBackground(toColor(Theme::Edit_findBg));
            selections << selection;
        }

        block = block.next();
    }
    mView->setExtraSelections(selections);
}

bool ChangelogFindAdapter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape)
            emit endFind();
        else if (keyEvent->key() == Qt::Key_F3) {
            if (keyEvent->modifiers().testFlag(Qt::ShiftModifier))
                emit findNext(true);
            else
                emit findNext(false);
        }
    }
    return FindAdapter::eventFilter(watched, event);
}


#ifdef QWEBENGINE
// -------------------------- ChangelogFindAdapter

WebViewFindAdapter::WebViewFindAdapter(QWebEngineView *view)
{
    mView = view;
    mView->parent()->installEventFilter(this);
}

WebViewFindAdapter::~WebViewFindAdapter()
{
    mView->parent()->removeEventFilter(this);
}

QWidget *WebViewFindAdapter::widget() const
{
    return mView;
}

bool WebViewFindAdapter::hasSelectedFind() const
{
    return false;
}

void WebViewFindAdapter::setFindTerm(const QRegularExpression &rex, FindOptions options)
{
    Q_UNUSED(rex)
    Q_UNUSED(options)
    DEB() << "Regular Expression not supported in WebEngineView";
}

bool WebViewFindAdapter::hasFindTerm()
{
    return false;
}

void WebViewFindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
    Q_UNUSED(rex)
    Q_UNUSED(options)
    DEB() << "Regular Expression not supported in WebEngineView";
    emitFindDone(false);
}

void WebViewFindAdapter::findText(const QString &text, FindOptions options)
{
    FindAdapter::findText(text, options);
    auto resFunc = std::function<void(const QWebEngineFindTextResult &)>();
    QWebEnginePage::FindFlags opt = {};
    if (options.testFlag(foBackwards)) opt.setFlag(QWebEnginePage::FindBackward);
    if (options.testFlag(foCaseSense)) opt.setFlag(QWebEnginePage::FindCaseSensitively);
    mView->findText(text, opt);
    if (text.isEmpty())
        return emitFindDone(false);
    return emitFindDone(true);
}

QString WebViewFindAdapter::currentFindSelection(bool &isCurrentWord)
{
    Q_UNUSED(isCurrentWord)
    QString sel = mView->selectedText();
    if (sel.contains('\n'))
        sel = QString();
    return sel;
}

void WebViewFindAdapter::invalidateSelection()
{
    mView->findText(QString(), {});
}

bool WebViewFindAdapter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape)
            emit endFind();
        else if (keyEvent->key() == Qt::Key_F3) {
            if (keyEvent->modifiers().testFlag(Qt::ShiftModifier))
                emit findNext(true);
            else
                emit findNext(false);
        }
    }
    return FindAdapter::eventFilter(watched, event);
}


#endif

} // namespace find
} // namespace studio
} // namespace gams
