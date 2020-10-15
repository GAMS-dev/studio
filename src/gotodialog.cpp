/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

GoToDialog::GoToDialog(QWidget *parent, int maxLines)
    : QDialog(parent)
    , ui(new Ui::GoToDialog)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    maxLineCount(maxLines);
    int min = parent->fontMetrics().width(QString::number(mMaxLines)+"0");
    ui->lineEdit->setMinimumWidth(min);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &GoToDialog::on_goToButton_clicked);
}

GoToDialog::~GoToDialog()
{
    delete ui;
}

int GoToDialog::lineNumber() const
{
    return mLineNumber;
}

void GoToDialog::maxLineCount(int maxLines)
{
    mMaxLines = maxLines;
    ui->lineEdit->setPlaceholderText(QString::number(mMaxLines));
}

void GoToDialog::open()
{
    ui->lineEdit->clear();
    QDialog::open();
}

void GoToDialog::on_goToButton_clicked()
{
    mLineNumber = (ui->lineEdit->text().toInt())-1;

    if (mLineNumber > mMaxLines)
        ui->lineEdit->setText(QString::number(mMaxLines));
    if (mLineNumber >= 0 && mLineNumber <= mMaxLines)
        accept();
    else
        reject();
}

}
}
