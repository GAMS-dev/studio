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
    void urlChanged(const QString &url);

protected:
    void showEvent(QShowEvent *event);
    void buttonClicked(QAbstractButton *button);

private slots:
    void textChanged(const QString &text);
    void on_bAlways_clicked();

private:
    Ui::EngineStartDialog *ui;
    EngineProcess *mProc;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // ENGINESTARTDIALOG_H
