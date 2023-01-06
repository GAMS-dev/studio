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
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "projectedit.h"
#include "ui_projectedit.h"
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

ProjectData::ProjectData(PExProjectNode *project)
{
    mProject = project;
    if (!mData.contains(file)) mData.insert(file, QDir::toNativeSeparators(project->fileName()));
    if (!mData.contains(name)) mData.insert(name, project->name());
    if (!mData.contains(nameExt)) mData.insert(nameExt, project->nameExt());
    if (!mData.contains(workDir)) mData.insert(workDir, QDir::toNativeSeparators(project->workDir()));
    if (!mData.contains(baseDir)) mData.insert(baseDir, QDir::toNativeSeparators(project->location()));
    if (!mData.contains(mainGms)) {
        if (mProject->runnableGms())
            mData.insert(mainGms, QDir::toNativeSeparators(mProject->runnableGms()->location()));
        else
            mData.insert(mainGms, "-no runnable-");

    }
    connect(project, &PExProjectNode::changed, this, &ProjectData::projectChanged);
}

void ProjectData::setFieldData(Field field, QString value)
{
    bool change = !mData.contains(field) || mData.value(field) != value;
    mData.insert(field, value);
    if (change) emit changed(field);
}

QString ProjectData::fieldData(Field field)
{
    return mData.value(field);
}

void ProjectData::save()
{
    QString path = QDir::fromNativeSeparators(mData.value(baseDir)).trimmed();
    if (path.compare(mProject->location(), FileType::fsCaseSense()))
        mProject->setLocation(path);
    path = QDir::fromNativeSeparators(mData.value(workDir)).trimmed();
    if (path.compare(mProject->workDir(), FileType::fsCaseSense()))
        mProject->setWorkDir(path);
}

void ProjectData::projectChanged(NodeId id)
{
    if (mProject->id() != id) return;
    bool updateTab = false;
    if (fieldData(ProjectData::file) != QDir::toNativeSeparators(mProject->fileName()))
        setFieldData(ProjectData::file, QDir::toNativeSeparators(mProject->fileName()));
    if (fieldData(ProjectData::name) != mProject->name()) {
        setFieldData(ProjectData::name, mProject->name());
        updateTab = true;
    }
    if (fieldData(ProjectData::nameExt) != mProject->nameExt()) {
        setFieldData(ProjectData::nameExt, mProject->nameExt());
        updateTab = true;
    }
    if (updateTab)
        emit tabNameChanged(mProject);
    if (mProject->runnableGms())
        setFieldData(ProjectData::mainGms, QDir::toNativeSeparators(mProject->runnableGms()->location()));
    else
        setFieldData(ProjectData::mainGms, "-no runnable-");
}

ProjectData *ProjectEdit::sharedData() const
{
    return mSharedData;
}

QString ProjectEdit::tabName(NameModifier mod)
{
    return '[' + mSharedData->project()->name() + mSharedData->project()->nameExt()
            + (mod == NameModifier::editState && isModified() ? "]*" : "]");
}

ProjectEdit::ProjectEdit(ProjectData *sharedData,  QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ProjectEdit)
{
    ui->setupUi(this);
    mSharedData = sharedData;
    ui->edName->setEnabled(false);
    ui->edName->setToolTip("Name: the name of the project, this is always the filename");
    ui->edMainGms->setEnabled(false);
    ui->edMainGms->setToolTip("Main file: this file will be excuted with GAMS");
    ui->edProjectFile->setEnabled(false);
    ui->edProjectFile->setToolTip("Project file: this file contains all project information");
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    ui->edBaseDir->setMinimumWidth(fontMetrics().height()*30);
    ui->edBaseDir->setToolTip("Base directory: used as base folder to represent the files");
    ui->edWorkDir->setToolTip("Working directory: used as working directory to run GAMS");
    ui->laName->setToolTip(ui->edName->toolTip());
    ui->laProjectFile->setToolTip(ui->edProjectFile->toolTip());
    ui->laMainGms->setToolTip(ui->edMainGms->toolTip());
    ui->laBaseDir->setToolTip(ui->edBaseDir->toolTip());
    ui->laWorkDir->setToolTip(ui->edWorkDir->toolTip());
    ui->bBaseDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->bBaseDir->setToolTip("Browse for base directory");
    ui->bWorkDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->bBaseDir->setToolTip("Browse for working directory");
    adjustSize();
    connect(sharedData, &ProjectData::changed, this, &ProjectEdit::updateData);
    connect(sharedData, &ProjectData::tabNameChanged, this, [this](PExProjectNode *project) {
        project->refreshProjectTabName();
    });
    updateData();
}

ProjectEdit::~ProjectEdit()
{
    if (mSharedData->project()) mSharedData->project()->unlinkProjectEditFileMeta();
    delete ui;
}

bool ProjectEdit::isModified() const
{
    return mModified;
}

void ProjectEdit::save()
{
    mSharedData->save();
    updateState();
}

void ProjectEdit::on_edWorkDir_textChanged(const QString &text)
{
    updateEditColor(ui->edWorkDir, text);
    if (text != mSharedData->fieldData(ProjectData::workDir))
        mSharedData->setFieldData(ProjectData::workDir, text);
    updateState();
}

void ProjectEdit::on_edBaseDir_textChanged(const QString &text)
{
    updateEditColor(ui->edBaseDir, text);
    if (text != mSharedData->fieldData(ProjectData::baseDir))
        mSharedData->setFieldData(ProjectData::baseDir, text);
    updateState();
}

void ProjectEdit::updateEditColor(QLineEdit *edit, const QString &text)
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

void ProjectEdit::updateState()
{
    bool isModified = false;
    QString fullName = mSharedData->fieldData(ProjectData::name) + mSharedData->fieldData(ProjectData::nameExt);
    if (ui->edName->text().trimmed().compare(fullName))
        isModified = true;
    QString path = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    if (path.endsWith("/")) path = path.left(path.length()-1);
    if (path.compare(mSharedData->project()->location(), FileType::fsCaseSense()))
        isModified = true;
    path = QDir::fromNativeSeparators(ui->edWorkDir->text()).trimmed();
    if (path.endsWith("/")) path = path.left(path.length()-1);
    if (path.compare(mSharedData->project()->workDir(), FileType::fsCaseSense()))
        isModified = true;
    if (isModified != mModified) {
        mModified = isModified;
        emit modificationChanged(mModified);
    }
}

void ProjectEdit::on_bWorkDir_clicked()
{
    showDirDialog("Select Working Directory", ui->edWorkDir, mSharedData->project()->workDir());
}

void ProjectEdit::on_bBaseDir_clicked()
{
    showDirDialog("Select Base Directory", ui->edBaseDir, mSharedData->project()->location());
}

void ProjectEdit::projectChanged(NodeId id)
{
    if (mSharedData->project()->id() != id) return;
    if (mSharedData->fieldData(ProjectData::file) != QDir::toNativeSeparators(mSharedData->project()->fileName())) {
        mSharedData->setFieldData(ProjectData::file, QDir::toNativeSeparators(mSharedData->project()->fileName()));
    }
    if (mSharedData->fieldData(ProjectData::name) != mSharedData->project()->name()) {
        mSharedData->setFieldData(ProjectData::name, mSharedData->project()->name());
    }
    if (mSharedData->project()->runnableGms())
        ui->edMainGms->setText(QDir::toNativeSeparators(mSharedData->project()->runnableGms()->location()));
    else
        ui->edMainGms->setText("-no runnable-");
    updateState();
}

void ProjectEdit::updateData()
{
    if (ui->edProjectFile->text() != mSharedData->fieldData(ProjectData::file))
        ui->edProjectFile->setText(mSharedData->fieldData(ProjectData::file));
    QString fullName = mSharedData->fieldData(ProjectData::name) + mSharedData->fieldData(ProjectData::nameExt);
    if (ui->edName->text() != fullName)
        ui->edName->setText(fullName);
    if (ui->edWorkDir->text() != mSharedData->fieldData(ProjectData::workDir))
        ui->edWorkDir->setText(mSharedData->fieldData(ProjectData::workDir));
    if (ui->edBaseDir->text() != mSharedData->fieldData(ProjectData::baseDir))
        ui->edBaseDir->setText(mSharedData->fieldData(ProjectData::baseDir));
    if (ui->edMainGms->text() != mSharedData->fieldData(ProjectData::mainGms))
        ui->edMainGms->setText(mSharedData->fieldData(ProjectData::mainGms));
}

void ProjectEdit::showDirDialog(const QString &title, QLineEdit *lineEdit, QString defaultDir)
{
    QString path = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    QDir dir(path);
    if (!dir.exists()) path = defaultDir;
    QFileDialog *dialog = new QFileDialog(this, title, path);
    dialog->setFileMode(QFileDialog::Directory);
    connect(dialog, &QFileDialog::accepted, this, [lineEdit, dialog]() {
        if (dialog->selectedFiles().count() == 1) {
            QDir dir(dialog->selectedFiles().at(0).trimmed());
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
