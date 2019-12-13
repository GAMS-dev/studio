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

MiroDeployDialog::MiroDeployDialog(const QString &modelAssemblyFile, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MiroDeployDialog),
      mModelAssemblyFile(modelAssemblyFile)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

bool MiroDeployDialog::baseMode() const
{
    return ui->baseBox->isChecked();
}

bool MiroDeployDialog::hypercubeMode() const
{
    return ui->hypercubeBox->isChecked();
}

bool MiroDeployDialog::testDeployment() const
{
    return mTestDeploy;
}

MiroTargetEnvironment MiroDeployDialog::targetEnvironment()
{
    if (ui->targetEnvBox->currentText() == "Single user")
        return MiroTargetEnvironment::SingleUser;
    if (ui->targetEnvBox->currentText() == "Multi user")
        return MiroTargetEnvironment::MultiUser;
    return MiroTargetEnvironment::LocalMultiUser;
}

void MiroDeployDialog::on_deployButton_clicked()
{
    if (showMessageBox())
        return;
    mTestDeploy = false;
    accept();
}

void MiroDeployDialog::on_testDeployButton_clicked()
{
    if (showMessageBox())
        return;
    mTestDeploy = true;
    accept();
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
