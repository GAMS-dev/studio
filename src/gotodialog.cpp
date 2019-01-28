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
#include "gotodialog.h"
#include "ui_gotodialog.h"

#include <QIntValidator>
#include <QPainter>

namespace gams {
namespace studio {

GoToDialog::GoToDialog(QWidget *parent, int maxLines, bool wait)
    : QDialog(parent),
      ui(new Ui::GoToDialog),
      mMaxLines(qAbs(maxLines)),
      mWait(wait)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    ui->lineEdit->setPlaceholderText(QString::number(maxLines));
    int min = parent->fontMetrics().width(QString::number(maxLines)+"0");
    ui->lineEdit->setMinimumWidth(min);
    ui->maxLines->setVisible(false);
    ui->label_2->setVisible(false);
    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &GoToDialog::on_goToButton_clicked);
}

GoToDialog::~GoToDialog()
{
    delete ui;
}

int GoToDialog::lineNumber() const
{
    return mLineNumber;
}

void GoToDialog::setMaxLines(int maxLines)
{
    if (!ui->maxLines->isVisible()) {
        ui->maxLines->setVisible(true);
        ui->label_2->setVisible(true);
        ui->lineEdit->setPlaceholderText(QString());
    }
    mMaxLines = qAbs(maxLines);
    qreal amount = qreal(mMaxLines) / qreal(mLineNumber);

    QLabel *la = ui->maxLines;
    la->setText(QString::number(mMaxLines));
    QPalette pal = la->palette();
    QPixmap pix(la->size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setBrush(QColor(100,150,200, 100));
    p.setPen(Qt::NoPen);
    int x = qRound(la->width() * qBound(0.0 ,amount, 1.0));
    if (amount < 1.0)
        p.drawRect(QRect(0, 0, x, la->height()));
    pal.setBrush(QPalette::Background, QBrush(pix));
    la->setPalette(pal);
    la->repaint();

    if (!mWait && mLineNumber <= mMaxLines)
        accept();
}

void GoToDialog::on_goToButton_clicked()
{
    mLineNumber = (ui->lineEdit->text().toInt())-1;
    if (!mWait && mLineNumber > mMaxLines)
        ui->lineEdit->setText(QString::number(mMaxLines));
    if (mLineNumber <= mMaxLines)
        accept();
    mWait = false;
}

}
}
