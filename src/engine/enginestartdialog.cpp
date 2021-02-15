/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
 */
#include "enginestartdialog.h"
#include "ui_enginestartdialog.h"
#include "settings.h"
#include "logger.h"
#include "engineprocess.h"
#include <QPushButton>
#include <QEvent>

namespace gams {
namespace studio {
namespace engine {

const QString CUnavailable("-");

EngineStartDialog::EngineStartDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::EngineStartDialog), mProc(nullptr)
{
    ui->setupUi(this);
    QFont f = ui->laWarn->font();
    f.setBold(true);
    ui->laWarn->setFont(f);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &EngineStartDialog::buttonClicked);
    ui->edUrl->setText(Settings::settings()->toString(SettingsKey::skEngineUrl));
    ui->edNamespace->setText(Settings::settings()->toString(SettingsKey::skEngineNamespace));
    ui->edUser->setText(Settings::settings()->toString(SettingsKey::skEngineUser));
    ui->cbForceGdx->setChecked(Settings::settings()->toBool(SettingsKey::skEngineForceGdx));
    connect(ui->edUrl, &QLineEdit::textEdited, this, &EngineStartDialog::urlEdited);
    connect(ui->edUrl, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edNamespace, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edUser, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edPassword, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    GamsProcess gp;
    QString about = gp.aboutGAMS();
    QRegExp regex("^GAMS Release\\s*:\\s+(\\d\\d\\.\\d).*");
    if (regex.exactMatch(about))
        mLocalGamsVersion = regex.cap(regex.captureCount()).split('.');
    emit textChanged("");
    ui->edUrl->installEventFilter(this);
    mConnectStateUpdater.setSingleShot(true);
    mConnectStateUpdater.setInterval(100);
    connect(&mConnectStateUpdater, &QTimer::timeout, this, &EngineStartDialog::updateConnectStateAppearance);

}

EngineStartDialog::~EngineStartDialog()
{
    if (mProc) mProc = nullptr;
    delete ui;
}

void EngineStartDialog::hiddenCheck()
{
    mHiddenCheck = true;
    getVersion();
}

void EngineStartDialog::setProcess(EngineProcess *process)
{
    mProc = process;
    connect(mProc, &EngineProcess::reVersion, this, &EngineStartDialog::reVersion);
    connect(mProc, &EngineProcess::reVersionError, this, &EngineStartDialog::reVersionError);
    mProc->setForceGdx(ui->cbForceGdx->isChecked());
    emit urlEdited(ui->edUrl->text());
}

EngineProcess *EngineStartDialog::process() const
{
    return mProc;
}

QString EngineStartDialog::url() const
{
    return ui->edUrl->text().trimmed();
}

QString EngineStartDialog::nSpace() const
{
    return ui->edNamespace->text();
}

QString EngineStartDialog::user() const
{
    return ui->edUser->text();
}

QString EngineStartDialog::password() const
{
    return ui->edPassword->text();
}

bool EngineStartDialog::forceGdx() const
{
    return ui->cbForceGdx->isChecked();
}

void EngineStartDialog::setLastPassword(QString lastPassword)
{
    ui->edPassword->setText(lastPassword);
}

void EngineStartDialog::focusEmptyField()
{
    if (ui->edUrl->text().isEmpty()) ui->edUrl->setFocus();
    else if (ui->edNamespace->text().isEmpty()) ui->edNamespace->setFocus();
    else if (ui->edUser->text().isEmpty()) ui->edUser->setFocus();
    else if (ui->edPassword->text().isEmpty()) ui->edPassword->setFocus();
}

void EngineStartDialog::setEngineVersion(QString version)
{
    ui->laEngineVersion->setText(version);
}

bool EngineStartDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->edUrl && event->type() == QEvent::FocusOut) {
        ui->edUrl->setText(mUrl.trimmed());
    }
    return QDialog::eventFilter(watched, event);
}

QDialogButtonBox::StandardButton EngineStartDialog::standardButton(QAbstractButton *button) const
{
    if (button == ui->bAlways)
        return QDialogButtonBox::YesToAll;
    return ui->buttonBox->standardButton(button);
}

void EngineStartDialog::closeEvent(QCloseEvent *event)
{
    if (mProc) mProc->abortRequests();
    QDialog::closeEvent(event);
}

void EngineStartDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    setFixedSize(size());
}

void EngineStartDialog::buttonClicked(QAbstractButton *button)
{
    bool always = button == ui->bAlways;
    bool start = always || ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok;
    if (mForcePreviousWork && mProc) mProc->forcePreviousWork();
    emit ready(start, always);
}

void EngineStartDialog::getVersion()
{
    setConnectionState(scsWaiting);
    if (mProc) {
//        if (mPendingRequest) mProc->abortRequests();
        mUrlChanged = false;
        mProc->setUrl(mUrl.trimmed());
        mProc->getVersions();
//        mPendingRequest = true;
    }
}

QString EngineStartDialog::ensureApi(QString url)
{
    if (url.endsWith("/")) return url + "api";
    return url + "/api";
}

void EngineStartDialog::setCanStart(bool canStart)
{
    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (!bOk) return;
    canStart = canStart && !ui->edUrl->text().isEmpty() && !ui->edNamespace->text().isEmpty()
            && !ui->edUser->text().isEmpty() && !ui->edPassword->text().isEmpty();
    if (canStart != bOk->isEnabled())
        bOk->setEnabled(canStart);
    if (canStart != ui->bAlways->isEnabled())
        ui->bAlways->setEnabled(canStart);
}

void EngineStartDialog::setConnectionState(ServerConnectionState state)
{
    mConnectState = state;
    mConnectStateUpdater.start();
}

void EngineStartDialog::urlEdited(const QString &text)
{
    mUrlChanged = true;
    mUrl = text;
    getVersion();
}

void EngineStartDialog::textChanged(const QString &/*text*/)
{
    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (!bOk) return;
    bool enabled = !ui->edUrl->text().isEmpty() && !ui->edNamespace->text().isEmpty()
            && !ui->edUser->text().isEmpty() && !ui->edPassword->text().isEmpty();
    if (enabled && ui->laEngineVersion->text() == CUnavailable) {
        getVersion();
    }
    setConnectionState(mConnectState);
}

void EngineStartDialog::on_bAlways_clicked()
{
    emit buttonClicked(ui->bAlways);
}

void EngineStartDialog::reVersion(const QString &engineVersion, const QString &gamsVersion)
{
//    mPendingRequest = false;
    mEngineVersion = engineVersion;
    mGamsVersion = gamsVersion;
    mValidUrl = mUrl;
    setConnectionState(scsValid);
}

void EngineStartDialog::reVersionError(const QString &errorText)
{
    Q_UNUSED(errorText)
//    mPendingRequest = false;
    if (mUrlChanged) {
        getVersion();
        return;
    }
    // if the raw input failed, try with "/api"
    if (mUrl == ui->edUrl->text()) {
        mUrl = ensureApi(mUrl.trimmed());
        getVersion();
        return;
    }
    // neither user-input nor user-input with "/api" is valid, so reset mUrl to user-input
    setConnectionState(scsInvalid);
    if (mUrl != mValidUrl)
        mUrl = ui->edUrl->text();

    // if server not found on hidden dialog - open dialog anyway
    if (!isVisible() && mHiddenCheck) {
        open();
    }
}

void EngineStartDialog::on_cbForceGdx_stateChanged(int state)
{
    if (mProc) mProc->setForceGdx(state != 0);
}

void EngineStartDialog::updateConnectStateAppearance()
{
    switch (mConnectState) {
    case scsNone: {
        ui->laEngGamsVersion->setText("");
        ui->laEngineVersion->setText(CUnavailable);
        ui->laWarn->setText("No GAMS Engine server");
        ui->laWarn->setToolTip("");
        mForcePreviousWork = false;
        setCanStart(false);
    } break;
    case scsWaiting: {
        ui->laEngGamsVersion->setText("");
        ui->laEngineVersion->setText(CUnavailable);
        ui->laWarn->setText("Waiting for server ...");
        ui->laWarn->setToolTip("");
        mForcePreviousWork = false;
        setCanStart(false);
    } break;
    case scsValid: {
        ui->laEngineVersion->setText("Engine "+mEngineVersion);
        ui->laEngGamsVersion->setText("GAMS "+mGamsVersion);
        if (!mProc->hasPreviousWorkOption()) {
            bool newerGamsVersion = false;
            QStringList engineGamsVersion = QString(mGamsVersion).split('.');
            if (mLocalGamsVersion.at(0).toInt() > engineGamsVersion.at(0).toInt())
                newerGamsVersion = true;
            if (mLocalGamsVersion.at(0).toInt() == engineGamsVersion.at(0).toInt() &&
                mLocalGamsVersion.at(1).toInt() > engineGamsVersion.at(1).toInt())
                newerGamsVersion = true;
            if (newerGamsVersion) {
                ui->laWarn->setText("Newer local GAMS: Added \"previousWork=1\"");
                ui->laWarn->setToolTip("set \"previousWork=0\" to suppress this");
                mForcePreviousWork = true;
            } else {
                ui->laWarn->setText("");
                ui->laWarn->setToolTip("");
                mForcePreviousWork = false;
            }
            if (!isVisible()) {
                // hidden start
                if (mForcePreviousWork && mProc) mProc->forcePreviousWork();
                emit ready(true, true);
            }
        } else {
            ui->laWarn->setText("");
            ui->laWarn->setToolTip("");
            mForcePreviousWork = false;
        }
        setCanStart(true);
    } break;
    case scsInvalid: {
        ui->laEngGamsVersion->setText("");
        ui->laEngineVersion->setText(CUnavailable);
        ui->laWarn->setText("No GAMS Engine server");
        ui->laWarn->setToolTip("");
        mForcePreviousWork = false;
        setCanStart(false);
    } break;
    }
}

} // namespace engine
} // namespace studio
} // namespace gams

