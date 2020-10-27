#include "neosstartdialog.h"
#include "ui_neosstartdialog.h"
#include "settings.h"
#include "neosprocess.h"
#include "logger.h"

namespace gams {
namespace studio {
namespace neos {

NeosStartDialog::NeosStartDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NeosStartDialog)
{
    ui->setupUi(this);
    setModal(true);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &NeosStartDialog::buttonClicked);
    ui->cbForceGdx->setChecked(Settings::settings()->toBool(SettingsKey::skNeosForceGdx));
    (Settings::settings()->toBool(SettingsKey::skNeosShortPrio) ? ui->rbShort : ui->rbLong)->setChecked(true);
    connect(ui->cbForceGdx, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->rbShort, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->rbLong, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->cbTerms, &QCheckBox::stateChanged, [this](){
        Settings::settings()->setBool(SettingsKey::skNeosAcceptTerms, ui->cbTerms->isChecked());
    });
    connect(ui->cbTerms, &QCheckBox::stateChanged, this, &NeosStartDialog::updateCanStart);
    ui->cbTerms->setChecked(Settings::settings()->toBool(SettingsKey::skNeosAcceptTerms));
    updateCanStart();
}

NeosStartDialog::~NeosStartDialog()
{
    delete ui;
}

void NeosStartDialog::setProcess(NeosProcess *proc)
{
    mProc = proc;
    updateValues();
}

void NeosStartDialog::buttonClicked(QAbstractButton *button)
{
    bool always = button == ui->bAlways;
    bool start = always || ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok;
    Settings::settings()->setBool(SettingsKey::skNeosAutoConfirm, always);
    if (start) accept();
    else reject();
}

void NeosStartDialog::updateValues()
{
    Settings::settings()->setBool(SettingsKey::skNeosForceGdx, ui->cbForceGdx->isChecked());
    Settings::settings()->setBool(SettingsKey::skNeosShortPrio, ui->rbShort->isChecked());
    if (mProc) {
        mProc->setForceGdx(ui->cbForceGdx->isChecked());
        mProc->setPriority(ui->rbShort->isChecked() ? Priority::prioShort : Priority::prioLong);
    }
}

void NeosStartDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    setFixedSize(size());
}

void NeosStartDialog::updateCanStart()
{
    bool enabled = ui->cbTerms->isChecked();
    ui->stackedWidget->setCurrentIndex(enabled ? 0 : 1);
    ui->bAlways->setEnabled(enabled);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

} // namespace neos
} // namespace studio
} // namespace gams
