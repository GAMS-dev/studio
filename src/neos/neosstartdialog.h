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
    explicit NeosStartDialog(QWidget *parent = nullptr);
    ~NeosStartDialog();
    void setProcess(neos::NeosProcess *proc);

private slots:
    void buttonClicked(QAbstractButton *button);
    void updateCanStart();
    void updateValues();

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::NeosStartDialog *ui;
    NeosProcess *mProc = nullptr;

};

} // namespace neos
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_NEOSSTARTDIALOG_H
