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
}

NavigatorDialog::~NavigatorDialog()
{
    delete ui;
    delete mNavModel;
    delete mFilterModel;
}

void NavigatorDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)

    updatePosition();
    updateContent(NavigatorMode::AllFiles);
    mInput->setFocus();
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
    } else if (input.startsWith("p ")) {
        mode = NavigatorMode::InProject;
        mFilterModel->setFilterWildcard("");
    } else if (input.startsWith("t ")) {
        mode = NavigatorMode::Tabs;
        mFilterModel->setFilterWildcard("");
    } else if (input.startsWith("l ")) {
        mode = NavigatorMode::Logs;
        mFilterModel->setFilterWildcard("");
    } else {
        mode = NavigatorMode::AllFiles;
        mFilterModel->setFilterWildcard(input);
    }
    updateContent(mode);
}

void NavigatorDialog::updateContent(NavigatorMode mode) {
    if (mode == mCurrentMode) return;

    QVector<NavigatorContent> content = QVector<NavigatorContent>();
    switch (mode) {
        case NavigatorMode::Help:
            generateHelpContent(content);
        break;
        case NavigatorMode::Line:
            navigateLine(content);
        break;
        case NavigatorMode::AllFiles:
            collectAllFiles(content);
        break;
        case NavigatorMode::InProject:
            collectInProject(content);
        break;
        case NavigatorMode::Tabs:
            collectTabs(content);
        break;
        case NavigatorMode::Logs:
            collectLogs(content);
        break;
        default:
            qWarning() << "Unhandled NavigatorMode";
        break;
    }
    mNavModel->setContent(content);
    mCurrentMode = mode;

    if (mFilterModel)
        mFilterModel->sort(0);
}

void NavigatorDialog::generateHelpContent(QVector<NavigatorContent> &content)
{
    content.append({ nullptr, ":NUMBER", "jump to line number"});
    content.append({ nullptr, "FILENAME", "filter all files"});
    content.append({ nullptr, "p FILENAME", "filter files in current project"});
    content.append({ nullptr, "t FILENAME", "filter open tabs"});
    content.append({ nullptr, "l FILENAME", "filter logs"});
}

void NavigatorDialog::collectAllFiles(QVector<NavigatorContent> &content)
{
    collectTabs(content);
    collectInProject(content);
    collectLogs(content);

    foreach (FileMeta* fm, mMain->fileRepo()->fileMetas()) {
        if (!valueExists(fm, content)) {
            NavigatorContent nc = {fm, fm->location(), "known files"};
            content.append(nc);
        }
    }
}

void NavigatorDialog::collectInProject(QVector<NavigatorContent> &content)
{
    PExFileNode* currentFile = mMain->projectRepo()->findFileNode(mMain->currentEdit());
    if (!currentFile) return;

    for (PExFileNode* f : currentFile->assignedProject()->listFiles()) {
        FileMeta* fm = f->file();
        if (!valueExists(fm, content)) {
            NavigatorContent nc = {fm, fm->location(), "current project"};
            content.append(nc);
        }
    }
}

void NavigatorDialog::collectTabs(QVector<NavigatorContent> &content)
{
    foreach (FileMeta* fm, mMain->fileRepo()->openFiles()) {
        if (!valueExists(fm, content)) {
            NavigatorContent nc = {fm, fm->location(), "open files"};
            content.append(nc);
        }
    }
}

void NavigatorDialog::collectLogs(QVector<NavigatorContent> &content)
{
    for (PExProjectNode* project : mMain->projectRepo()->projects()) {
        PExLogNode* log = project->logNode();

        FileMeta* fm = log->file();
        if (fm->editors().empty()) continue;

        NavigatorContent nc = {fm, fm->location(), "open logs"};
        content.append(nc);
    }
}

void NavigatorDialog::navigateLine(QVector<NavigatorContent> &content)
{
    FileMeta* fm = mMain->fileRepo()->fileMeta(mMain->recent()->editor());
    NavigatorContent nc = { fm, fm->location(),
                            "Max Lines: " + QString::number(mMain->linesInCurrentEditor())
                          };
    content.append(nc);
}

void NavigatorDialog::returnPressed()
{
    QModelIndex index = mFilterModel->mapToSource(ui->tableView->currentIndex());

    if (mCurrentMode == NavigatorMode::Line) {
        QString inputText = mInput->text();
        bool ok = false;

        int lineNr = inputText.midRef(1).toInt(&ok);
        if (ok) mMain->jumpToLine(lineNr-1);

    } else if (index.row() != -1) {
        FileMeta* fm = mNavModel->content().at(index.row()).file;
        mMain->openFile(fm, true);

    } else {
        close();
    }
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
    } else if (e->key() == Qt::Key_Escape) {
        close();
    } else mInput->receiveKeyEvent(e);
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

bool NavigatorDialog::valueExists(FileMeta* fm, const QVector<NavigatorContent>& content)
{
    foreach (NavigatorContent c, content) {
        if (c.file == fm)
            return true;
    }
    return false;
}

}
}
