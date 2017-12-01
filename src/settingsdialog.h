#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "studiosettings.h"

namespace Ui {
class SettingsDialog;
}

namespace gams {
namespace studio {


class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(StudioSettings* settings, QWidget *parent = 0);
    ~SettingsDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::SettingsDialog *ui;
    StudioSettings *mSettings;
    void saveSettings();
};

}
}

#endif // SETTINGSDIALOG_H
