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
#include "mirodeploydialog.h"
#include "ui_mirodeploydialog.h"
#include "filesystemmodel.h"
#include "theme.h"

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
        ui->assemblyFileLabel->setText(fi.fileName());
    } else {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Red));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText("none");
    }
    updateTestDeployButtons();
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

void MiroDeployDialog::on_createButton_clicked()
{
    if (selectedFiles().isEmpty())
        QMessageBox::critical(this, "No deployment files!", "Please select the files for your MIRO deployment.");
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
    emit testDeploy(true, MiroDeployMode::Base);
}

void MiroDeployDialog::on_testHcubeButton_clicked()
{
    emit testDeploy(true, MiroDeployMode::Hypercube);
}

void MiroDeployDialog::on_deployButton_clicked()
{
    accept();
}

void MiroDeployDialog::updateTestDeployButtons()
{
    ui->testBaseButton->setEnabled(ui->baseBox->isChecked() &&
                                   mValidAssemblyFile);
    ui->testHcubeButton->setEnabled(ui->hypercubeBox->isChecked() &&
                                    mValidAssemblyFile);
    ui->deployButton->setEnabled((ui->baseBox->isChecked() ||
                                 ui->hypercubeBox->isChecked()) &&
                                 mValidAssemblyFile);
}

void MiroDeployDialog::setupViewModel()
{
    if (mWorkingDirectory.isEmpty())
        return;

    mFileSystemModel->setRootPath(mWorkingDirectory);
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
    ui->directoryView->expandAll();
}

}
}
}
