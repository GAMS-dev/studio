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
#include "ui_navigatordialog.h"
#include "navigator/navigatorlineedit.h"
#include "navigatordialog.h"

// TODO(rogo): delete
#include <QTime>
#include <QDebug>

namespace gams {
namespace studio {

NavigatorDialog::NavigatorDialog(MainWindow *main, NavigatorLineEdit* inputField)
    : QDialog((QWidget*)main), ui(new Ui::NavigatorDialog), mMain(main), mInput(inputField)
{
    setWindowFlags(Qt::Popup);
    ui->setupUi(this);
    mNavModel = new NavigatorModel(this, main);
    updateContent(NavigatorMode::AllFiles);

    mFilterModel = new QSortFilterProxyModel(this);
    mFilterModel->setSourceModel(mNavModel);
    mFilterModel->sort(0);
    mFilterModel->setDynamicSortFilter(false);
    mFilterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setFilterKeyColumn(0);

    ui->tableView->setModel(mFilterModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->installEventFilter(this);

    connect(mInput, &QLineEdit::returnPressed, this, &NavigatorDialog::returnPressed);
    connect(mInput, &QLineEdit::textEdited, this, &NavigatorDialog::setInput);
    connect(mNavModel, &NavigatorModel::dataChanged, mFilterModel, &QSortFilterProxyModel::dataChanged);
}

NavigatorDialog::~NavigatorDialog()
{
    delete ui;
    delete mNavModel;
    delete mFilterModel;
}

void NavigatorDialog::setInput(const QString &input)
{
    if (!isVisible()) {
        show();
        mInput->setFocus();
    }

    NavigatorMode mode;

    if (input.startsWith("?")) {
        mode = NavigatorMode::Help;
        mFilterModel->setFilterWildcard("");
    } else if (input.startsWith(":")) {
        mode = NavigatorMode::Line;
        mFilterModel->setFilterWildcard("");
    } else {
        mode = NavigatorMode::AllFiles;
        mFilterModel->setFilterWildcard(input);
    }
    updateContent(mode);
}

void NavigatorDialog::updateContent(NavigatorMode mode) {
    if (mode == mCurrentMode) return;

    QVector<NavigatorContent> content;
    switch (mode) {
        case NavigatorMode::AllFiles:
            content = collectAllFiles();
            break;
        case NavigatorMode::Line:
            content = navigateLine();
        break;
        case NavigatorMode::Help:
            content = showHelpContent();
        break;
        default:
            content = QVector<NavigatorContent>();
        break;
    }
    mNavModel->setContent(content);
    mCurrentMode = mode;

    if (mFilterModel)
        mFilterModel->sort(0);
}

QVector<NavigatorContent> NavigatorDialog::showHelpContent()
{
    QVector<NavigatorContent> content;

    content.append({ nullptr, "FILENAME", "filter files"});
    content.append({ nullptr, ":NUMBER", "jump to line number"});

    return content;
}

QVector<NavigatorContent> NavigatorDialog::collectAllFiles()
{
    QVector<NavigatorContent> content;
    foreach (FileMeta* fm, mMain->fileRepo()->openFiles()) {
        NavigatorContent nc = {fm, fm->location(), "open files"};
        content.append(nc);
    }

    foreach (FileMeta* fm, mMain->fileRepo()->fileMetas()) {
        if (!valueExists(fm, content)) {
            NavigatorContent nc = {fm, fm->location(), "known files"};
            content.append(nc);
        }
    }

    return content;
}

bool NavigatorDialog::valueExists(FileMeta* fm, const QVector<NavigatorContent>& content)
{
    foreach (NavigatorContent c, content) {
        if (c.file == fm)
            return true;
    }
    return false;
}

QVector<NavigatorContent> NavigatorDialog::navigateLine()
{
    QVector<NavigatorContent> content;

    FileMeta* fm = mMain->fileRepo()->fileMeta(mMain->recent()->editor());
    NavigatorContent nc = { fm, fm->location(),
                            "Max Lines: " + QString::number(mMain->linesInCurrentEditor())
                          };
    content.append(nc);

    return content;
}

void NavigatorDialog::returnPressed()
{
    QModelIndex index = mFilterModel->mapToSource(ui->tableView->currentIndex());

    if (mCurrentMode == NavigatorMode::AllFiles && index.row() != -1) {
        FileMeta* fm = mNavModel->content().at(index.row()).file;
        mMain->openFile(fm, true);

    } else if (mCurrentMode == NavigatorMode::Line) {
        QString inputText = mInput->text();
        bool ok = false;

        int lineNr = inputText.midRef(1).toInt(&ok);
        if (ok) mMain->jumpToLine(lineNr-1);
    }
    close();
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
    } else if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Return ||  e->key() == Qt::Key_Enter) {
        close();
    } else mInput->receiveKeyEvent(e);
}

void NavigatorDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)

    updatePosition();
    mInput->setFocus();
}

void NavigatorDialog::updatePosition()
{
    QPoint position;

    position.setX(mInput->pos().x() - width() - mInput->width());
    position.setY(mInput->pos().y() - height() - 5);

    move(mInput->mapToGlobal(position));
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

}
}
