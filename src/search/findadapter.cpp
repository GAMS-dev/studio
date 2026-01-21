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
#include "editors/codeedit.h"
#include "editors/textview.h"

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
{ }

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

QString EditFindAdapter::currentFindSelection() const
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
    CodeEdit* edit = ViewHelper::toCodeEdit(view->edit());
    connect(edit, &CodeEdit::endFind, this, &FindAdapter::endFind);
    view->updateExtraSelections();
}

ViewFindAdapter::~ViewFindAdapter()
{

}

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

bool ViewFindAdapter::findText(const QRegularExpression &rex, FindOptions options)
{
    if (!options.testFlag(foContinued)) {
        QPoint absPos = options.testFlag(foBackwards) ? mView->anchor() : mView->position();
        mView->jumpTo(absPos.y(), absPos.x());
    }
    return mView->findText(rex, findFlags(options), options.testFlag(foContinued));
}

QString ViewFindAdapter::currentFindSelection() const
{
    return mView->currentFindSelection(false);
}

void ViewFindAdapter::invalidateSelection()
{
    mView->clearSelectedFind();
}


} // namespace find
} // namespace studio
} // namespace gams
