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
 */
#include "confirmdialog.h"
#include "ui_confirmdialog.h"

#include "settings.h"
#include "logger.h"

namespace gams {
namespace studio {

ConfirmDialog::ConfirmDialog(QString title, QString text, QString checkText, QWidget *parent) :
    QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint),
    ui(new Ui::ConfirmDialog)
{
    ui->setupUi(this);
    setModal(true);
    setWindowTitle(title);
    ui->text->setText(text);
    ui->checkBox->setText(checkText);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonAlwaysOk->setEnabled(false);
}

ConfirmDialog::~ConfirmDialog()
{
    delete ui;
}

void ConfirmDialog::setBoxAccepted(bool accept)
{
    ui->checkBox->setCheckState(accept ? Qt::Checked : Qt::Unchecked);
}

void ConfirmDialog::on_checkBox_stateChanged(int state)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(state == Qt::Checked);
    ui->buttonAlwaysOk->setEnabled(state == Qt::Checked);
    emit setAcceptBox(ui->checkBox->checkState() != Qt::Unchecked);
}

void ConfirmDialog::on_buttonAlwaysOk_clicked()
{
    emit autoConfirm();
    accept();
}

}
}
