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
    scsValid,
    scsInvalid
};

class EngineStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineStartDialog(QWidget *parent = nullptr);
    ~EngineStartDialog() override;
    void hiddenCheck();

    void setProcess(EngineProcess *process);
    EngineProcess *process() const;
    QString url() const;
    QString nSpace() const;
    QString user() const;
    QString password() const;
    bool forceGdx() const;
    void setLastPassword(QString lastPassword);
    void focusEmptyField();
    void setEngineVersion(QString version);
    bool eventFilter(QObject *watched, QEvent *event) override;

    QDialogButtonBox::StandardButton standardButton(QAbstractButton *button) const;

signals:
    void ready(bool start, bool always);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void buttonClicked(QAbstractButton *button);
    void getVersion();
    QString ensureApi(QString url);
    void setCanStart(bool valid);
    void setConnectionState(ServerConnectionState state);

private slots:
    void urlEdited(const QString &text);
    void textChanged(const QString &);
    void on_bAlways_clicked();
    void reVersion(const QString &engineVersion, const QString &gamsVersion);
    void reVersionError(const QString &errorText);
    void on_cbForceGdx_stateChanged(int state);
    void updateConnectStateAppearance();

private:
    Ui::EngineStartDialog *ui;
    EngineProcess *mProc;
    QStringList mLocalGamsVersion;
    ServerConnectionState mConnectState = scsNone;
    QString mUrl;
    QString mValidUrl;
//    bool mPendingRequest = false;
    bool mUrlChanged = false;
    bool mForcePreviousWork = true;
    bool mHiddenCheck = false;
    QTimer mConnectStateUpdater;
    QString mEngineVersion;
    QString mGamsVersion;

};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // ENGINESTARTDIALOG_H
