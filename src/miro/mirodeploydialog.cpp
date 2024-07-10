/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "theme.h"

#include <QMessageBox>
#include <QFileInfo>

namespace gams {
namespace studio {
namespace miro {

MiroDeployDialog::MiroDeployDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MiroDeployDialog)
{
    ui->setupUi(this);
    ui->fsWidget->setTitle("Assembly File");
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    updateTestDeployButtons();
    connect(ui->fsWidget, &gams::studio::fs::FileSystemWidget::createClicked, this, &MiroDeployDialog::createClicked);
}

MiroDeployDialog::~MiroDeployDialog()
{
    delete ui;
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
    ui->fsWidget->clearSelection();
    ui->targetEnvBox->setCurrentIndex(0);
}

void MiroDeployDialog::setAssemblyFileName(const QString &file) {
    mModelAssemblyFile = file;
    QFileInfo fi(mModelAssemblyFile);
    mValidAssemblyFile = fi.exists();
    if (fi.exists()) {
        ui->fsWidget->setInfo("File " + fi.fileName() + " found.", true);
    } else {
        ui->fsWidget->setInfo("No file " + fi.fileName() + " found!", false);
    }
    updateTestDeployButtons();
}

void MiroDeployDialog::setModelName(const QString &modelName) {
    mModelName = modelName;
    ui->fsWidget->setModelName(modelName);
    isDataContractAvailable();
}

QStringList MiroDeployDialog::selectedFiles()
{
    return ui->fsWidget->selectedFiles();
}

void MiroDeployDialog::setSelectedFiles(const QStringList &files)
{
    ui->fsWidget->setSelectedFiles(files);
    if (!mModelAssemblyFile.isEmpty()) setAssemblyFileName(mModelAssemblyFile);
}

void MiroDeployDialog::setWorkingDirectory(const QString &workingDirectory)
{
    ui->fsWidget->setWorkingDirectory(workingDirectory);
    ui->fsWidget->setupViewModel();
}

void MiroDeployDialog::showEvent(QShowEvent *event)
{
    updateTestDeployButtons();
    QDialog::showEvent(event);
}

void MiroDeployDialog::createClicked()
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
    ui->testBaseButton->setEnabled(mValidAssemblyFile && isDataContractAvailable());
    ui->deployButton->setEnabled(mValidAssemblyFile && isDataContractAvailable());
}

bool MiroDeployDialog::isDataContractAvailable()
{
    QString path = ui->fsWidget->workingDirectory() + "/" +
                   MiroCommon::confDirectory(mModelName) + "/" +
                   MiroCommon::dataContractFileName(mModelName);

    QFileInfo file(path);
    if (file.exists()) {
        ui->errorLabel->setText("");
        return true;
    }

    auto palette = qApp->palette();
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
