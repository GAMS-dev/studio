#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QDialog>

namespace Ui {
class ConfirmDialog;
}

namespace gams {
namespace studio {

class ConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfirmDialog(QString title, QString text, QString checkText, QWidget *parent = nullptr);
    ~ConfirmDialog();
    void setBoxAccepted(bool accept);

signals:
    void autoConfirm();
    void setAcceptBox(bool accept);

private slots:
    void on_checkBox_stateChanged(int state);
    void on_buttonAlwaysOk_clicked();

private:
    Ui::ConfirmDialog *ui;
};

}
}

#endif // CONFIRMDIALOG_H
