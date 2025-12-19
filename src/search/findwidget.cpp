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
#include "findwidget.h"
#include "ui_findwidget.h"
#include "viewhelper.h"
#include "logger.h"

#include <QKeyEvent>

namespace gams {
namespace studio {
namespace find {

FindWidget::FindWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindWidget)
{
    ui->setupUi(this);
    ui->edFind->showOptions(FilterLineEdit::foCaSens);
    ui->edReplace->hideOptions(FilterLineEdit::FilterLineEditFlags(FilterLineEdit::foExact | FilterLineEdit::foRegEx));
}

FindWidget::~FindWidget()
{
    delete ui;
}

void FindWidget::setEditWidget(QWidget *widget)
{
    if (mEdit) {
        disconnect(mEdit, &QWidget::destroyed, this, &FindWidget::editDestroyed);
        if (CodeEdit* ce = ViewHelper::toCodeEdit(mEdit)) {
            disconnect(ce, &CodeEdit::allowReplaceChanged, this, &FindWidget::allowReplaceChanged);
            disconnect(ce, &CodeEdit::endFind, this, &FindWidget::on_bClose_clicked);
        }
    }
    mEdit = widget;
    if (mEdit)
        connect(mEdit, &QWidget::destroyed, this, &FindWidget::editDestroyed);

    if (CodeEdit* ce = ViewHelper::toCodeEdit(mEdit)) {
        connect(ce, &CodeEdit::allowReplaceChanged, this, &FindWidget::allowReplaceChanged);
        connect(ce, &CodeEdit::endFind, this, &FindWidget::on_bClose_clicked);
        if (mActive) show();
        ce->updateExtraSelections();
    } else if (TextView* tv = ViewHelper::toTextView(mEdit)) {
        if (mActive) show();
        tv->updateExtraSelections();
    } else hide();
    allowReplaceChanged(mEdit);
}

bool FindWidget::isActive() const
{
    return mActive;
}

void FindWidget::setActive(bool newActive)
{
    mActive = newActive;
    if (!mActive) {
        hide();
        if (mEdit)
            mEdit->setFocus();
    }
}

void FindWidget::updateButtonStates()
{
    bool canFind = !ui->edFind->text().isEmpty();
    ui->bNext->setEnabled(canFind);
    ui->bPrev->setEnabled(canFind);

    CodeEdit *edit = ViewHelper::toCodeEdit(mEdit);
    bool canReplace = canFind  && edit && !edit->isReadOnly() && edit->hasSelectedFind();
    ui->bReplace->setEnabled(canReplace);
    ui->bReplaceForward->setEnabled(canReplace);
    ui->bReplaceBackward->setEnabled(canReplace);
}

void FindWidget::setLastMatch(const QString &text, size_t pos)
{
    mLastMatch = text;
    mLastPos = pos;
}

bool FindWidget::checkLastMatch(const QString &text, size_t pos)
{
    return (text.compare(mLastMatch) == 0 && pos == mLastPos);
}

QString FindWidget::getFindText() const
{
    return ui->edFind->text();
}

bool FindWidget::setFindText(const QString &text)
{
    if (mLastMatch.isEmpty() || text != mLastMatch) {
        ui->edFind->setText(text);
        find();
        return true;
    }
    return false;
}

QRegularExpression FindWidget::termRexEx()
{
    QRegularExpression res;
    if (ui->edFind->isRegEx())
        res = ui->edFind->regExp();
    else
        res.setPattern(res.escape(ui->edFind->text()));

    if (!ui->edFind->isCaseSensitive())
        res.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    return res;
}

QTextDocument::FindFlags FindWidget::findFlags(bool backwards)
{
    QTextDocument::FindFlags res = QTextDocument::FindFlags();
    if (backwards)
        res |= QTextDocument::FindBackward;
    if (ui->edFind->exactMatch())
        res |= QTextDocument::FindWholeWords;
    if (ui->edFind->isCaseSensitive())
        res |= QTextDocument::FindCaseSensitively;
    return res;
}

bool FindWidget::find(FindOptions options, bool keepSearch)
{
    if (!mLastMatch.isEmpty())
        mLastMatch = QString();
    QString match;
    size_t pos = 0;
    if (CodeEdit *edit = ViewHelper::toCodeEdit(mEdit)) {
        if (!edit->hasSelectedFind() && !keepSearch) {
            bool keep = !getFindText().isEmpty();
            QString term = edit->currentFindSelection(keep);
            if (!keep)
                ui->edFind->setText(term);
        }
        edit->findLoop(termRexEx(), findFlags(options.testFlag(foBackwards)), options.testFlag(foContinued));
        if (options.testFlag(foFocusEdit))
            edit->setFocus();
        match = edit->textCursor().selectedText();

    } else if (TextView *view = ViewHelper::toTextView(mEdit)) {
        bool continued = options.testFlag(foContinued);
        view->findText(termRexEx(), findFlags(options.testFlag(foBackwards)), continued);
        if (options.testFlag(foFocusEdit))
            view->edit()->setFocus();
        match = view->selectedText();
    }
    if (!match.isEmpty())
        setLastMatch(match, pos);
    return !match.isEmpty();

}

QString FindWidget::replacementText() const
{
    return ui->edReplace->text();
}

void FindWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    ui->edFind->selectAll();
    ui->edFind->setFocus();
}

void FindWidget::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        && ui->edReplace->hasFocus() && ui->bReplace->isEnabled()) {
        on_bReplaceForward_clicked();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_F3) {
        FindOptions options = foFocusEdit;
        if (event->modifiers().testFlag(Qt::ShiftModifier)) options.setFlag(foBackwards);
        if (event->key() == Qt::Key_F3) options.setFlag(foContinued);
        find(options);
        event->accept();
    } else if (event->key() == Qt::Key_Escape) {
        event->accept();
        if (mEdit)
            mEdit->setFocus();
    } else
        QWidget::keyPressEvent(event);
}

void FindWidget::editDestroyed()
{
    mEdit = nullptr;
}

void FindWidget::allowReplaceChanged(QWidget *edit)
{
    if (edit == mEdit)
       updateButtonStates();
}

void FindWidget::on_bClose_clicked()
{
    setActive(false);
}

void FindWidget::on_bNext_clicked()
{
    find(FindOptions(foFocusEdit | foContinued));
}

void FindWidget::on_bPrev_clicked()
{
    find(FindOptions(foFocusEdit | foBackwards | foContinued));
}

void FindWidget::on_bReplace_clicked()
{
    replace();
    if (mEdit)
        mEdit->setFocus();
}

void FindWidget::on_bReplaceForward_clicked()
{
    if (replace())
        find(foContinued, true);
}

void FindWidget::on_bReplaceBackward_clicked()
{
    if (replace(true))
        find(FindOptions(foBackwards | foContinued), true);
}

void FindWidget::on_edFind_textEdited(const QString &term)
{
    Q_UNUSED(term)
    if (ui->edFind->isRegEx() && !ui->edFind->regExp().isValid())
        return;

    find(foFocusTerm);
}

bool FindWidget::replace(bool cursorToStart)
{
    if (ui->edReplace->text().isEmpty()) return false;

    if (!mLastMatch.isEmpty())
        mLastMatch = QString();
    if (CodeEdit *edit = ViewHelper::toCodeEdit(mEdit)) {
        edit->findReplace(ui->edReplace->text());
        if (cursorToStart) {
            QTextCursor cursor = edit->textCursor();
            cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, ui->edReplace->text().length());
            edit->setTextCursor(cursor);
        }
    } else {
        return false;
    }
    return true;
}

void FindWidget::on_edReplace_textChanged(const QString &)
{
    updateButtonStates();
}




} // namespace find
} // namespace studio
} // namespace gams
