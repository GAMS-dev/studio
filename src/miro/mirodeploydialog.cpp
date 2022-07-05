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
#include "mirodeploydialog.h"
#include "ui_mirodeploydialog.h"
#include "mirocommon.h"
#include "filesystemmodel.h"
#include "theme.h"

#include <QMessageBox>

namespace gams {
namespace studio {
namespace miro {

MiroDeployDialog::MiroDeployDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MiroDeployDialog)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    updateTestDeployButtons();
    connect(ui->fileWidget, &gams::studio::fs::FileSystemWidget::createButtonClicked, this, &MiroDeployDialog::createButtonClicked);
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
    ui->fileWidget->clear();
    ui->targetEnvBox->setCurrentIndex(0);
}

QString MiroDeployDialog::assemblyFileName() const {
    return ui->fileWidget->assemblyFileName();
}

void MiroDeployDialog::setAssemblyFileName(const QString &file) {
    ui->fileWidget->setAssemblyFileName(file);
    updateTestDeployButtons();
}

void MiroDeployDialog::setModelName(const QString &modelName) {
    mModelName = modelName;
    isDataContractAvailable();
}

QStringList MiroDeployDialog::selectedFiles()
{
    return ui->fileWidget->selectedFiles();
}

void MiroDeployDialog::setSelectedFiles(const QStringList &files)
{
    ui->fileWidget->setSelectedFiles(files);
}

void MiroDeployDialog::setWorkingDirectory(const QString &workingDirectory)
{
    ui->fileWidget->setWorkingDirectory(workingDirectory);
}

void MiroDeployDialog::showEvent(QShowEvent *event)
{
    updateTestDeployButtons();
    QDialog::showEvent(event);
}

void MiroDeployDialog::createButtonClicked()
{
    if (selectedFiles().isEmpty())
        QMessageBox::critical(this, "No deployment files!",
                              "Please select the files for your MIRO deployment.");
    else
        emit newAssemblyFileData();
    updateTestDeployButtons();
}

void MiroDeployDialog::on_testBaseButton_clicked()
{
    emit deploy(true, MiroDeployMode::Base);
}

void MiroDeployDialog::on_deployButton_clicked()
{
    accept();
}

void MiroDeployDialog::updateTestDeployButtons()
{
    ui->testBaseButton->setEnabled(ui->fileWidget->validAssemblyFile() && isDataContractAvailable());
    ui->deployButton->setEnabled(ui->fileWidget->validAssemblyFile() && isDataContractAvailable());
}

bool MiroDeployDialog::isDataContractAvailable()
{
    QString path = ui->fileWidget->workingDirectory() + "/" +
                   MiroCommon::confDirectory(mModelName) + "/" +
                   MiroCommon::dataContractFileName(mModelName);

    QFileInfo file(path);
    if (file.exists()) {
        ui->errorLabel->setText("");
        return true;
    }

    auto palette = ui->errorLabel->palette();
    palette.setColor(ui->errorLabel->foregroundRole(),
                     Theme::color(Theme::Normal_Red));
    ui->errorLabel->setPalette(palette);
    ui->errorLabel->setText("It looks like the data contract is missing: " + path +
                            "\n\nPlease run MIRO first before executing any MIRO deploy step.");
    return false;
}

}
}
}
