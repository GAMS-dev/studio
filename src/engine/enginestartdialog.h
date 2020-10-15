#ifndef ENGINESTARTDIALOG_H
#define ENGINESTARTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractButton>

namespace gams {
namespace studio {
namespace engine {

namespace Ui {
class EngineStartDialog;
}

class EngineStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineStartDialog(QWidget *parent = nullptr);
    ~EngineStartDialog();

    QString url() const;
    QString nSpace() const;
    QString user() const;
    QString password() const;
    void setLastPassword(QString lastPassword);
    void focusEmptyField();

    QDialogButtonBox::StandardButton standardButton(QAbstractButton *button) const;

signals:
    void buttonClicked(QAbstractButton *button);

protected:
    void showEvent(QShowEvent *event);

private slots:
    void textChanged(const QString &text);

private:
    Ui::EngineStartDialog *ui;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // ENGINESTARTDIALOG_H
