#include "mirodeploydialog.h"
#include "ui_mirodeploydialog.h"

#include <QMessageBox>
#include <QDir>

namespace gams {
namespace studio {

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
