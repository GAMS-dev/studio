#include "confirmdialog.h"
#include "ui_confirmdialog.h"

#include "settings.h"

namespace gams {
namespace studio {

ConfirmDialog::ConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmDialog)
{
    ui->setupUi(this);
    ui->buttonBox->setEnabled(false);
    ui->buttonAlwaysOk->setEnabled(false);
}

ConfirmDialog::~ConfirmDialog()
{
    delete ui;
}

void ConfirmDialog::on_checkBox_stateChanged(int state)
{
    ui->buttonBox->setEnabled(state == Qt::Checked);
    ui->buttonAlwaysOk->setEnabled(state == Qt::Checked);
}

void ConfirmDialog::on_buttonAlwaysOk_clicked()
{
    mConfirm = false;
    accept();
}

}
}
