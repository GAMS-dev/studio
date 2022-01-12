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
#include "projectoptions.h"
#include "ui_projectoptions.h"
#include "file/pexgroupnode.h"
#include "file/filemeta.h"
#include "theme.h"

#include <QDir>
#include <QPushButton>
#include <QFont>
#include <QFileDialog>

namespace gams {
namespace studio {
namespace project {

Qt::CaseSensitivity fsCaseSensitive()
{
#ifdef __unix__
    return Qt::CaseSensitive;
#else
    return Qt::CaseInsensitive;
#endif
}

ProjectOptions::ProjectOptions(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ProjectOptions)
{
    ui->setupUi(this);
    ui->edMainGms->setEnabled(false);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    ui->edBaseDir->setMinimumWidth(fontMetrics().height()*30);
    ui->edBaseDir->setToolTip("Base directory: used as base folder to represent the files");
    ui->edWorkDir->setToolTip("Working directory: used as working directory to run GAMS");
    ui->laBaseDir->setToolTip(ui->edBaseDir->toolTip());
    ui->laWorkDir->setToolTip(ui->edWorkDir->toolTip());
    ui->bBaseDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->bWorkDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    adjustSize();
}

ProjectOptions::~ProjectOptions()
{
    if (mProject) mProject->unlinkProjectOptionsFileMeta();
    delete ui;
}

bool ProjectOptions::isModified() const
{
    return mModified;
}

void ProjectOptions::setProject(PExProjectNode *project)
{
    if (mProject)
        disconnect(mProject, &PExProjectNode::changed, this, &ProjectOptions::projectChanged);
    if (!project) return;
    mProject = project;
    if (mProject) {
        connect(mProject, &PExProjectNode::changed, this, &ProjectOptions::projectChanged);
        mName = mProject->name();
        ui->edName->setText(mProject->name());
        ui->edWorkDir->setText(QDir::toNativeSeparators(mProject->workDir()));
        ui->edBaseDir->setText(QDir::toNativeSeparators(mProject->location()));
        if (mProject->runnableGms())
            ui->edMainGms->setText(QDir::toNativeSeparators(mProject->runnableGms()->location()));
        else
            ui->edMainGms->setText("-no runnable-");
    }
}

void ProjectOptions::save()
{
    if (ui->edName->text().trimmed().compare(mProject->name()))
        mProject->setName(ui->edName->text().trimmed());
    QString path = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    if (path.compare(mProject->location(), fsCaseSensitive()))
        mProject->setLocation(path);
    path = QDir::fromNativeSeparators(ui->edWorkDir->text()).trimmed();
    if (path.compare(mProject->workDir(), fsCaseSensitive()))
        mProject->setWorkDir(path);
    updateState();
}

void ProjectOptions::on_edName_textChanged(const QString &text)
{
    Q_UNUSED(text)
    updateState();
}

void ProjectOptions::on_edWorkDir_textChanged(const QString &text)
{
    updateEditColor(ui->edWorkDir, text);
    updateState();
}

void ProjectOptions::on_edBaseDir_textChanged(const QString &text)
{
    updateEditColor(ui->edBaseDir, text);
    updateState();
}

void ProjectOptions::updateEditColor(QLineEdit *edit, const QString &text)
{
    QDir dir(text.trimmed());
    if (!dir.exists()) {
        QPalette pal = edit->palette();
        pal.setColor(QPalette::Text, Theme::color(Theme::Mark_errorFg));
        edit->setPalette(pal);
    } else {
        edit->setPalette(QPalette());
    }
}

void ProjectOptions::updateState()
{
    bool isModified = false;
    if (ui->edName->text().trimmed().compare(mProject->name()))
        isModified = true;
    QString path = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    if (path.compare(mProject->location(), fsCaseSensitive()))
        isModified = true;
    path = QDir::fromNativeSeparators(ui->edWorkDir->text()).trimmed();
    if (path.compare(mProject->workDir(), fsCaseSensitive()))
        isModified = true;
    if (isModified != mModified) {
        mModified = isModified;
        emit modificationChanged(mModified);
    }
}

void ProjectOptions::on_bWorkDir_clicked()
{
    showDirDialog("Select Working Directory", ui->edWorkDir, mProject->workDir());
}

void ProjectOptions::on_bBaseDir_clicked()
{
    showDirDialog("Select Base Directory", ui->edBaseDir, mProject->location());
}

void ProjectOptions::projectChanged(NodeId id)
{
    if (mProject->id() != id) return;
    if (mName != mProject->name()) {
        mName = mProject->name();
        ui->edName->setText(mProject->name());
    }
    if (mProject->runnableGms())
        ui->edMainGms->setText(QDir::toNativeSeparators(mProject->runnableGms()->location()));
    else
        ui->edMainGms->setText("-no runnable-");
    updateState();
}

void ProjectOptions::showDirDialog(const QString &title, QLineEdit *lineEdit, QString defaultDir)
{
    QString path = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    QDir dir(path);
    if (!dir.exists()) path = defaultDir;
    QFileDialog *dialog = new QFileDialog(this, title, path);
    dialog->setFileMode(QFileDialog::Directory);
    connect(dialog, &QFileDialog::accepted, this, [lineEdit, dialog]() {
        if (dialog->selectedFiles().count() == 1) {
            QDir dir(dialog->selectedFiles().first().trimmed());
            if (dir.exists()) lineEdit->setText(QDir::toNativeSeparators(dir.path()));
        }
    });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->open();
}

} // namespace project
} // namespace studio
} // namespace gams
