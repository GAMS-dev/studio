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
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    ui->setupUi(this);
    setModal(true);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &NeosStartDialog::buttonClicked);
    connect(ui->bAlways, &QPushButton::clicked, [this](){
        buttonClicked(ui->bAlways);
    });
    ui->cbForceGdx->setChecked(Settings::settings()->toBool(SettingsKey::skNeosForceGdx));
    (Settings::settings()->toBool(SettingsKey::skNeosShortPrio) ? ui->rbShort : ui->rbLong)->setChecked(true);
    connect(ui->cbForceGdx, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->rbShort, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->rbLong, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->cbTerms, &QCheckBox::stateChanged, [this](){
        Settings::settings()->setBool(SettingsKey::skNeosAcceptTerms, ui->cbTerms->isChecked());
        updateCanStart();
    });
    connect(ui->cbHideTerms, &QCheckBox::stateChanged, [this](){
        Settings::settings()->setBool(SettingsKey::skNeosAutoConfirm, ui->cbHideTerms->isChecked());
    });
    connect(ui->cbTerms, &QCheckBox::stateChanged, this, &NeosStartDialog::updateCanStart);
    ui->cbTerms->setChecked(Settings::settings()->toBool(SettingsKey::skNeosAcceptTerms));
    ui->cbHideTerms->setChecked(Settings::settings()->toBool(SettingsKey::skNeosAutoConfirm));
    if (ui->cbHideTerms->isChecked() && ui->cbTerms->isChecked()) ui->widTerms->setVisible(false);
    resize(width(), height() - ui->widTerms->height());
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
    bool mAlways = button == ui->bAlways;
    emit noDialogFlagChanged(mAlways && ui->cbHideTerms->isChecked());
    bool start = mAlways || ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok;
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
    if (mFirstShow) {
        resize(width(), height()/2);
        setMinimumSize(sizeHint());
        setMaximumSize(sizeHint());
    }
    mFirstShow = false;
}

void NeosStartDialog::updateCanStart()
{
    bool enabled = ui->cbTerms->isChecked();
    ui->bAlways->setEnabled(enabled);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

} // namespace neos
} // namespace studio
} // namespace gams
