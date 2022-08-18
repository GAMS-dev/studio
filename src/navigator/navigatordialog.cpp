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
#include <QKeyEvent>
#include "navigatordialog.h"
#include "ui_navigatordialog.h"

#include <QTime>
#include <QDebug>

namespace gams {
namespace studio {

NavigatorDialog::NavigatorDialog(MainWindow *parent)
    : QDialog((QWidget*)parent), ui(new Ui::NavigatorDialog)
{
    ui->setupUi(this);
    setWindowTitle("Navigator");
    mNavigator = new NavigatorComponent(this, parent);

    connect(ui->input, &QLineEdit::returnPressed, this, &NavigatorDialog::returnPressed);
    connect(ui->input, &QLineEdit::textEdited, mNavigator, &NavigatorComponent::setInput);
    ui->tableView->setModel(mNavigator->Model());

    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

NavigatorDialog::~NavigatorDialog()
{
    delete ui;
}

void NavigatorDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Down) {
        int pos = ui->tableView->currentIndex().row() + 1;
        if (pos >= ui->tableView->model()->rowCount())
            pos = 0;

        ui->tableView->setCurrentIndex(mNavigator->FilterModel()->index(pos, 0));
    } else if (e->key() == Qt::Key_Up) {
        int pos = ui->tableView->currentIndex().row() - 1;
        if (pos < 0)
            pos = ui->tableView->model()->rowCount() - 1;

        ui->tableView->setCurrentIndex(mNavigator->FilterModel()->index(pos, 0));
    } else
        QDialog::keyPressEvent(e);
}

void NavigatorDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
    ui->input->setFocus();
}

void NavigatorDialog::returnPressed()
{
    close();
}

}
}
