/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include <QSslSocket>

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
    connect(ui->bOk, &QPushButton::clicked, this, [this]() { buttonClicked(ui->bOk); });
    connect(ui->bCancel, &QPushButton::clicked, this, [this]() { buttonClicked(ui->bCancel); });
    connect(ui->bAlways, &QPushButton::clicked, this, [this]() { buttonClicked(ui->bAlways); });

    connect(ui->edUrl, &QLineEdit::textEdited, this, [this]() { mUrlChangedTimer.start(); });
    connect(ui->edUrl, &QLineEdit::textChanged, this, &EngineStartDialog::updateLoginStates);
    connect(ui->cbNamespace, &QComboBox::currentTextChanged, this, &EngineStartDialog::updateSubmitStates);
    connect(ui->edUser, &QLineEdit::textChanged, this, &EngineStartDialog::updateLoginStates);
    connect(ui->edPassword, &QLineEdit::textChanged, this, &EngineStartDialog::updateLoginStates);
    connect(ui->bLogout, &QPushButton::clicked, this, &EngineStartDialog::bLogoutClicked);
    connect(ui->cbForceGdx, &QCheckBox::stateChanged, this, &EngineStartDialog::forceGdxStateChanged);
    connect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);

    ui->stackedWidget->setCurrentIndex(0);
    ui->bAlways->setVisible(false);
    ui->cbAcceptCert->setVisible(false);
    ui->bOk->setText("Login");
    ui->cbInstance->setVisible(false);
    ui->laInstance->setVisible(false);
    ui->laAvailable->setVisible(false);
    ui->laAvailDisk->setVisible(false);
    ui->laAvailVolume->setVisible(false);

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
    else {
        showSubmit();
        urlEdited(ui->edUrl->text());
    }
}

void EngineStartDialog::setProcess(EngineProcess *process)
{
    mProc = process;
    connect(mProc, &EngineProcess::authorized, this, &EngineStartDialog::authorizeChanged);
    connect(mProc, &EngineProcess::authorizeError, this, &EngineStartDialog::authorizeError);
    connect(mProc, &EngineProcess::reListJobs, this, &EngineStartDialog::reListJobs);
    connect(mProc, &EngineProcess::reListJobsError, this, &EngineStartDialog::reListJobsError);
    connect(mProc, &EngineProcess::reListNamspaces, this, &EngineStartDialog::reListNamespaces);
    connect(mProc, &EngineProcess::reListNamespacesError, this, &EngineStartDialog::reListNamespacesError);
    connect(mProc, &EngineProcess::reVersion, this, &EngineStartDialog::reVersion);
    connect(mProc, &EngineProcess::reVersionError, this, &EngineStartDialog::reVersionError);
    connect(mProc, &EngineProcess::reUserInstances, this, &EngineStartDialog::reUserInstances);
    connect(mProc, &EngineProcess::reUserInstancesError, this, &EngineStartDialog::reUserInstancesError);
    connect(mProc, &EngineProcess::quotaHint, this, &EngineStartDialog::quotaHint);
    connect(mProc, &EngineProcess::sslSelfSigned, this, &EngineStartDialog::selfSignedCertFound);
    mProc->setForceGdx(ui->cbForceGdx->isChecked());
    mProc->initUsername(user());
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

void EngineStartDialog::initData(const QString &_url, const QString &_user, int authExpireMinutes, bool selfCert, const QString &_nSpace, bool _forceGdx)
{
    mUrl = cleanUrl(_url);
    ui->edUrl->setText(mUrl);
    ui->nUrl->setText(mUrl);
    if (mProc) mProc->initUsername(_user.trimmed());
    ui->edUser->setText(_user.trimmed());
    ui->nUser->setText(_user.trimmed());
    if (selfCert) {
        ui->cbAcceptCert->setVisible(true);
        ui->cbAcceptCert->setChecked(true);
    }
    mAuthExpireMinutes = authExpireMinutes;
    ui->cbNamespace->setCurrentText(_nSpace.trimmed());
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
    return ui->cbNamespace->currentText();
}

QString EngineStartDialog::user() const
{
    return ui->edUser->text().trimmed();
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
    if (inLogin()) {
        if (ui->edUrl->text().isEmpty()) ui->edUrl->setFocus();
        else if (user().isEmpty()) ui->edUser->setFocus();
        else if (ui->edPassword->text().isEmpty()) ui->edPassword->setFocus();
        else ui->bOk->setFocus();
    } else {
        if (ui->cbNamespace->currentText().isEmpty()) ui->cbNamespace->setFocus();
        else ui->bOk->setFocus();
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
    ui->bOk->setText("Login");
    ensureOpened();
}

void EngineStartDialog::showSubmit()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->bAlways->setVisible(true);
    ui->bOk->setText("OK");
    ui->nUser->setText(user());
    ui->nUrl->setText(mProc->url().toString());
    ui->laInstance->setVisible(mProc->inKubernetes());
    ui->cbInstance->setVisible(mProc->inKubernetes());
    ui->laAvailable->setVisible(mProc->inKubernetes());
    ui->laAvailDisk->setVisible(mProc->inKubernetes());
    ui->laAvailVolume->setVisible(mProc->inKubernetes());
    setCanSubmit(true);
    if (!mHiddenMode)
        ensureOpened();
}

bool EngineStartDialog::inLogin()
{
    return ui->stackedWidget->currentIndex() == 0;
}

void EngineStartDialog::ensureOpened()
{
    if (!isVisible()) {
        mHiddenMode = false;
        open();
    }
    focusEmptyField();
}

void EngineStartDialog::bLogoutClicked()
{
    ui->edPassword->setText("");
    mProc->setAuthToken("");
    mProc->initUsername("");
    emit mProc->authorized("");
    showLogin();
}

void EngineStartDialog::authorizeError(const QString &error)
{
    ui->laWarn->setText("Could not log in: " + error.trimmed());
    ui->laWarn->setToolTip("Please check your username and password");
}

void EngineStartDialog::buttonClicked(QAbstractButton *button)
{
    if (!mProc) return;
    ui->bOk->setEnabled(false);
    ui->bAlways->setEnabled(false);

    if (inLogin() && button == ui->bOk) {
        mProc->authorize(user(), ui->edPassword->text(), mAuthExpireMinutes);
        return;
    }
    mAlways = button == ui->bAlways;
    bool start = mAlways || button == ui->bOk;
    if (mForcePreviousWork && mProc) mProc->forcePreviousWork();
    if (!start) {
        disconnect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);
        ui->cbAcceptCert->setChecked(false);
        connect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);
    }
    if (mProc->inKubernetes()) {
        QVariantList data = ui->cbInstance->currentData().toList();
        if (data.size() == 4)
            mProc->setSelectedInstance(data.first().toString());
    }
    mProc->setNamespace(ui->cbNamespace->currentText().trimmed());
    emit submit(start);
}

void EngineStartDialog::getVersion()
{
    setConnectionState(scsWaiting);
    if (mProc) {
        if (mProc->setUrl(mUrl)) {
            bool visibleCheck = ui->cbAcceptCert->isVisible() || !inLogin();
            if (protocol(mUrl) == ucHttps && visibleCheck && ui->cbAcceptCert->isChecked())
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
            && !user().isEmpty()
            && (!ui->edPassword->text().isEmpty() || !mProc->authToken().isEmpty())
            && (!ui->cbAcceptCert->isVisible() || ui->cbAcceptCert->isChecked());
    if (value != ui->bOk->isEnabled()) {
        ui->bOk->setEnabled(value);
        ui->bAlways->setEnabled(value);
    }
}

void EngineStartDialog::setCanSubmit(bool value)
{
    //TODO(JM) regard kubernetes
    value = value && !ui->cbNamespace->currentText().isEmpty() && mAuthorized;
    if (value != ui->bOk->isEnabled()) {
        ui->bOk->setEnabled(value);
        ui->bAlways->setEnabled(value);
    }
}

void EngineStartDialog::setConnectionState(ServerConnectionState state)
{
    mConnectState = state;
    mConnectStateUpdater.start();
}

void EngineStartDialog::certAcceptChanged()
{
    if (!mProc) return;
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
    if (!isVisible() && !mHiddenMode && !inLogin())
        showLogin();
}

void EngineStartDialog::updateLoginStates()
{
    bool enabled = !ui->edUrl->text().isEmpty() && !user().isEmpty() && !ui->edPassword->text().isEmpty();
    if (enabled && ui->laEngineVersion->text() == CUnavailable) {
        getVersion();
    }
    setConnectionState(mConnectState);
}

void EngineStartDialog::updateSubmitStates()
{
    bool enabled = !ui->cbNamespace->currentText().isEmpty();
    setCanSubmit(enabled);
    setConnectionState(mConnectState);
}

void EngineStartDialog::reListJobs(qint32 count)
{
    ui->nJobCount->setText(QString::number(count));
    mAuthorized = true;
    showSubmit();
    mProc->sendPostLoginRequests();
}

void EngineStartDialog::reListJobsError(const QString &error)
{
    DEB() << "ERROR: " << error;
    if (!inLogin())
        showLogin();
}

void EngineStartDialog::reListNamespaces(const QStringList &list)
{
    QString text = ui->cbNamespace->currentText().trimmed();
    ui->cbNamespace->clear();
    if (list.isEmpty()) {
        ui->cbNamespace->addItem("-no namespace-", "SKIP");
        ui->cbNamespace->setToolTip("");
        return;
    }
    ui->cbNamespace->addItems(list);
    if (!text.isEmpty() && list.contains(text))
        ui->cbNamespace->setCurrentIndex(list.indexOf(text));
    else
        ui->cbNamespace->setCurrentIndex(0);
    if (list.size() == 1)
        ui->cbNamespace->setToolTip("This is the only namespace with permissions");
    else
        ui->cbNamespace->setToolTip(QString::number(list.size())+" namespaces with permissions");
}

void EngineStartDialog::reListNamespacesError(const QString &error)
{
    ui->cbNamespace->clear();
    ui->cbNamespace->addItem("-no namespace-", "SKIP");
    ui->cbNamespace->setToolTip("");
    ui->laWarn->setText("Could not read namespaces" + error.trimmed());
    ui->laWarn->setToolTip("Try to login again or contact your administrator");
}

void EngineStartDialog::reVersion(const QString &engineVersion, const QString &gamsVersion, bool inKubernetes)
{
    Q_UNUSED(inKubernetes)
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
        else {
            showSubmit();
            mProc->listJobs();
        }
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

void EngineStartDialog::reUserInstances(const QList<QPair<QString, QList<int> > > instances, const QString &defaultLabel)
{
    ui->cbInstance->clear();
    int cur = 0;
    for (const QPair<QString, QList<int> > &entry : instances) {
        if (entry.second.size() != 3) continue;
        if (entry.first == defaultLabel) cur = ui->cbInstance->count();
        QString text("%1 (%2 vCPU, %3 MiB RAM, %4x)");
        text = text.arg(entry.first).arg(entry.second[0]).arg(entry.second[1]).arg(entry.second[2]);
        QVariantList data;
        data << entry.first << entry.second[0] << entry.second[1] << entry.second[2];
        ui->cbInstance->addItem(text, data);
    }
    if (ui->cbInstance->count())
        ui->cbInstance->setCurrentIndex(cur);
    ui->cbInstance->setToolTip("");
}

void EngineStartDialog::reUserInstancesError(const QString &errorText)
{
    ui->cbInstance->clear();
    ui->cbInstance->addItem("-no instances defined-");
    ui->cbInstance->setToolTip(errorText);
}

void EngineStartDialog::quotaHint(const QStringList &diskHint, const QStringList &volumeHint)
{
    ui->laAvailDisk->setVisible(diskHint.size() > 1);
    if (ui->laAvailDisk->isVisible()) {
        ui->laAvailDisk->setText(diskHint.at(0));
        ui->laAvailDisk->setToolTip("Limited by " + diskHint.at(1));
    }
    if (ui->laAvailVolume->isVisible()) {
        ui->laAvailVolume->setText(volumeHint.at(0));
        ui->laAvailVolume->setToolTip("Limited by " + volumeHint.at(1));
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
        if (mNoSSL) {
            ui->laWarn->setText("SSL not supported on this machine.");
            ui->laWarn->setToolTip("Maybe the GAMSDIR variable doesn't point to the GAMS installation path.");
        } else {
            ui->laWarn->setText("No GAMS Engine server");
            ui->laWarn->setToolTip("");
        }
        mForcePreviousWork = false;
        setCanLogin(false);
    } break;
    case scsWaiting: {
        ui->laEngGamsVersion->setText("");
        ui->laEngineVersion->setText(CUnavailable);
        if (mNoSSL) {
            ui->laWarn->setText("SSL not supported on this machine.");
            ui->laWarn->setToolTip("Maybe the GAMSDIR variable doesn't point to the GAMS installation path.");
        } else {
            ui->laWarn->setText("Waiting for server ...");
            ui->laWarn->setToolTip("");
        }
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
                mProc->setNamespace(ui->cbNamespace->currentText().trimmed());
                emit submit(true);
            }
        } else {
            ui->laWarn->setText("");
            ui->laWarn->setToolTip("");
            mForcePreviousWork = false;
        }
        if (inLogin()) setCanLogin(true);
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
            if (mNoSSL) {
                ui->laWarn->setText("SSL not supported on this machine.");
                ui->laWarn->setToolTip("Maybe the GAMSDIR variable doesn't point to the GAMS installation path.");
            } else {
                ui->laWarn->setText("No GAMS Engine server");
                ui->laWarn->setToolTip("");
            }
        }
        mForcePreviousWork = false;
        setCanLogin(false);
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
    mNoSSL = !QSslSocket::supportsSsl() && mInitialProtocol == ucHttps;
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
    if (!QSslSocket::supportsSsl()) {
        mUrlChecks.setFlag(ucApiHttps, false);
        mUrlChecks.setFlag(ucHttps, false);
    }
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

