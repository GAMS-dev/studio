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
    explicit ConfirmDialog(QWidget *parent = nullptr);
    ~ConfirmDialog();
    bool confirm() const { return mConfirm; }

private slots:
    void on_checkBox_stateChanged(int state);
    void on_buttonAlwaysOk_clicked();

private:
    Ui::ConfirmDialog *ui;
    bool mConfirm = true;
};

}
}

#endif // CONFIRMDIALOG_H
