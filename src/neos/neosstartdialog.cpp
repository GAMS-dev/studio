#include "neosstartdialog.h"
#include "ui_neosstartdialog.h"
#include "settings.h"

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
    ui->rbShort->setChecked(Settings::settings()->toBool(SettingsKey::skNeosShortPrio));
    ui->rbLong->setChecked(!ui->rbShort->isChecked());
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
            connect(mConfirmTerms, &QCheckBox::stateChanged, this, &NeosStartDialog::updateCanStart);
            cbCheck->setChecked(Settings::settings()->toBool(SettingsKey::skNeosAcceptTerms));
        } else cbCheck->setText(checkboxText);
    }
}

QDialogButtonBox::StandardButton NeosStartDialog::standardButton(QAbstractButton *button) const
{
    if (button == ui->bAlways)
        return QDialogButtonBox::YesToAll;
    return ui->buttonBox->standardButton(button);
}

void NeosStartDialog::buttonClicked(QAbstractButton *button)
{
    bool always = button == ui->bAlways;
    bool start = always || ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok;
    emit ready(start, always);
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
