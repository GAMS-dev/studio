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
#ifndef ENGINESTARTDIALOG_H
#define ENGINESTARTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QTimer>

class QCompleter;
class QStringListModel;

namespace gams {
namespace studio {
namespace engine {

class EngineProcess;

namespace Ui {
class EngineStartDialog;
}

enum ServerConnectionState {
    scsNone,
    scsWaiting,
    scsHttpFound,
    scsHttpsFound,
    scsHttpsSelfSignedFound,
    scsValid,
    scsInvalid,
};

class EngineStartDialog : public QDialog
{
    Q_OBJECT
public:
    enum UrlCheck { ucNone = 0x00, ucHttps = 0x01, ucHttp = 0x02, ucApiHttps = 0x04, ucApiHttp = 0x08, ucAll = 0x0F };
    Q_DECLARE_FLAGS(UrlChecks, UrlCheck)
    Q_FLAG(UrlChecks)

public:
    explicit EngineStartDialog(QWidget *parent = nullptr);
    ~EngineStartDialog() override;
    void setHiddenMode(bool preferHidden);
    void setResume(bool resume);
    void start();

    void setProcess(EngineProcess *process);
    EngineProcess *process() const;
    void setAcceptCert();
    bool isCertAccepted();
    void initData(const QString &_url, const int authMethod, const QString &_user, const QString &_userToken, const QString &ssoName,
                  int authExpireMinutes, bool selfCert, const QString &_nSpace, const QString &_userInst, bool _forceGdx);
    bool isAlways();
    QString url() const;
    QString nSpace() const;
    QString userInstance() const;
    QString user() const;
    QString authToken() const;
    QString ssoName() const;
    bool forceGdx() const;
    void focusEmptyField();
    void setEngineVersion(const QString &version);
    int authMethod();
    QString jobTag();
    void setJobTag(const QString &jobTag);

signals:
    void submit(bool start);
    void engineUrlValidated(const QString &validUrl);
    void jobTagChanged(const QString &jobTag);
    void storeInstanceSelection();

public slots:
    void authorizeChanged(const QString &authToken);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

    void showLogin();
    void showConnect();
    void showSubmit();
    bool inLogin();
    void ensureOpened();
    void buttonClicked(QAbstractButton *button);
    void getVersionAndIP();
    void setCanLogin(bool value);
    void setConnectionState(ServerConnectionState state);
    void initUrlAndChecks(const QString &url);
    bool fetchNextUrl();
    UrlCheck protocol(const QString &url);
    QString cleanUrl(const QString &url);
    void updateUrlEdit();
    bool openInBrowser(const QString &text);
    void updatePoolControls(bool adaptCheckBox = false, bool adaptSize = false);

private slots:
    void urlEdited(const QString &text);
    void updateLoginStates();
    void updateSubmitStates();
    void bLogoutClicked();
    void authorizeError(const QString &error);
    void reGetUsername(const QString &user);
    void reGetInvitees(const QStringList &invitees);
    void reListProviderError(const QString &error);
    void showVerificationCode(const QString &userCode, const QString &verifyUri, const QString &verifyUriComplete);
    void reListJobs(qint32 count);
    void reListJobsError(const QString &error);
    void reListNamespaces(const QStringList &list);
    void reListNamespacesError(const QString &error);
    void reVersion(const QString &engineVersion, const QString &gamsVersion, bool inKubernetes);
    void reVersionError(const QString &errorText);
    void rePrioAccess(bool hasAccess);
    void reListProvider(const QList<QHash<QString, QVariant> > &allProvider);
    void reUserInstances(const QList<QPair<QString, QList<double> > > &instances, QMap<QString, QString> *poolOwners,
                         const QString &defaultLabel = QString());
    void reUserInstancesError(const QString &errorText);
    void reUpdateInstancePool();
    void reUpdateInstancePoolError(const QString &errorText);
    void quotaHint(const QStringList &diskHint, const QStringList &volumeHint);
    void forceGdxStateChanged(Qt::CheckState state);
    void updateConnectStateAppearance();
    void selfSignedCertFound(int sslError);
    void certAcceptChanged(Qt::CheckState);
    void hideCert();

    void on_cbLoginMethod_currentIndexChanged(int index);
    void on_bCopyCode_clicked();
    void on_bShowLogin_clicked();
    void on_edJobTag_editingFinished();
    void on_pbInstSend_clicked();
    void on_tbRefreshInstances_clicked();
    void on_cbInstActivate_clicked();
    void on_sbInstSize_valueChanged(int value);
    void on_cbInstance_activated(int index);

private:
    Ui::EngineStartDialog *ui;
    EngineProcess *mProc;
    QStringList mLocalGamsVersion;
    ServerConnectionState mConnectState = scsNone;
    QList<QPair<QString, QList<double>>> mInstances;
    QList<QPair<QString, QList<double>>> mInstancePools;
    QMap<QString, QString> mInstancePoolOwners;
    QMap<QString, int> mInstancePoolSize;
    int mLastInstancePoolSize = 2;
    QString mLastValidInstance;
    QString mRawUrl;
    QString mUrl;
    QString mValidUrl;
    QString mValidSelfCertUrl;
    UrlChecks mUrlChecks;
    UrlCheck mInitialProtocol = ucNone;
    bool mNoSSL = false;
    int mLastSslError = 0;
    bool mUrlChanged = false;
    bool mForcePreviousWork = true;
    bool mHiddenMode = false;
    bool mAuthorized = false;
    bool mAlways = false;
    bool mResume = false;
    int mInstanceListsLoaded = 0;
    QStringList mInvitees;
    QTimer mConnectStateUpdater;
    QTimer mUrlChangedTimer;
    QString mEngineVersion;
    QString mGamsVersion;
    int mAuthExpireMinutes = 60*2;
    int mAuthIndex = 0;

    static const QString CUnavailable;
    static QRegularExpression mRexVersion;

};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // ENGINESTARTDIALOG_H
