#ifndef ENGINESTARTDIALOG_H
#define ENGINESTARTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractButton>

class QCompleter;
class QStringListModel;

namespace gams {
namespace studio {
namespace engine {

class EngineProcess;

namespace Ui {
class EngineStartDialog;
}

class EngineStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineStartDialog(QWidget *parent = nullptr);
    ~EngineStartDialog();
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


private slots:
    void urlEdited(const QString &text);
    void textChanged(const QString &);
    void on_bAlways_clicked();
    void reVersion(const QString &engineVersion, const QString &gamsVersion);
    void reVersionError(const QString &errorText);

    void on_cbForceGdx_stateChanged(int state);

private:
    Ui::EngineStartDialog *ui;
    EngineProcess *mProc;
    QCompleter *mCompleter;
    QStringListModel *mCompleteModel;
    QStringList mLocalGamsVersion;
    QString mUrl;
    bool mDelete = false;
    bool mUrlChanged = false;
    bool mPendingRequest = false;
    bool mForcePreviousWork = true;
    bool mHiddenCheck = false;

};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // ENGINESTARTDIALOG_H
