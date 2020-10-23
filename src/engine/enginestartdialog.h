#ifndef ENGINESTARTDIALOG_H
#define ENGINESTARTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractButton>

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

    void setProcess(EngineProcess *process);
    EngineProcess *process() const;
    QString url() const;
    QString nSpace() const;
    QString user() const;
    QString password() const;
    QString gamsVersion() const { return mGamsVersion; }
    void setLastPassword(QString lastPassword);
    void focusEmptyField();
    void setEngineVersion(QString version);

    QDialogButtonBox::StandardButton standardButton(QAbstractButton *button) const;

signals:
    void ready(bool start, bool always);

protected:
    void showEvent(QShowEvent *event);
    void buttonClicked(QAbstractButton *button);
    void getVersion();

private slots:
    void urlEdited(const QString &text);
    void textChanged(const QString &);
    void on_bAlways_clicked();
    void reVersion(const QString &engineVersion, const QString &gamsVersion);
    void reVersionError(const QString &errorText);

private:
    Ui::EngineStartDialog *ui;
    EngineProcess *mProc;
    QString mGamsVersion;
    QString mUrl;
    QString mOldUrl;
    bool mUrlChanged = false;
    bool mPendingRequest = false;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // ENGINESTARTDIALOG_H
