/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include <QClipboard>
#include <QDesktopServices>

namespace gams {
namespace studio {
namespace engine {

const QString EngineStartDialog::CUnavailable("-");
QRegularExpression EngineStartDialog::mRexVersion("GAMS Release\\s*:\\s+(\\d\\d\\.\\d).*");

EngineStartDialog::EngineStartDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::EngineStartDialog), mProc(nullptr)
{
    ui->setupUi(this);
    ui->laSsoLink->installEventFilter(this);
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
    connect(ui->edSsoName, &QLineEdit::textChanged, this, &EngineStartDialog::updateLoginStates);
    connect(ui->edToken, &QPlainTextEdit::textChanged, this, [this]() { updateLoginStates(); });
    connect(ui->bLogout, &QPushButton::clicked, this, &EngineStartDialog::bLogoutClicked);
    connect(ui->cbForceGdx, &QCheckBox::stateChanged, this, &EngineStartDialog::forceGdxStateChanged);
    connect(ui->cbAcceptCert, &QCheckBox::stateChanged, this, &EngineStartDialog::certAcceptChanged);

    ui->stackedWidget->setCurrentIndex(0);
    ui->cbLoginMethod->setCurrentIndex(0);
    ui->stackLoginInput->setCurrentIndex(0);
    ui->bAlways->setVisible(false);
    ui->cbAcceptCert->setVisible(false);
    ui->bOk->setText("Login");
    ui->cbInstance->setVisible(false);
    ui->laInstance->setVisible(false);
    ui->laAvailable->setVisible(false);
    ui->laAvailDisk->setVisible(false);
    ui->laAvailVolume->setVisible(false);
    ui->laAvailVolSec->setVisible(false);
    ui->ssoPane->setVisible(false);

    if (Theme::instance()->baseTheme(Theme::instance()->activeTheme()) != 0)
        ui->laLogo->setPixmap(QPixmap(QString::fromUtf8(":/img/engine-logo-w")));
    GamsProcess gp;
    QString about = gp.aboutGAMS();
    QRegularExpressionMatch match = mRexVersion.match(about);
    if (match.hasMatch())
        mLocalGamsVersion = match.captured(1).split('.');
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

void EngineStartDialog::authorizeChanged(const QString &authToken)
{
    Q_UNUSED(authToken)
    if (mProc->authToken().isEmpty())
        showLogin();
    else
        mProc->listJobs();
}

void EngineStartDialog::setHiddenMode(bool preferHidden)
{
    mHiddenMode = preferHidden || mResume;
}

void EngineStartDialog::setResume(bool resume)
{
    mResume = resume;
    if (mProc && mProc->setUrl(mUrl)) {
        bool visibleCheck = ui->cbAcceptCert->isVisible() || !inLogin();
        if (protocol(mUrl) == ucHttps && visibleCheck && ui->cbAcceptCert->isChecked())
            mProc->setIgnoreSslErrorsCurrentUrl(true);
        mUrlChanged = false;
        mProc->getVersion();
        mProc->listProvider();
        return;
    }

}

void EngineStartDialog::start()
{
    if (!mProc) return;
    if (ui->edUrl->text().isEmpty() || mResume)
        showLogin();
    else {
        showConnect();
        urlEdited(ui->edUrl->text());
    }
}

void EngineStartDialog::setProcess(EngineProcess *process)
{
    mProc = process;
    connect(mProc, &EngineProcess::authorized, this, &EngineStartDialog::authorizeChanged);
    connect(mProc, &EngineProcess::reListProviderError, this, &EngineStartDialog::reListProviderError);
    connect(mProc, &EngineProcess::showVerificationCode, this, &EngineStartDialog::showVerificationCode);
    connect(mProc, &EngineProcess::authorizeError, this, &EngineStartDialog::authorizeError);
    connect(mProc, &EngineProcess::reGetUsername, this, &EngineStartDialog::reGetUsername);
    connect(mProc, &EngineProcess::reListJobs, this, &EngineStartDialog::reListJobs);
    connect(mProc, &EngineProcess::reListJobsError, this, &EngineStartDialog::reListJobsError);
    connect(mProc, &EngineProcess::reListNamspaces, this, &EngineStartDialog::reListNamespaces);
    connect(mProc, &EngineProcess::reListNamespacesError, this, &EngineStartDialog::reListNamespacesError);
    connect(mProc, &EngineProcess::reVersion, this, &EngineStartDialog::reVersion);
    connect(mProc, &EngineProcess::reListProvider, this, &EngineStartDialog::reListProvider);
    connect(mProc, &EngineProcess::reVersionError, this, &EngineStartDialog::reVersionError);
    connect(mProc, &EngineProcess::reUserInstances, this, &EngineStartDialog::reUserInstances);
    connect(mProc, &EngineProcess::reUserInstancesError, this, &EngineStartDialog::reUserInstancesError);
    connect(mProc, &EngineProcess::quotaHint, this, &EngineStartDialog::quotaHint);
    connect(mProc, &EngineProcess::sslSelfSigned, this, &EngineStartDialog::selfSignedCertFound);
    mProc->setForceGdx(ui->cbForceGdx->isChecked());
    mProc->initUsername(user());
    mProc->setNamespace(ui->cbNamespace->currentText());
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

void EngineStartDialog::initData(const QString &_url, const int authMethod, const QString &_user, const QString &_userToken, const QString &ssoName,
                                 int authExpireMinutes, bool selfCert, const QString &_nSpace, const QString &_userInst, bool _forceGdx)
{
    mUrl = cleanUrl(_url);
    ui->edUrl->setText(mUrl);
    ui->nUrl->setText(mUrl);
    mAuthIndex = authMethod;
    ui->cbLoginMethod->setCurrentIndex(qMin(authMethod, 2));
    ui->stackLoginInput->setCurrentIndex(qMin(authMethod, 2));
    if (mProc && authMethod == 0) mProc->initUsername(_user.trimmed());
    if (mProc && authMethod == 1) mProc->setAuthToken(_userToken.trimmed());
    if (mProc && authMethod == 2) {
        ui->edSsoName->setText(ssoName.trimmed());
    }
    ui->edUser->setText(_user.trimmed());
    ui->nUser->setText(_user.trimmed());
    if (selfCert) {
        ui->cbAcceptCert->setVisible(true);
        ui->cbAcceptCert->setChecked(true);
    }
    mAuthExpireMinutes = authExpireMinutes;
    ui->cbNamespace->addItem(_nSpace.trimmed());
    ui->cbInstance->addItem(_userInst.trimmed(), QVariantList() << _userInst.trimmed());
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

QString EngineStartDialog::userInstance() const
{
    QVariantList datList = ui->cbInstance->currentData().toList();
    return  datList.count() ? datList.first().toString() : "";
}

QString EngineStartDialog::user() const
{
    return ui->edUser->text().trimmed();
}

QString EngineStartDialog::authToken() const
{
    if (mProc) return mProc->authToken();
    return ui->edToken->document()->toPlainText().trimmed();
}

QString EngineStartDialog::ssoName() const
{
    return ui->edSsoName->text().trimmed();
}

bool EngineStartDialog::forceGdx() const
{
    return ui->cbForceGdx->isChecked();
}

void EngineStartDialog::focusEmptyField()
{
    if (inLogin()) {
        int method = ui->cbLoginMethod->currentIndex();
        if (ui->edUrl->text().isEmpty()) ui->edUrl->setFocus();
        else if (method == 2 && ui->edSsoName->text().isEmpty())
            ui->edSsoName->setFocus();
        else if (method == 1 && authToken().isEmpty())
            ui->edToken->setFocus();
        else if (method == 0 && user().isEmpty())
            ui->edUser->setFocus();
        else if (method == 0 && ui->edPassword->isVisible() && ui->edPassword->text().isEmpty())
            ui->edPassword->setFocus();

        else ui->bOk->setFocus();
    } else {
        if (ui->cbNamespace->count() > 1) ui->cbNamespace->setFocus();
        else ui->bOk->setFocus();
    }
}

void EngineStartDialog::setEngineVersion(const QString &version)
{
    ui->laEngineVersion->setText(version);
}

int EngineStartDialog::authMethod()
{
    return qBound(0, ui->cbLoginMethod->currentIndex(), ui->cbLoginMethod->count());
}

QString EngineStartDialog::jobTag()
{
    return ui->edJobTag->text();
}

void EngineStartDialog::setJobTag(const QString &jobTag)
{
    ui->edJobTag->setText(jobTag);
}

bool EngineStartDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->edUrl && event->type() == QEvent::FocusOut)
        updateUrlEdit();
    if (watched == ui->laSsoLink && event->type() == QEvent::MouseButtonRelease)
        openInBrowser(ui->laSsoLink->text());
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
    if (mProc->authToken().isEmpty() || mResume)
        showLogin();
    else
        showConnect();
    QDialog::showEvent(event);
    setFixedSize(size());
    if (isHidden)
        ui->cbAcceptCert->setVisible(false);
}

void EngineStartDialog::showLogin()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->bAlways->setVisible(false);
    setWindowTitle("GAMS Engine Login");
    ui->bOk->setText("Login");
    ensureOpened();
}

void EngineStartDialog::showConnect()
{
    setWindowTitle("Connecting");
    ui->stackedWidget->setCurrentIndex(2);
}

void EngineStartDialog::showSubmit()
{
    if (mResume) {
        close();
        return;
    }
    setWindowTitle("Submit Job");
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
    ui->laAvailVolSec->setVisible(mProc->inKubernetes());
    updateSubmitStates();
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
    ui->edToken->document()->clear();
    mProc->setAuthToken("");
    mProc->initUsername("");
    emit mProc->authorized("");
    showLogin();
}

void EngineStartDialog::authorizeError(const QString &error)
{
    if (mResume) return;
    if (mProc->authToken().isEmpty()) {
        ui->laWarn->setText("Could not log in: Please retry");
        ui->laWarn->setToolTip(error.trimmed());
    } else {
        ui->laWarn->setText("Could not log in: Session expired");
        ui->laWarn->setToolTip(error.trimmed());
    }
    showLogin();
}

void EngineStartDialog::reGetUsername(const QString &user)
{
    ui->edUser->setText(user);
    showSubmit();
}

void EngineStartDialog::reListProviderError(const QString &error)
{
    ui->laWarn->setText("Error: " + error.trimmed());
    ui->laWarn->setToolTip("Please select a valid provider");
}

void EngineStartDialog::showVerificationCode(const QString &userCode, const QString &verifyUri, const QString &verifyUriComplete)
{
    Q_UNUSED(verifyUriComplete)
    ui->laSsoCode->setText(userCode.isEmpty() ? "[missing Code]" : userCode);
    ui->laSsoLink->setText(verifyUri.isEmpty() ? "[missing Link]" : verifyUri);

    bool ok = !userCode.isEmpty() && !verifyUri.isEmpty();
    ui->ssoPane->setVisible(ok);
}

void EngineStartDialog::buttonClicked(QAbstractButton *button)
{
    if (!mProc) return;
    ui->bOk->setEnabled(false);
    ui->bAlways->setEnabled(false);

    if (inLogin() && button == ui->bOk) {
        switch (authMethod()) {
        case 0:
            mProc->authorize(user(), ui->edPassword->text(), mAuthExpireMinutes);
            break;
        case 1:
            mProc->authorize(ui->edToken->document()->toPlainText().trimmed());
            break;
        case 2: {
            // SSO authorization
            mProc->listProvider(ui->edSsoName->text().trimmed());
        }   break;
        default:
            mProc->authorizeProviderName(ui->cbLoginMethod->currentData().toHash().value("name").toString());
            break;
        }
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
    mProc->setJobTag(ui->edJobTag->text().trimmed());
    mProc->setNamespace(ui->cbNamespace->currentText());
    emit submit(start);
}

void EngineStartDialog::getVersionAndIP()
{
    setConnectionState(scsWaiting);
    if (mProc) {
        if (mProc->setUrl(mUrl)) {
            bool visibleCheck = ui->cbAcceptCert->isVisible() || !inLogin();
            if (protocol(mUrl) == ucHttps && visibleCheck && ui->cbAcceptCert->isChecked())
                mProc->setIgnoreSslErrorsCurrentUrl(true);
            mUrlChanged = false;
            mProc->getVersion();
            mProc->listProvider();
            return;
        }
    }
    mConnectState = scsNone;
    updateConnectStateAppearance();
}

void EngineStartDialog::setCanLogin(bool value)
{
    value = value && !ui->edUrl->text().isEmpty()
            && (!mProc->authToken().isEmpty() ||
                  (authMethod()==0 && !user().isEmpty() && (!ui->edPassword->text().isEmpty()))
               || (authMethod()==1 && !ui->edToken->document()->toPlainText().isEmpty())
               || (authMethod()==2 && !ui->edSsoName->text().isEmpty())
               || (authMethod() > 2))
            && (!ui->cbAcceptCert->isVisible() || ui->cbAcceptCert->isChecked());
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
    getVersionAndIP();
    if (!isVisible() && !mHiddenMode && !inLogin())
        showLogin();
}

void EngineStartDialog::updateLoginStates()
{
    bool enabled = !ui->edUrl->text().isEmpty() && !user().isEmpty() && !ui->edPassword->text().isEmpty();
    if (enabled && ui->laEngineVersion->text() == CUnavailable) {
        getVersionAndIP();
    }
    setConnectionState(mConnectState);
}

void EngineStartDialog::updateSubmitStates()
{
    bool value = !ui->cbNamespace->currentText().isEmpty() && mAuthorized &&
            (!mProc->inKubernetes() || (ui->cbInstance->count() && ui->cbInstance->currentData().toList().size() == 4));
    if (value != ui->bOk->isEnabled()) {
        ui->bOk->setEnabled(value);
        ui->bAlways->setEnabled(value);
    }
    setConnectionState(mConnectState);
}

void EngineStartDialog::reListJobs(qint32 count)
{
    ui->nJobCount->setText(QString::number(count));
    mAuthorized = true;
    if (mResume) {
        mResume = false;
        close();
        return;
    }
    showSubmit();
    mProc->sendPostLoginRequests();
}

void EngineStartDialog::reListJobsError(const QString &error)
{
    DEB() << "ERROR: " << error;
    if (!inLogin()) {
        showLogin();
    } else if (authMethod() == 1) {
        ui->laWarn->setText("Invalid access token");
        ui->edToken->setFocus();
    }
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
        ui->cbNamespace->setCurrentIndex(int(list.indexOf(text)));
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
            if (!mResume)
                showConnect();
            mProc->getUsername();
        }
        emit engineUrlValidated(mValidUrl);
    }
}

void EngineStartDialog::reVersionError(const QString &errorText)
{
    if (!mValidUrl.isEmpty()) return;
    Q_UNUSED(errorText)

    if (mUrlChanged) {
        getVersionAndIP();
        return;
    }
    // if the raw input failed, try next protocol/api combination
    if (!mLastSslError && fetchNextUrl()) {
        getVersionAndIP();
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

void EngineStartDialog::reListProvider(const QList<QHash<QString, QVariant> > &allProvider)
{
    int cbIndex = 3;
    for (const QHash<QString, QVariant> &provider : allProvider) {
        QString label = provider.value("label").toString();
        if (ui->cbLoginMethod->count() == cbIndex) {
            ui->cbLoginMethod->addItem(label, provider);
        } else if (ui->cbLoginMethod->itemText(cbIndex) != label) {
            ui->cbLoginMethod->setItemText(cbIndex, label);
            ui->cbLoginMethod->setItemData(cbIndex, provider);
        }
        ++cbIndex;
    }
    while (ui->cbLoginMethod->count() > cbIndex)
        ui->cbLoginMethod->removeItem(cbIndex);
    if (mAuthIndex >= 0 && ui->cbLoginMethod->currentIndex() != mAuthIndex && ui->cbLoginMethod->count() > mAuthIndex) {
        ui->cbLoginMethod->setCurrentIndex(mAuthIndex);
        mAuthIndex = -1;
    }
}

void EngineStartDialog::reUserInstances(const QList<QPair<QString, QList<double> > > &instances, const QString &defaultLabel)
{
    QVariantList datList = ui->cbInstance->currentData().toList();
    QString lastInst = datList.count() ? datList.first().toString() : "";
    ui->cbInstance->clear();
    int cur = 0;
    int prev = -1;
    for (const QPair<QString, QList<double> > &entry : instances) {
        if (entry.second.size() != 3) continue;
        if (entry.first == defaultLabel) cur = ui->cbInstance->count();
        QString text("%1 (%2 vCPU, %3 GB RAM, %4x)");
        text = text.arg(entry.first).arg(entry.second[0]).arg(entry.second[1]).arg(entry.second[2]);
        QVariantList data;
        data << entry.first << entry.second[0] << entry.second[1] << entry.second[2];
        if (entry.first == lastInst) prev = ui->cbInstance->count();
        ui->cbInstance->addItem(text, data);
    }
    if (!ui->cbInstance->count()) {
        ui->cbInstance->addItem("-no instances-");
        ui->laWarn->setText("No user instances assigned");
        ui->laWarn->setToolTip("Please contact your administrator");
    }
    if (prev >= 0)
        ui->cbInstance->setCurrentIndex(prev);
    else if (ui->cbInstance->count())
        ui->cbInstance->setCurrentIndex(cur);
    updateSubmitStates();
}

void EngineStartDialog::reUserInstancesError(const QString &errorText)
{
    ui->cbInstance->clear();
    ui->cbInstance->addItem("-no instances-");
    ui->laWarn->setText("Error reading user instances: " + errorText);
    ui->laWarn->setToolTip("Try to login again or contact your administrator");
}

void EngineStartDialog::quotaHint(const QStringList &diskHint, const QStringList &volumeHint)
{
    if (diskHint.isEmpty() && volumeHint.isEmpty()) {
        ui->laAvailDisk->setVisible(true);
        ui->laAvailDisk->setText("unlimited");
        ui->laAvailDisk->setToolTip("");
        ui->laAvailVolume->setVisible(false);
        ui->laAvailVolSec->setVisible(false);
        return;
    }
    ui->laAvailDisk->setVisible(diskHint.size() > 0);
    if (ui->laAvailDisk->isVisible()) {
        ui->laAvailDisk->setText(diskHint.at(0));
        ui->laAvailDisk->setToolTip(diskHint.size() > 1 ? "Limited by " + diskHint.at(1) : "");
    }
    ui->laAvailVolume->setVisible(volumeHint.size() > 0);
    if (ui->laAvailVolume->isVisible()) {
        ui->laAvailVolume->setText(volumeHint.at(0));
        ui->laAvailVolSec->setVisible(volumeHint.size() > 2);
        if (ui->laAvailVolSec->isVisible())
            ui->laAvailVolSec->setText(volumeHint.at(1));
        ui->laAvailVolume->setToolTip(volumeHint.size() > 2 ? "Limited by " + volumeHint.at(2) : "");
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
            if (!isVisible() && mHiddenMode && !mResume) {
                // hidden start
                if (mForcePreviousWork && mProc) mProc->forcePreviousWork();
                mAlways = true;
//                mProc->setNamespace(ui->cbNamespace->currentText().trimmed());
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

void EngineStartDialog::initUrlAndChecks(const QString &url)
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

EngineStartDialog::UrlCheck EngineStartDialog::protocol(const QString &url)
{
    if (url.startsWith("http://", Qt::CaseInsensitive))
        return ucHttp;
    if (url.startsWith("https://", Qt::CaseInsensitive))
        return ucHttps;
    return ucNone;
}

QString EngineStartDialog::cleanUrl(const QString &url)
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


void EngineStartDialog::on_cbLoginMethod_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    if (ui->cbLoginMethod->currentIndex() < 2) {
        ui->stackLoginInput->setCurrentIndex(ui->cbLoginMethod->currentIndex());
    } else {
        ui->stackLoginInput->setCurrentIndex(2);
        ui->laSso->setVisible(ui->cbLoginMethod->currentIndex() == 2);
        ui->edSsoName->setVisible(ui->cbLoginMethod->currentIndex() == 2);

        setCanLogin(ui->cbLoginMethod->currentIndex() > 2 || !ui->edSsoName->text().isEmpty());
    }
    if (!mValidUrl.isEmpty())
        emit engineUrlValidated(mValidUrl);
}

bool EngineStartDialog::openInBrowser(const QString &text)
{
    if (text.isEmpty() || !text.startsWith("http")) {
        DEB() << "invalid link";
        return false;
    }
    QDesktopServices::openUrl(QUrl(text));
    return true;
}

void EngineStartDialog::on_bCopyCode_clicked()
{
    if (!openInBrowser(ui->laSsoLink->text()))
        return;
    if (ui->laSsoCode->text().isEmpty() || ui->laSsoCode->text().startsWith("[")) {
        DEB() << "missing code";
        return;
    }
    QClipboard *clip = QGuiApplication::clipboard();
    QString text = ui->laSsoCode->text();
    QTimer::singleShot(10, this, [text, clip]() {
        clip->setText(text);
    });
}

void EngineStartDialog::on_bShowLogin_clicked()
{
    mProc->abortRequests();
    showLogin();
}


void EngineStartDialog::on_cbInstance_currentIndexChanged(int index)
{
    if (!mProc) return;
    double parallel = 0;
    if (ui->cbInstance->itemData(index).canConvert(QMetaType(QMetaType::QVariantList))) {
        QVariantList list = ui->cbInstance->itemData(index).toList();
        bool ok = false;
        if (list.size() > 3)
            parallel = list.at(3).toDouble(&ok);
        if (!ok) parallel = -1;
    }

    mProc->updateQuota(parallel);
}


void EngineStartDialog::on_edJobTag_editingFinished()
{
    emit jobTagChanged(ui->edJobTag->text().trimmed());
}


} // namespace engine
} // namespace studio
} // namespace gams

