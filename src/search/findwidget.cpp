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
}

FindWidget::~FindWidget()
{
    delete ui;
}

void FindWidget::setEditWidget(QWidget *widget)
{
    mEdit = widget;
    if (CodeEdit* ce = ViewHelper::toCodeEdit(mEdit)) {
        if (mActive) show();
        ce->updateExtraSelections();
    } else if (TextView* tv = ViewHelper::toTextView(mEdit)) {
        if (mActive) show();
        tv->updateExtraSelections();
    } else hide();
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
        triggerFind();
        return true;
    }
    return false;
}

void FindWidget::setReadonly(bool readonly)
{
    ui->bReplace->setEnabled(!readonly);
    ui->bReplaceForward->setEnabled(!readonly);
    ui->bReplaceBackward->setEnabled(!readonly);
}

QRegularExpression FindWidget::termRexEx()
{
    if (ui->edFind->isRegEx())
        return ui->edFind->regExp();
    QRegularExpression res;
    res.setPattern(res.escape(ui->edFind->text()));
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

void FindWidget::triggerFind(FindOptions options)
{
    if (!mLastMatch.isEmpty())
        mLastMatch = QString();
    doFind(options);
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
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_F3) {
        FindOptions options = foFocusEdit;
        if (event->modifiers().testFlag(Qt::ShiftModifier)) options.setFlag(foBackwards);
        if (event->key() == Qt::Key_F3) options.setFlag(foContinued);
        triggerFind(options);
        event->accept();
    } else if (event->key() == Qt::Key_Escape) {
        event->accept();
        if (mEdit)
            mEdit->setFocus();
    } else
        QWidget::keyPressEvent(event);
}

void FindWidget::on_bClose_clicked()
{
    setActive(false);
}

void FindWidget::on_bNext_clicked()
{
    triggerFind(FindOptions(foFocusEdit | foContinued));
}

void FindWidget::on_bPrev_clicked()
{
    triggerFind(FindOptions(foFocusEdit | foBackwards | foContinued));
}

void FindWidget::on_bReplace_clicked()
{
    replace();
}

void FindWidget::on_bReplaceForward_clicked()
{
    if (replace())
        triggerFind(foContinued);
}

void FindWidget::on_bReplaceBackward_clicked()
{
    if (replace())
        triggerFind(FindOptions(foBackwards | foContinued));
}

void FindWidget::on_edFind_textEdited(const QString &term)
{
    Q_UNUSED(term)
    if (ui->edFind->isRegEx() && !ui->edFind->regExp().isValid())
        return;

    triggerFind(foFocusTerm);
}

bool FindWidget::replace()
{
    if (ui->edReplace->text().isEmpty()) return false;

    if (!mLastMatch.isEmpty())
        mLastMatch = QString();
    if (CodeEdit *edit = ViewHelper::toCodeEdit(mEdit))
        edit->findReplace(ui->edReplace->text());
    else return false;
    return true;
}

bool FindWidget::doFind(FindOptions options)
{
    QString match;
    size_t pos = 0;
    if (CodeEdit *edit = ViewHelper::toCodeEdit(mEdit)) {
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



} // namespace find
} // namespace studio
} // namespace gams
