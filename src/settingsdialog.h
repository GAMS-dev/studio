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

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_fontComboBox_currentIndexChanged(const QString &arg1);
    void on_sb_fontsize_valueChanged(int arg1);
    void setModified();

private:
    StudioSettings *mSettings;
    Ui::SettingsDialog *ui;
    void saveSettings();
    void loadSettings();
    void setModifiedStatus(bool status);
    bool isModified = false;
};

}
}

#endif // SETTINGSDIALOG_H
