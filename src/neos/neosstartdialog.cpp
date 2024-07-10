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
#include "neosstartdialog.h"
#include "ui_neosstartdialog.h"
#include "settings.h"
#include "neosprocess.h"
#include "logger.h"

namespace gams {
namespace studio {
namespace neos {

NeosStartDialog::NeosStartDialog(const QString &eMail, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::NeosStartDialog)
{
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    ui->setupUi(this);
    setModal(true);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &NeosStartDialog::buttonClicked);
    connect(ui->bAlways, &QPushButton::clicked, this, [this](){
        buttonClicked(ui->bAlways);
    });
    ui->cbForceGdx->setChecked(Settings::settings()->toBool(SettingsKey::skNeosForceGdx));
    (Settings::settings()->toBool(SettingsKey::skNeosShortPrio) ? ui->rbShort : ui->rbLong)->setChecked(true);
    connect(ui->cbForceGdx, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->rbShort, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->rbLong, &QCheckBox::toggled, this, &NeosStartDialog::updateValues);
    connect(ui->cbTerms, &QCheckBox::stateChanged, this, [this](){
        Settings::settings()->setBool(SettingsKey::skNeosAcceptTerms, ui->cbTerms->isChecked());
        updateCanStart();
    });
    connect(ui->cbHideTerms, &QCheckBox::stateChanged, this, &NeosStartDialog::updateCanStart);
    ui->cbTerms->setChecked(Settings::settings()->toBool(SettingsKey::skNeosAcceptTerms));
    ui->cbHideTerms->setChecked(Settings::settings()->toBool(SettingsKey::skNeosAutoConfirm));
    if (Settings::settings()->toBool(SettingsKey::skNeosAutoConfirm)
            && Settings::settings()->toBool(SettingsKey::skNeosAcceptTerms)) ui->widTerms->setVisible(false);
    if (!eMail.isEmpty()) {
        ui->edEmail->setText(eMail);
    }
    connect(ui->edEmail, &QLineEdit::textEdited, this, [this]() {
        updateValues();
        updateCanStart();
        emit eMailChanged(ui->edEmail->text());
    });
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
    emit noDialogFlagChanged(mAlways);
    bool start = mAlways || ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok;
    if (start) {
        Settings::settings()->setBool(SettingsKey::skNeosAutoConfirm, ui->cbHideTerms->isChecked());
        Settings::settings()->setBool(SettingsKey::skNeosForceGdx, ui->cbForceGdx->isChecked());
        Settings::settings()->setBool(SettingsKey::skNeosShortPrio, ui->rbShort->isChecked());
        accept();
    }
    else reject();
}

void NeosStartDialog::updateValues()
{
    if (mProc) {
        mProc->setForceGdx(ui->cbForceGdx->isChecked());
        mProc->setPriority(ui->rbShort->isChecked() ? Priority::prioShort : Priority::prioLong);
        mProc->setMail(ui->edEmail->text().trimmed());
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

QString NeosStartDialog::validateEmail(const QString &eMail)
{
    QString s = eMail.trimmed();
    if (s.isEmpty()) return "The email is initialized from NEOS_EMAIL in the GAMS config or an environment variable.";
    QString invalidText("\nInvalid email.");
    if (s.indexOf(' ') > 0) return invalidText;
    if (s.indexOf('\t') > 0) return invalidText;
    int i = s.indexOf('@');
    if (i < 1) return invalidText;
    i = s.indexOf('.', i+2);
    if (i < 0) return invalidText;
    if (i == s.length()-1) return invalidText;
    return QString("\n");
}

void NeosStartDialog::updateCanStart()
{
    QString message = validateEmail(ui->edEmail->text().trimmed());
    if (message != ui->laEmailHint->text()) {
        ui->laEmailHint->setText(message);
    }
    bool enabled = ui->cbTerms->isChecked() && message.trimmed().isEmpty();
    ui->bAlways->setEnabled(enabled && ui->cbHideTerms->isChecked());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

} // namespace neos
} // namespace studio
} // namespace gams
