/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "mirodeploydialog.h"
#include "ui_mirodeploydialog.h"
#include "mirocommon.h"
#include "filesystemmodel.h"
#include "theme.h"
#include "keys.h"

#include <QMessageBox>

namespace gams {
namespace studio {
namespace miro {

MiroDeployDialog::MiroDeployDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MiroDeployDialog)
    , mFileSystemModel(new FileSystemModel(this))
    , mFilterModel(new FilteredFileSystemModel(this))
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mFilterModel->setSourceModel(mFileSystemModel);
    auto oldModel = ui->directoryView->selectionModel();
    ui->directoryView->setModel(mFilterModel);
    delete oldModel;

    updateTestDeployButtons();
    connect(ui->baseBox, &QCheckBox::stateChanged,
            this, &MiroDeployDialog::updateTestDeployButtons);
    connect(ui->hypercubeBox, &QCheckBox::stateChanged,
            this, &MiroDeployDialog::updateTestDeployButtons);
    connect(ui->baseBox, &QCheckBox::stateChanged,
            this, &MiroDeployDialog::checkMiroPaths);
    connect(ui->hypercubeBox, &QCheckBox::stateChanged,
            this, &MiroDeployDialog::checkMiroPaths);
}

bool MiroDeployDialog::baseMode() const
{
    return ui->baseBox->isChecked();
}

bool MiroDeployDialog::hypercubeMode() const
{
    return ui->hypercubeBox->isChecked();
}

MiroTargetEnvironment MiroDeployDialog::targetEnvironment()
{
    if (ui->targetEnvBox->currentText() == "Single user")
        return MiroTargetEnvironment::SingleUser;
    if (ui->targetEnvBox->currentText() == "Multi user")
        return MiroTargetEnvironment::MultiUser;
    return MiroTargetEnvironment::LocalMultiUser;
}

void MiroDeployDialog::setDefaults()
{
    mFileSystemModel->clearSelection();
    ui->baseBox->setCheckState(Qt::Unchecked);
    ui->hypercubeBox->setCheckState(Qt::Unchecked);
    ui->targetEnvBox->setCurrentIndex(0);
}

void MiroDeployDialog::setAssemblyFileName(const QString &file) {
    mModelAssemblyFile = file;
    QFileInfo fi(mModelAssemblyFile);
    mValidAssemblyFile = fi.exists();
    if (fi.exists()) {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Green));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText("File " + fi.fileName() + " found.");
    } else {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Red));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText("No file " + fi.fileName() + " found!");
    }
    updateTestDeployButtons();
}

void MiroDeployDialog::setModelName(const QString &modelName) {
    mModelName = modelName;
    checkMiroPaths();
}

QStringList MiroDeployDialog::selectedFiles()
{
    if (mFileSystemModel)
        return mFileSystemModel->selectedFiles();
    return QStringList();
}

void MiroDeployDialog::setSelectedFiles(const QStringList &files)
{
    mFileSystemModel->setSelectedFiles(files);
}

void MiroDeployDialog::setWorkingDirectory(const QString &workingDirectory)
{
    mWorkingDirectory = workingDirectory;
    setupViewModel();
}

void MiroDeployDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Hotkey::OpenHelp) {
        emit showMiroHelp();
        e->accept();
        return;
    }

    QDialog::keyPressEvent(e);
}

void MiroDeployDialog::on_createButton_clicked()
{
    if (selectedFiles().isEmpty())
        QMessageBox::critical(this, "No deployment files!",
                              "Please select the files for your MIRO deployment.");
    else
        emit newAssemblyFileData();
}

void MiroDeployDialog::on_selectAllButton_clicked()
{
    mFileSystemModel->selectAll();
}

void MiroDeployDialog::on_clearButton_clicked()
{
    mFileSystemModel->clearSelection();
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
}

void MiroDeployDialog::on_testBaseButton_clicked()
{
    emit deploy(true, MiroDeployMode::Base);
}

void MiroDeployDialog::on_testHcubeButton_clicked()
{
    emit deploy(true, MiroDeployMode::Hypercube);
}

void MiroDeployDialog::on_deployButton_clicked()
{
    accept();
}

void MiroDeployDialog::updateTestDeployButtons()
{
    ui->testBaseButton->setEnabled(ui->baseBox->isChecked() &&
                                   mValidAssemblyFile &&
                                   checkMiroPaths());
    ui->testHcubeButton->setEnabled(ui->hypercubeBox->isChecked() &&
                                    mValidAssemblyFile &&
                                    checkMiroPaths());
    ui->deployButton->setEnabled((ui->baseBox->isChecked() ||
                                 ui->hypercubeBox->isChecked()) &&
                                 mValidAssemblyFile &&
                                 checkMiroPaths());
}

bool MiroDeployDialog::checkMiroPaths()
{
    QStringList paths {
        mWorkingDirectory + "/" + MiroCommon::confDirectory(mModelName),
        mWorkingDirectory + "/" + MiroCommon::dataDirectory(mModelName) };

    QStringList missing;
    for (int i=0; i<paths.size(); ++i) {
        QDir dir(paths[i]);
        if (!dir.exists() || dir.isEmpty()) {
            if (missing.size()>0)
                missing << "and";
            missing << paths[i];
        }
    }

    if (missing.isEmpty()) {
        ui->errorLabel->setText("");
        return true;
    }

    auto palette = ui->errorLabel->palette();
    palette.setColor(ui->errorLabel->foregroundRole(),
                     Theme::color(Theme::Normal_Red));
    ui->errorLabel->setPalette(palette);
    ui->errorLabel->setText("It looks like " +
                            missing.join(" ") +
                            (missing.size()>1 ? " are" : " is") +
                            " missing or empty. Please run MIRO first before"
                            " executing any MIRO deploy step.");
    return false;
}

void MiroDeployDialog::setupViewModel()
{
    if (mWorkingDirectory.isEmpty())
        return;

    mFileSystemModel->setRootPath(mWorkingDirectory);
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
}

}
}
}
