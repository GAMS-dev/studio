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
    updateCanStart();
}

NeosStartDialog::~NeosStartDialog()
{
    delete ui;
}

void NeosStartDialog::setConfirmText(QString text, QString checkboxText)
{
    QVBoxLayout *lay = qobject_cast<QVBoxLayout *>(this->layout());
    if (lay) {
        QLabel *laText = mLabelTerms;
        if (!laText) {
            laText = new QLabel(text, this);
            laText->setWordWrap(true);
            laText->setOpenExternalLinks(true);
            lay->insertWidget(1, laText);
            mLabelTerms = laText;
        } else mLabelTerms->setText(text);

        QCheckBox *cbCheck = mConfirmTerms;
        if (!cbCheck) {
            cbCheck = new QCheckBox(checkboxText, this);
            lay->insertWidget(3, cbCheck);
            mConfirmTerms = cbCheck;
            connect(mConfirmTerms, &QCheckBox::stateChanged, [this](){
                Settings::settings()->setBool(SettingsKey::skNeosAcceptTerms, mConfirmTerms->isChecked());
            });
            connect(mConfirmTerms, &QCheckBox::stateChanged, this, &NeosStartDialog::updateCanStart);
            cbCheck->setChecked(Settings::settings()->toBool(SettingsKey::skNeosAcceptTerms));
        } else cbCheck->setText(checkboxText);
    }
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
    bool enabled = mConfirmTerms && mConfirmTerms->isChecked();
    ui->widget->setVisible(enabled);
    if (mLabelTerms) mLabelTerms->setVisible(!enabled);
    ui->cbForceGdx->setEnabled(enabled);
    ui->rbShort->setEnabled(enabled);
    ui->rbLong->setEnabled(enabled);
    ui->bAlways->setEnabled(enabled);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

} // namespace neos
} // namespace studio
} // namespace gams
