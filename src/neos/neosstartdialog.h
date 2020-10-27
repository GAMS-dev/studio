#ifndef GAMS_STUDIO_NEOSSTARTDIALOG_H
#define GAMS_STUDIO_NEOSSTARTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

class QLabel;
class QCheckBox;

namespace gams {
namespace studio {
namespace neos {

namespace Ui {
class NeosStartDialog;
}

class NeosStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NeosStartDialog(QWidget *parent = nullptr);
    ~NeosStartDialog();
    void setConfirmText(QString text, QString checkboxText);

    QDialogButtonBox::StandardButton standardButton(QAbstractButton *button) const;

signals:
    void ready(bool start, bool always);

private slots:
    void buttonClicked(QAbstractButton *button);
    void updateCanStart();

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::NeosStartDialog *ui;
    QLabel *mLabelTerms = nullptr;
    QCheckBox *mConfirmTerms = nullptr;

};

} // namespace neos
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_NEOSSTARTDIALOG_H
