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
    void start();

    void setProcess(EngineProcess *process);
    EngineProcess *process() const;
    void setAcceptCert();
    bool isCertAccepted();
    void initData(const QString &_url, const QString &_user, int authExpireMinutes, bool selfCert, const QString &_nSpace, bool _forceGdx);
    bool isAlways();
    QString url() const;
    QString nSpace() const;
    QString user() const;
    QString authToken() const;
    bool forceGdx() const;
    void focusEmptyField();
    void setEngineVersion(QString version);

signals:
    void submit(bool start);

public slots:
    void authorizeChanged(QString authToken);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

    void showLogin();
    void showSubmit();
    bool inLogin();
    void ensureOpened();
    void buttonClicked(QAbstractButton *button);
    void getVersion();
    void setCanLogin(bool value);
    void setCanSubmit(bool value);
    void setConnectionState(ServerConnectionState state);
    void initUrlAndChecks(QString url);
    bool fetchNextUrl();
    UrlCheck protocol(QString url);
    QString cleanUrl(const QString url);
    void updateUrlEdit();

private slots:
    void urlEdited(const QString &text);
    void updateLoginStates();
    void updateSubmitStates();
    void bLogoutClicked();
    void authorizeError(const QString &error);
    void reListJobs(qint32 count);
    void reListJobsError(const QString &error);
    void reListNamespaces(const QStringList &list);
    void reListNamespacesError(const QString &error);
    void reVersion(const QString &engineVersion, const QString &gamsVersion, bool inKubernetes);
    void reVersionError(const QString &errorText);
    void reUserInstances(const QList<QPair<QString, QList<int> > > instances, const QString &defaultLabel);
    void reUserInstancesError(const QString &errorText);
    void quotaHint(const QStringList &diskHint, const QStringList &volumeHint);
    void forceGdxStateChanged(int state);
    void updateConnectStateAppearance();
    void selfSignedCertFound(int sslError);
    void certAcceptChanged();
    void hideCert();

private:
    Ui::EngineStartDialog *ui;
    EngineProcess *mProc;
    QStringList mLocalGamsVersion;
    ServerConnectionState mConnectState = scsNone;
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
    QTimer mConnectStateUpdater;
    QTimer mUrlChangedTimer;
    QString mEngineVersion;
    QString mGamsVersion;
    int mAuthExpireMinutes = 60*2;

    static const QString CUnavailable;

};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // ENGINESTARTDIALOG_H
