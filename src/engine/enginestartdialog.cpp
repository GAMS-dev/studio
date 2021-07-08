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
#include "logger.h"
#include "engineprocess.h"
#include "theme.h"
#include <QPushButton>
#include <QEvent>
#include <QUrl>
#include <QSslError>

namespace gams {
namespace studio {
namespace engine {

const QString EngineStartDialog::CUnavailable("-");

EngineStartDialog::EngineStartDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::EngineStartDialog), mProc(nullptr)
{
    ui->setupUi(this);
    setCanLogin(false);
    QFont f = ui->laWarn->font();
    f.setBold(true);
    ui->laWarn->setFont(f);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &EngineStartDialog::buttonClicked);
    connect(ui->edUrl, &QLineEdit::textEdited, this, [this]() { mUrlChangedTimer.start(); });
    connect(ui->edUrl, &QLineEdit::textChanged, this, &EngineStartDialog::updateLoginStates);
    connect(ui->edNamespace, &QLineEdit::textChanged, this, &EngineStartDialog::updateSubmitStates);
    connect(ui->edUser, &QLineEdit::textChanged, this, &EngineStartDialog::updateLoginStates);
    connect(ui->edPassword, &QLineEdit::textChanged, this, &EngineStartDialog::updateLoginStates);
    connect(ui->bLogin, &QPushButton::clicked, this, &EngineStartDialog::bLoginClicked);
    connect(ui->bLogout, &QPushButton::clicked, this, &EngineStartDialog::bLogoutClicked);
    connect(ui->cbForceGdx, &QCheckBox::stateChanged, this, &EngineStartDialog::forceGdxStateChanged);
    connect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);
    connect(ui->bAlways, &QPushButton::clicked, this, [this]() { buttonClicked(ui->bAlways); });

    ui->stackedWidget->setCurrentIndex(0);
    ui->bAlways->setVisible(false);
    ui->buttonBox->setVisible(false);
    ui->bLogin->setVisible(true);
    ui->cbAcceptCert->setVisible(false);

    if (Theme::instance()->baseTheme(Theme::instance()->activeTheme()) != 0)
        ui->laLogo->setPixmap(QPixmap(QString::fromUtf8(":/img/engine-logo-w")));
    GamsProcess gp;
    QString about = gp.aboutGAMS();
    QRegExp regex("^GAMS Release\\s*:\\s+(\\d\\d\\.\\d).*");
    if (regex.exactMatch(about))
        mLocalGamsVersion = regex.cap(regex.captureCount()).split('.');
    updateLoginStates();
    ui->edUrl->installEventFilter(this);
    mConnectStateUpdater.setSingleShot(true);
    mConnectStateUpdater.setInterval(100);
    connect(&mConnectStateUpdater, &QTimer::timeout, this, &EngineStartDialog::updateConnectStateAppearance);
    mUrlChangedTimer.setSingleShot(true);
    mUrlChangedTimer.setInterval(200);
    connect(&mUrlChangedTimer, &QTimer::timeout, this, [this]() { urlEdited(ui->edUrl->text()); });
}

EngineStartDialog::~EngineStartDialog()
{
    if (mProc) mProc = nullptr;
    delete ui;
}

void EngineStartDialog::authorizeChanged(QString authToken)
{
    Q_UNUSED(authToken)
    if (mProc->authToken().isEmpty())
        showLogin();
    else
        mProc->listJobs();
}

void EngineStartDialog::setHiddenMode(bool preferHidden)
{
    mHiddenMode = preferHidden;
}

void EngineStartDialog::start()
{
    if (!mProc) return;
    if (ui->edUrl->text().isEmpty())
        showLogin();
    else
        urlEdited(ui->edUrl->text());
}

void EngineStartDialog::setProcess(EngineProcess *process)
{
    mProc = process;
    connect(mProc, &EngineProcess::authorized, this, &EngineStartDialog::authorizeChanged);
    connect(mProc, &EngineProcess::reListJobs, this, &EngineStartDialog::reListJobs);
    connect(mProc, &EngineProcess::reListJobsError, this, &EngineStartDialog::reListJobsError);
    connect(mProc, &EngineProcess::reVersion, this, &EngineStartDialog::reVersion);
    connect(mProc, &EngineProcess::reVersionError, this, &EngineStartDialog::reVersionError);
    connect(mProc, &EngineProcess::sslSelfSigned, this, &EngineStartDialog::selfSignedCertFound);
    mProc->setForceGdx(ui->cbForceGdx->isChecked());
}

EngineProcess *EngineStartDialog::process() const
{
    return mProc;
}

void EngineStartDialog::setAcceptCert()
{
    if (!ui->cbAcceptCert->isVisible())
        ui->cbAcceptCert->setVisible(true);
    ui->cbAcceptCert->setChecked(true);
    mProc->setIgnoreSslErrorsCurrentUrl(true);
}

bool EngineStartDialog::isCertAccepted()
{
    return ui->cbAcceptCert->isChecked();
}

void EngineStartDialog::initData(const QString &_url, const QString &_user, int authExpireMinutes, const QString &_nSpace, bool _forceGdx)
{
    mUrl = cleanUrl(_url);
    ui->edUrl->setText(mUrl);
    ui->nUrl->setText(mUrl);
    ui->edUser->setText(_user.trimmed());
    ui->nUser->setText(_user.trimmed());
    mAuthExpireMinutes = authExpireMinutes;
    ui->edNamespace->setText(_nSpace.trimmed());
    ui->cbForceGdx->setChecked(_forceGdx);
}

bool EngineStartDialog::isAlways()
{
    return mAlways;
}

QString EngineStartDialog::url() const
{
    return mValidUrl;
}

QString EngineStartDialog::nSpace() const
{
    return ui->edNamespace->text();
}

QString EngineStartDialog::user() const
{
    return ui->edUser->text();
}

QString EngineStartDialog::authToken() const
{
    return mProc ? mProc->authToken() : QString();
}

bool EngineStartDialog::forceGdx() const
{
    return ui->cbForceGdx->isChecked();
}

void EngineStartDialog::focusEmptyField()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        if (ui->edUrl->text().isEmpty()) ui->edUrl->setFocus();
        else if (ui->edUser->text().isEmpty()) ui->edUser->setFocus();
        else if (ui->edPassword->text().isEmpty()) ui->edPassword->setFocus();
    } else {
        if (ui->edNamespace->text().isEmpty()) ui->edNamespace->setFocus();
    }
}

void EngineStartDialog::setEngineVersion(QString version)
{
    ui->laEngineVersion->setText(version);
}

bool EngineStartDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->edUrl && event->type() == QEvent::FocusOut)
        updateUrlEdit();
    return QDialog::eventFilter(watched, event);
}

void EngineStartDialog::updateUrlEdit()
{
    QString url = cleanUrl(mValidUrl.isEmpty() ? mValidSelfCertUrl : mValidUrl);
    UrlCheck prot = protocol(ui->edUrl->text().trimmed());
    if (!url.isEmpty() && (prot == ucNone || prot == protocol(url)))
        ui->edUrl->setText(url);
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
    disconnect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);
    ui->cbAcceptCert->setChecked(false);
    connect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);
    QDialog::closeEvent(event);
}

void EngineStartDialog::showEvent(QShowEvent *event)
{
    bool isHidden = !ui->cbAcceptCert->isVisible();
    if (isHidden)
        ui->cbAcceptCert->setVisible(true);
    QDialog::showEvent(event);
    setFixedSize(size());
    if (isHidden)
        ui->cbAcceptCert->setVisible(false);
}

void EngineStartDialog::showLogin()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->bAlways->setVisible(false);
    ui->buttonBox->setVisible(false);
    ui->bLogin->setVisible(true);
    ensureOpened();
}

void EngineStartDialog::showSubmit()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->bAlways->setVisible(true);
    ui->buttonBox->setVisible(true);
    ui->bLogin->setVisible(false);
    ui->nUser->setText(ui->edUser->text().trimmed());
    ui->nUrl->setText(mProc->url().toString());
    if (!mHiddenMode)
        ensureOpened();
}

void EngineStartDialog::ensureOpened()
{
    if (!isVisible()) {
        mHiddenMode = false;
//        if (!ui->cbAcceptCert->isVisible())
//            ui->cbAcceptCert->setVisible(true);
        open();
        focusEmptyField();
    }
}

void EngineStartDialog::bLoginClicked()
{
    mProc->authorize(ui->edUser->text(), ui->edPassword->text(), mAuthExpireMinutes);
}

void EngineStartDialog::bLogoutClicked()
{
    ui->edPassword->setText("");
    mProc->setAuthToken("");
    showLogin();
}

void EngineStartDialog::buttonClicked(QAbstractButton *button)
{
    mAlways = button == ui->bAlways;
    bool start = mAlways || ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok;
    if (mForcePreviousWork && mProc) mProc->forcePreviousWork();
    if (!start) {
        disconnect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);
        ui->cbAcceptCert->setChecked(false);
        connect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);
    }
    mProc->setNamespace(ui->edNamespace->text().trimmed());
    emit submit(start);
}

void EngineStartDialog::getVersion()
{
    setConnectionState(scsWaiting);
    if (mProc) {
        if (mProc->setUrl(mUrl)) {
            if (protocol(mUrl) == ucHttps && ui->cbAcceptCert->isVisible() && ui->cbAcceptCert->isChecked())
                mProc->setIgnoreSslErrorsCurrentUrl(true);
            mUrlChanged = false;
            mProc->getVersions();
            return;
        }
    }
    mConnectState = scsNone;
    updateConnectStateAppearance();
}

void EngineStartDialog::setCanLogin(bool value)
{
    value = value && !ui->edUrl->text().isEmpty()
            && !ui->edUser->text().isEmpty()
            && (!ui->edPassword->text().isEmpty() || !mProc->authToken().isEmpty())
            && (!ui->cbAcceptCert->isVisible() || ui->cbAcceptCert->isChecked());
    if (value != ui->bLogin->isEnabled())
        ui->bLogin->setEnabled(value);
}

void EngineStartDialog::setCanSubmit(bool value)
{
    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    value = value && ui->edNamespace->text().isEmpty();
    if (value != bOk->isEnabled())
        bOk->setEnabled(value);
}

void EngineStartDialog::setConnectionState(ServerConnectionState state)
{
    mConnectState = state;
    mConnectStateUpdater.start();
}

void EngineStartDialog::certAcceptChanged()
{
    mProc->abortRequests();
    mProc->setIgnoreSslErrorsCurrentUrl(ui->cbAcceptCert->isChecked() && ui->cbAcceptCert->isVisible());
    urlEdited(ui->edUrl->text());
}

void EngineStartDialog::hideCert()
{
    ui->cbAcceptCert->setVisible(false);
}

void EngineStartDialog::urlEdited(const QString &text)
{
    mProc->abortRequests();
    initUrlAndChecks(text);
    getVersion();
    if (!isVisible() && !mHiddenMode)
        showLogin();
}

void EngineStartDialog::updateLoginStates()
{
    bool enabled = !ui->edUrl->text().isEmpty()
            && !ui->edUser->text().isEmpty() && !ui->edPassword->text().isEmpty();
    if (enabled && ui->laEngineVersion->text() == CUnavailable) {
        getVersion();
    }
    setConnectionState(mConnectState);
}

void EngineStartDialog::updateSubmitStates()
{
    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (!bOk) return;
    bool enabled = !ui->edNamespace->text().isEmpty();
    if (enabled && ui->laEngineVersion->text() == CUnavailable) {
        getVersion();
    }
    setConnectionState(mConnectState);
}

void EngineStartDialog::reListJobs(qint32 count)
{
    ui->nJobCount->setText(QString::number(count));
    showSubmit();
}

void EngineStartDialog::reListJobsError(const QString &error)
{
    ui->bLogin->setEnabled(true);
}

void EngineStartDialog::reVersion(const QString &engineVersion, const QString &gamsVersion)
{
    mUrlChecks = ucNone;
    mEngineVersion = engineVersion;
    mGamsVersion = gamsVersion;
    UrlCheck protUser = protocol(cleanUrl(ui->edUrl->text()));
    UrlCheck protServer = protocol(mProc->url().toString());
    if (protUser != ucNone && protUser != protServer) {
        setConnectionState((protServer == ucHttp || protServer == ucApiHttp) ? scsHttpFound : scsHttpsFound);
    } else {
        mValidUrl = mProc->url().toString();
        if (focusWidget() != ui->edUrl)
            updateUrlEdit();
        setConnectionState(scsValid);
        if (mProc->authToken().isEmpty())
            showLogin();
        else
            mProc->listJobs();
    }
}

void EngineStartDialog::reVersionError(const QString &errorText)
{
    if (!mValidUrl.isEmpty()) return;
    Q_UNUSED(errorText)

    if (mUrlChanged) {
        getVersion();
        return;
    }
    // if the raw input failed, try next protocol/api combination
    if (!mLastSslError && fetchNextUrl()) {
        getVersion();
        return;
    }
    // neither user-input nor user-input with modifications is valid, so reset mUrl to user-input
    setConnectionState(scsInvalid);
    if (mUrl != mValidUrl)
        mUrl = ui->edUrl->text();

    // if server not found on hidden dialog - open dialog anyway
    if (!isVisible()) {
        ensureOpened();
    }
}

void EngineStartDialog::selfSignedCertFound(int sslError)
{
    mValidSelfCertUrl = mProc->url().toString();
    setConnectionState(scsHttpsSelfSignedFound);
    mLastSslError = sslError;
    if (mInitialProtocol != ucHttp)
        ui->cbAcceptCert->setVisible(true);
}

void EngineStartDialog::forceGdxStateChanged(int state)
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
        setCanLogin(false);
    } break;
    case scsWaiting: {
        ui->laEngGamsVersion->setText("");
        ui->laEngineVersion->setText(CUnavailable);
        ui->laWarn->setText("Waiting for server ...");
        ui->laWarn->setToolTip("");
        mForcePreviousWork = false;
        setCanLogin(false);
    } break;
    case scsHttpFound: {
        ui->laWarn->setText("HTTP found.");
        ui->laWarn->setToolTip("");
    } break;
    case scsHttpsFound: {
        ui->laWarn->setText("HTTPS found.");
        ui->laWarn->setToolTip("");
    } break;
    case scsHttpsSelfSignedFound: {
        ui->laWarn->setText("Self-signed HTTPS found.");
        ui->laWarn->setToolTip("");
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
                ui->laWarn->setToolTip("Set \"previousWork=0\" to suppress this");
                mForcePreviousWork = true;
            } else {
                ui->laWarn->setText("");
                ui->laWarn->setToolTip("");
                mForcePreviousWork = false;
            }
            if (!isVisible() && mHiddenMode) {
                // hidden start
                if (mForcePreviousWork && mProc) mProc->forcePreviousWork();
                mAlways = true;
                mProc->setNamespace(ui->edNamespace->text().trimmed());
                emit submit(true);
            }
        } else {
            ui->laWarn->setText("");
            ui->laWarn->setToolTip("");
            mForcePreviousWork = false;
        }
        setCanLogin(true);
    } break;
    case scsInvalid: {
        if (!mValidSelfCertUrl.isEmpty()) {
            ui->laEngGamsVersion->setText("");
            ui->laEngineVersion->setText(CUnavailable);
            if (mLastSslError==int(QSslError::CertificateStatusUnknown))
                ui->laWarn->setText(mInitialProtocol == ucHttp ? "HTTPS found with certification error"
                                                               : "Certification error encountered");
            else
                ui->laWarn->setText(mInitialProtocol == ucHttp ? "HTTPS found with self-signed certificate"
                                                               : "Self-signed certificate found");
            if (mInitialProtocol == ucHttp)
                ui->laWarn->setToolTip("Change the URL to " + QString(mInitialProtocol == ucHttp ? "HTTPS" : "HTTP"));
            else
                ui->laWarn->setToolTip("Use checkbox below to connect anyway");
        } else {
            ui->laEngGamsVersion->setText("");
            ui->laEngineVersion->setText(CUnavailable);
            ui->laWarn->setText("No GAMS Engine server");
            ui->laWarn->setToolTip("");
        }
        mForcePreviousWork = false;
        setCanLogin(false);
    } break;
    case scsLoggedIn: {
        showSubmit();
    } break;
    }
}

void EngineStartDialog::initUrlAndChecks(QString url)
{
    mValidSelfCertUrl = "";
    mLastSslError = 0;
    mUrlChanged = true;
    mValidUrl = QString();
    mUrl = url.trimmed();
    mUrlChecks = ucAll;
    mInitialProtocol = protocol(mUrl);
    if (!mUrl.endsWith('/'))
            mUrl += '/';
    if (mUrl.endsWith("/api/", Qt::CaseInsensitive)) {
        mUrlChecks.setFlag(ucApiHttps, false);
        mUrlChecks.setFlag(ucApiHttp, false);
    }
    if (mInitialProtocol == ucHttp) {
        mUrlChecks.setFlag(ucHttp, false);
    } else {
        if (mInitialProtocol == ucNone)
            mUrl = "https://" + mUrl;
        mUrlChecks.setFlag(ucHttps, false);
        mInitialProtocol = ucHttps;
    }
    mUrl = cleanUrl(mUrl);
    mRawUrl = mUrl;
    ui->cbAcceptCert->setVisible(mProc->isIgnoreSslErrors() && protocol(mRawUrl) != ucHttp);
}

bool EngineStartDialog::fetchNextUrl()
{
    mLastSslError = 0;
    // first check for a missing "api/"
    if (!mUrlChecks.testFlag(ucHttps) && mUrlChecks.testFlag(ucApiHttps)) {
        mUrl = "https" + mRawUrl.mid(mRawUrl.indexOf("://"), mRawUrl.length()) + "api/";
        if (mUrl.contains(":443/"))
            mUrl.replace(":443/", "/");
        mUrlChecks.setFlag(ucApiHttps, false);
        return true;
    }
    if (!mUrlChecks.testFlag(ucHttp) && mUrlChecks.testFlag(ucApiHttp)) {
        mUrl = "http" + mRawUrl.mid(mRawUrl.indexOf("://"), mRawUrl.length()) + "api/";
        mUrlChecks.setFlag(ucApiHttp, false);
        return true;
    }
    // then check for the protocol
    if (mUrlChecks.testFlag(ucHttps)) {
        mUrl = cleanUrl("https" + mRawUrl.mid(mRawUrl.indexOf("://"), mRawUrl.length()));
        mUrlChecks.setFlag(ucHttps, false);
        return true;
    }
    if (mUrlChecks.testFlag(ucHttp)) {
        mUrl = cleanUrl("http" + mRawUrl.mid(mRawUrl.indexOf("://"), mRawUrl.length()));
        mUrlChecks.setFlag(ucHttp, false);
        return true;
    }
    return false;
}

EngineStartDialog::UrlCheck EngineStartDialog::protocol(QString url)
{
    if (url.startsWith("http://", Qt::CaseInsensitive))
        return ucHttp;
    if (url.startsWith("https://", Qt::CaseInsensitive))
        return ucHttps;
    return ucNone;
}

QString EngineStartDialog::cleanUrl(const QString url)
{
    QString res = url.trimmed();
    if (res.startsWith("http://", Qt::CaseInsensitive)) {
        if (res.contains(":80/"))
            res.replace(":80/", "/");
    } else if (res.startsWith("https://", Qt::CaseInsensitive)) {
        if (res.contains(":443/"))
            res.replace(":443/", "/");
    }
    return res;
}

} // namespace engine
} // namespace studio
} // namespace gams

