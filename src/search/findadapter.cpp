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

#include <QTextBrowser>

namespace gams {
namespace studio {
namespace find {

//  -------------------------- FindAdapter

FindAdapter *FindAdapter::createAdapter(QWidget *widget)
{
    FindAdapter *res = nullptr;

    if (CodeEdit *ce = ViewHelper::toCodeEdit(widget))
        res = new EditFindAdapter(ce);

    if (TextView *tv = ViewHelper::toTextView(widget))
        res = new ViewFindAdapter(tv);

    if (QTextBrowser *browser = qobject_cast<QTextBrowser*>(widget))
        res = new ChangelogFindAdapter(browser);

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

int FindAdapter::findReplaceAll(const QRegularExpression &rex, FindOptions options, const QString &replacement)
{
    return 0;
}

bool FindAdapter::findReplace(const QRegularExpression &rex, FindOptions options, const QString &replacement)
{
    return false;
}

void FindAdapter::widgetDestroyed()
{
    delete this;
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
    connect(edit, &CodeEdit::allowReplaceChanged, this, &FindAdapter::allowReplaceChanged);
    connect(edit, &CodeEdit::endFind, this, &FindAdapter::endFind);
    edit->updateExtraSelections();
}

EditFindAdapter::~EditFindAdapter()
{}

QWidget *EditFindAdapter::widget() const
{
    return mEdit;
}

bool EditFindAdapter::canReplace() const
{
    return mEdit && !mEdit->isReadOnly();
}

bool EditFindAdapter::hasSelection() const
{
    return mEdit->hasSelectedFind();
}

void EditFindAdapter::setFindTerm(const QRegularExpression &rex, FindOptions options)
{
    mEdit->setFindTerm(rex, findFlags(options));
}

bool EditFindAdapter::hasFindTerm()
{
    return mEdit->hasFindTerm();
}

bool EditFindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
    return mEdit->findText(rex, findFlags(options), options.testFlag(foContinued));
}

bool EditFindAdapter::findReplace(const QRegularExpression &rex, FindOptions options, const QString &replacement)
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
    return mEdit->currentFindSelection(false);
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

bool ViewFindAdapter::hasSelection() const
{
    return mView->hasSelectedFind();
}

void ViewFindAdapter::setFindTerm(const QRegularExpression &rex, FindOptions options)
{
    mView->setFindTerm(rex, findFlags(options));
}

bool ViewFindAdapter::hasFindTerm()
{
    return static_cast<CodeEdit*>(mView->edit())->hasFindTerm();
}

bool ViewFindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
    if (!options.testFlag(foContinued)) {
        QPoint absPos = options.testFlag(foBackwards) ? mView->anchor() : mView->position();
        mView->jumpTo(absPos.y(), absPos.x());
    }
    return mView->findText(rex, findFlags(options), options.testFlag(foContinued));
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
{}

QWidget *ChangelogFindAdapter::widget() const
{
    return mView;
}

bool ChangelogFindAdapter::hasSelection() const
{
    return !mSelection.isEmpty();
}

void ChangelogFindAdapter::setFindTerm(const QRegularExpression &rex, FindOptions options)
{
    if (mRex)
        delete mRex;
    mRex = rex.isValid() ? new QRegularExpression(rex) : nullptr;

    calcExtraSelections();
}

bool ChangelogFindAdapter::hasFindTerm()
{
    return mRex && mRex->isValid();
}

bool ChangelogFindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
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
    return !cur.isNull();
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
    while (block.blockNumber() <= curTo.blockNumber()) {
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

} // namespace find
} // namespace studio
} // namespace gams
