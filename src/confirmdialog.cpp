#include "confirmdialog.h"
#include "ui_confirmdialog.h"

#include "settings.h"
#include "logger.h"

namespace gams {
namespace studio {

ConfirmDialog::ConfirmDialog(QString title, QString text, QString checkText, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmDialog)
{
    ui->setupUi(this);
    setModal(true);
    setWindowTitle(title);
    ui->text->setText(text);
    ui->checkBox->setText(checkText);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonAlwaysOk->setEnabled(false);
}

ConfirmDialog::~ConfirmDialog()
{
    delete ui;
}

void ConfirmDialog::setBoxAccepted(bool accept)
{
    ui->checkBox->setCheckState(accept ? Qt::Checked : Qt::Unchecked);
}

void ConfirmDialog::on_checkBox_stateChanged(int state)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(state == Qt::Checked);
    ui->buttonAlwaysOk->setEnabled(state == Qt::Checked);
    emit setAcceptBox(ui->checkBox->checkState() != Qt::Unchecked);
}

void ConfirmDialog::on_buttonAlwaysOk_clicked()
{
    emit autoConfirm();
    accept();
}

}
}
