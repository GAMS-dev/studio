/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class QAbstractButton;

namespace gams {
namespace studio {

class MainWindow;
class StudioSettings;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(StudioSettings* settings, MainWindow *parent = nullptr);
    ~SettingsDialog();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void editorFontChanged(const QString &fontFamily, int fontSize);
    void editorLineWrappingChanged();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_fontComboBox_currentIndexChanged(const QString &arg1);
    void on_sb_fontsize_valueChanged(int arg1);
    void setModified();

    void on_btn_openUserLibLocation_clicked();
    void on_btn_browse_clicked();
    void on_btn_export_clicked();
    void on_btn_import_clicked();
    void on_btn_resetView_clicked();

private:
    Ui::SettingsDialog *ui;
    StudioSettings *mSettings;
    MainWindow *mMain;

    void saveSettings();
    void loadSettings();
    void setModifiedStatus(bool status);
    bool isModified = false;
};

}
}

#endif // SETTINGSDIALOG_H
