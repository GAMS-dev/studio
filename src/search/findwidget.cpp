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
#include <QKeyEvent>

namespace gams {
namespace studio {
namespace find {

FindWidget::FindWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindWidget)
{
    ui->setupUi(this);
}

FindWidget::~FindWidget()
{
    delete ui;
}

bool FindWidget::active() const
{
    return mActive;
}

void FindWidget::setActive(bool newActive)
{
    mActive = newActive;
    if (!mActive) hide();
}

void FindWidget::setFindText(const QString &text)
{
    ui->edFind->setText(text);
}

void FindWidget::setReadonly(bool readonly)
{
    ui->laReplace->setEnabled(!readonly);
    ui->edReplace->setEnabled(!readonly);
    ui->bReplace->setEnabled(!readonly);
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
    // TODO(JM) Do we want to add case sensitivity to ui->edFind
    return res;
}

void FindWidget::triggerFind(bool backwards)
{
    emit find(termRexEx(), findFlags(backwards));
}

void FindWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    ui->edFind->setFocus();
}

void FindWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        setActive(false);
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        QTextDocument::FindFlags options = QTextDocument::FindFlags();
        if (event->modifiers().testFlag(Qt::ShiftModifier))
            options |= QTextDocument::FindBackward;
        triggerFind(options);
    }
    QWidget::keyPressEvent(event);
}

void FindWidget::on_bClose_clicked()
{
    setActive(false);
}


} // namespace find
} // namespace studio
} // namespace gams
