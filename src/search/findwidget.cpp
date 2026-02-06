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
    ui->edFind->setBoundaryMode(FilterLineEdit::bmWordBound);
    ui->edFind->setNoWildcards(true);
    ui->bClose->setIcon(Theme::icon(":/%1/remove"));
    ui->bNext->setIcon(Theme::icon(":/%1/sort-desc"));
    ui->bPrev->setIcon(Theme::icon(":/%1/sort-asc"));
    ui->edReplace->hideOptions(FilterLineEdit::FilterLineEditFlags(FilterLineEdit::foExact | FilterLineEdit::foRegEx));
    ui->bReplace->setIcon(Theme::icon(":/%1/replace"));
    ui->bReplaceAll->setIcon(Theme::icon(":/%1/replace-all"));
    ui->bToggleReplace->setChecked(false);
    on_bToggleReplace_clicked();
}

FindWidget::~FindWidget()
{
    delete ui;
}

void FindWidget::setEditWidget(QWidget *widget)
{
    if (mFinder) {
        if (mFinder->widget() == widget)
            return;
        FindAdapter *finder = mFinder;
        mFinder = nullptr;
        delete finder;
    }
    mFinder = FindAdapter::createAdapter(widget);
    if (!mFinder) {
        hide();
        return;
    }
    connect(mFinder, &FindAdapter::destroyed, this, &FindWidget::editDestroyed);
    connect(mFinder, &FindAdapter::allowReplaceChanged, this, &FindWidget::allowReplaceChanged);
    connect(mFinder, &FindAdapter::endFind, this, &FindWidget::on_bClose_clicked);
    connect(mFinder, &FindAdapter::findNext, this, [this](bool backwards) {
        if (backwards) on_bPrev_clicked();
        else on_bNext_clicked();
    });
    if (mActive)
        show();
    if (!ui->edFind->text().isEmpty()) {
        FindOptions options;
        if (ui->edFind->exactMatch()) options.setFlag(foExactMatch);
        if (ui->edFind->isCaseSensitive()) options.setFlag(foCaseSense);
        mFinder->setFindTerm(termRegEx(), options);
    }
    if (canReplace() && mReplaceVisible) {
        if (!ui->bToggleReplace->isChecked())
            setReplaceVisible(true);
    } else if (ui->bToggleReplace->isChecked()) {
        setReplaceVisible(false);
    }

    updateButtonStates();
}

QWidget *FindWidget::editWidget() const
{
    if (mFinder) return mFinder->widget();
    return nullptr;
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
        if (mFinder) {
            mFinder->setFocus();
            mFinder->setFindTerm(QRegularExpression(), foNone);
            mFinder->invalidateSelection();
        }
        ui->edFind->clear();
    }
}

void FindWidget::updateButtonStates()
{
    bool canFind = mFinder && !ui->edFind->text().isEmpty()
                   && (!ui->edFind->isRegEx() || ui->edFind->regExp().isValid());
    ui->bNext->setEnabled(canFind);
    ui->bPrev->setEnabled(canFind);

    bool canReplace = canFind  && this->canReplace();
    ui->bReplace->setEnabled(canReplace);
    ui->bReplaceAll->setEnabled(canReplace);
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
        if (!ui->edFind->isRegEx() || ui->edFind->regExp().isValid())
            find();
        updateButtonStates();
        return true;
    }
    return false;
}

bool FindWidget::hasTerm()
{
    return mFinder->hasFindTerm();
}

QRegularExpression FindWidget::termRegEx()
{
    QRegularExpression res;
    res = ui->edFind->regExp();
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

bool FindWidget::find(FindOptions options, bool keepSearchTerm)
{
    if (!mFinder) return false;

    if (!mLastMatch.isEmpty())
        mLastMatch = QString();
    QString match;
    size_t pos = 0;
    bool isCurrentWord = false;
    if (!mFinder->hasFindTerm() && mFinder->supportsRegEx() && !ui->edFind->hasFocus())
        keepSearchTerm = false;
    if (!mFinder->hasSelectedFind() && !keepSearchTerm) {
        QString term = mFinder->currentFindSelection(isCurrentWord);
        if (isCurrentWord && !options.testFlag(foContinued))
            options.setFlag(foSkipFind);
        if (!term.isEmpty()) {
            ui->edFind->setText(term);
            updateButtonStates();
        }
    }
    if (ui->edFind->exactMatch()) options.setFlag(foExactMatch);
    if (ui->edFind->isCaseSensitive()) options.setFlag(foCaseSense);
    bool found = mFinder->supportsRegEx() ? mFinder->findText(termRegEx(), options)
                                          : mFinder->findText(ui->edFind->text(), options);
    if (options.testFlag(foFocusEdit))
        mFinder->setFocus();
    match = mFinder->currentFindSelection(isCurrentWord);
    if (!found)
        mFinder->invalidateSelection();

    if (!match.isEmpty())
        setLastMatch(match, pos);

    return !match.isEmpty();
}

QString FindWidget::currentFindSelection(bool &isCurrentWord)
{
    if (!mFinder) return QString();
    return mFinder->currentFindSelection(isCurrentWord);
}

bool FindWidget::canReplace() const
{
    return mFinder && mFinder->canReplace();
}

void FindWidget::toggleReplace(bool ensureVisible)
{
    if (canReplace()) {
        if (!ensureVisible || !ui->bToggleReplace->isChecked())
            ui->bToggleReplace->setChecked(!ui->bToggleReplace->isChecked());
        on_bToggleReplace_clicked();
        if (ui->bToggleReplace->isChecked())
            ui->edReplace->setFocus();
    }
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
        on_bReplace_clicked();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_F3) {
        FindOptions options = foContinued;
        if (event->modifiers().testFlag(Qt::ShiftModifier)) options.setFlag(foBackwards);
        find(options);
        event->accept();
    } else if (event->key() == Qt::Key_Escape) {
        event->accept();
        if (mFinder)
            mFinder->setFocus();
    } else if (event->key() == Qt::Key_R && event->modifiers().testFlag(Qt::ControlModifier)) {
        event->accept();
        toggleReplace();
    } else
        QWidget::keyPressEvent(event);
}

void FindWidget::editDestroyed()
{
    mFinder = nullptr;
}

void FindWidget::termChanged()
{
    if (!ui->edFind->isRegEx() || (!ui->edFind->regExp().pattern().isEmpty() && ui->edFind->regExp().isValid()))
        find(foFocusTerm);
    updateButtonStates();
}

void FindWidget::allowReplaceChanged()
{
    updateButtonStates();
}

void FindWidget::on_bClose_clicked()
{
    setActive(false);
}

void FindWidget::on_bNext_clicked()
{
    if (mFinder && (mFinder->hasFindTerm() || !mFinder->supportsRegEx()))
        find(FindOptions(foFocusEdit | foContinued));
}

void FindWidget::on_bPrev_clicked()
{
    if (mFinder && (mFinder->hasFindTerm() || !mFinder->supportsRegEx()))
        find(FindOptions(foFocusEdit | foBackwards | foContinued));
}

void FindWidget::on_bReplace_clicked()
{
    if (replace())
        find(foContinued);
}

void FindWidget::on_bReplaceAll_clicked()
{
    if (!mFinder || !mFinder->canReplace()) return;

    FindOptions options;
    if (ui->edFind->exactMatch()) options.setFlag(foExactMatch);
    if (ui->edFind->isCaseSensitive()) options.setFlag(foCaseSense);
    int count = mFinder->findReplaceAll(termRegEx(), options, ui->edReplace->text());
    mFinder->setFocus();
    QString countText = (count ? QString::number(count) : QString("No"));
    DEB() << QString("%1 occurrencies replaced").arg(countText);
    // TODO(JM) show count of replacements
}

void FindWidget::on_edFind_textEdited(const QString &term)
{
    Q_UNUSED(term)
    termChanged();
}

bool FindWidget::replace()
{
    if (!mFinder || !mFinder->canReplace()) return false;

    if (!mLastMatch.isEmpty())
        mLastMatch = QString();
    FindOptions options;
    if (ui->edFind->exactMatch()) options.setFlag(foExactMatch);
    if (ui->edFind->isCaseSensitive()) options.setFlag(foCaseSense);
    return mFinder->findReplace(ui->edReplace->text());
}

void FindWidget::on_edReplace_textChanged(const QString &)
{
    updateButtonStates();
}

void FindWidget::on_bToggleReplace_clicked()
{
    bool visible = ui->bToggleReplace->isChecked() && mFinder;
    mReplaceVisible = visible;
    setReplaceVisible(visible);
}

void FindWidget::setReplaceVisible(bool visible)
{
    ui->bToggleReplace->setChecked(visible);
    ui->bToggleReplace->setIcon(Theme::icon(visible ? ":/%1/hide" :":/%1/show"));
    ui->bToggleReplace->setToolTip(visible ? "Hide Replace" : "Show Replace");
    ui->edReplace->setVisible(visible);
    ui->bReplace->setVisible(visible);
    ui->bReplaceAll->setVisible(visible);
}


} // namespace find
} // namespace studio
} // namespace gams
