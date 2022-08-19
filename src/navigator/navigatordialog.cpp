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

// TODO(rogo): delete
#include <QTime>
#include <QDebug>

namespace gams {
namespace studio {

NavigatorDialog::NavigatorDialog(MainWindow *main)
    : QDialog((QWidget*)main), ui(new Ui::NavigatorDialog), mMain(main)
{
    ui->setupUi(this);
    setWindowTitle("Navigator");
    mNavModel = new NavigatorModel(this, main);
    fillContent();

    mFilterModel = new QSortFilterProxyModel(this);
    mFilterModel->setSourceModel(mNavModel);
    mFilterModel->sort(0);
    mFilterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setFilterKeyColumn(0);

    ui->tableView->setModel(mFilterModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->installEventFilter(this);

    connect(ui->input, &QLineEdit::returnPressed, this, &NavigatorDialog::returnPressed);
    connect(ui->input, &QLineEdit::textEdited, this, &NavigatorDialog::setInput);

}

NavigatorDialog::~NavigatorDialog()
{
    delete ui;
    delete mNavModel;
    delete mFilterModel;
}

void NavigatorDialog::setInput(const QString &input)
{
    mFilterModel->setFilterWildcard(input);
}

void NavigatorDialog::fillContent()
{
    QVector<NavigatorContent> content;
    foreach (FileMeta* fm, mMain->fileRepo()->openFiles()) {
        NavigatorContent nc = {fm, "open files"};
        content.append(nc);
    }

    foreach (FileMeta* fm, mMain->fileRepo()->fileMetas()) {
        if (!valueExists(fm, content)) {
            NavigatorContent nc = {fm, "known files"};
            content.append(nc);
        }
    }

    mNavModel->setContent(content);
}

bool NavigatorDialog::valueExists(FileMeta* fm, const QVector<NavigatorContent>& content)
{
    foreach (NavigatorContent c, content) {
        if (c.file == fm)
            return true;
    }
    return false;
}

void NavigatorDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Down) {
        int pos = ui->tableView->currentIndex().row() + 1;
        if (pos >= ui->tableView->model()->rowCount())
            pos = 0;

        ui->tableView->setCurrentIndex(mFilterModel->index(pos, 0));
    } else if (e->key() == Qt::Key_Up) {
        int pos = ui->tableView->currentIndex().row() - 1;
        if (pos < 0)
            pos = ui->tableView->model()->rowCount() - 1;

        ui->tableView->setCurrentIndex(mFilterModel->index(pos, 0));
    } else
        QDialog::keyPressEvent(e);
}

void NavigatorDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
    ui->input->setFocus();
}

bool NavigatorDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != ui->tableView) return false;
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
            keyPressEvent(keyEvent);
            return true;
        }
    }
    return false;
}

void NavigatorDialog::returnPressed()
{
    FileMeta* fm = mNavModel->content().at(
                    mFilterModel->mapToSource(ui->tableView->currentIndex()).row()).file;

    mMain->openFilePath(fm->location(), true);
    close();
}

}
}
