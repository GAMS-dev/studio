/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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

#include <QMessageBox>
#include <QDir>

namespace gams {
namespace studio {
namespace miro {

MiroDeployDialog::MiroDeployDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MiroDeployDialog)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

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
    ui->baseBox->setCheckState(Qt::Unchecked);
    ui->hypercubeBox->setCheckState(Qt::Unchecked);
    ui->targetEnvBox->setCurrentIndex(0);
}

void MiroDeployDialog::on_testBaseButton_clicked()
{
    if (showMessageBox())
        return;
    emit testDeploy(true, MiroDeployMode::Base);
}

void MiroDeployDialog::on_testHcubeButton_clicked()
{
    if (showMessageBox())
        return;
    emit testDeploy(true, MiroDeployMode::Hypercube);
}

void MiroDeployDialog::on_deployButton_clicked()
{
    if (showMessageBox())
        return;
    accept();
}

void MiroDeployDialog::updateTestDeployButtons()
{
    ui->testBaseButton->setEnabled(ui->baseBox->isChecked());
    ui->testHcubeButton->setEnabled(ui->hypercubeBox->isChecked());
    ui->deployButton->setEnabled(ui->baseBox->isChecked() || ui->hypercubeBox->isChecked());
}

bool MiroDeployDialog::showMessageBox()
{
    bool noFile = !QDir().exists(mModelAssemblyFile);
    if (noFile)
        QMessageBox::critical(this,
                              "No model assembly file!",
                              "Please create the MIRO model assembly file first.");
    return noFile;
}

}
}
}
