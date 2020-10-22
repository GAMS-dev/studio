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
    void setLastPassword(QString lastPassword);
    void focusEmptyField();
    void setEngineVersion(QString version);

    QDialogButtonBox::StandardButton standardButton(QAbstractButton *button) const;

signals:
    void ready(bool start, bool always);

protected:
    void showEvent(QShowEvent *event);
    void buttonClicked(QAbstractButton *button);

private slots:
    void textEdited(const QString &);
    void textChanged(const QString &);
    void on_bAlways_clicked();
    void reVersion(const QString &engineVersion, const QString &gamsVersion);
    void reVersionError(const QString &errorText);

private:
    Ui::EngineStartDialog *ui;
    EngineProcess *mProc;
    QString mGamsVersion;
    QString mUrl;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // ENGINESTARTDIALOG_H
