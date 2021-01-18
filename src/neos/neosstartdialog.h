#ifndef GAMS_STUDIO_NEOSSTARTDIALOG_H
#define GAMS_STUDIO_NEOSSTARTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

class QLabel;
class QCheckBox;

namespace gams {
namespace studio {
namespace neos {

class NeosProcess;

namespace Ui {
class NeosStartDialog;
}

class NeosStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NeosStartDialog(QString eMail, QWidget *parent = nullptr);
    ~NeosStartDialog() override;
    void setProcess(neos::NeosProcess *proc);

signals:
    void noDialogFlagChanged(bool noDialog);

private slots:
    void buttonClicked(QAbstractButton *button);
    void updateCanStart();
    void updateValues();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::NeosStartDialog *ui;
    NeosProcess *mProc = nullptr;
    bool mFirstShow = true;

};

} // namespace neos
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_NEOSSTARTDIALOG_H
