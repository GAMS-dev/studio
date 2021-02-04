/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
class Settings;
class ThemeWidget;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(MainWindow *parent = nullptr);
    ~SettingsDialog() override;

    bool miroSettingsEnabled() const;
    void setMiroSettingsEnabled(bool enabled);

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void editorFontChanged(const QString &fontFamily, int fontSize);
    void editorLineWrappingChanged();
    void themeChanged();
    void userGamsTypeChanged();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_tabWidget_currentChanged(int index);
    void on_fontComboBox_currentIndexChanged(const QString &value);
    void on_sb_fontsize_valueChanged(int size);
    void themeModified();
    void updateUserTypeToolTip(bool direct);
    void setModified();
    void appearanceIndexChanged(int index);
    void editorBaseColorChanged();

    void on_btn_openUserLibLocation_clicked();
    void on_btn_browse_clicked();
    void on_btn_export_clicked();
    void on_btn_import_clicked();
    void on_btn_resetView_clicked();
    void on_cb_writeLog_toggled(bool checked);
    void on_sb_nrLogBackups_valueChanged(int value);
    void on_miroBrowseButton_clicked();
    void on_btn_resetHistory_clicked();
    void on_btRenameTheme_clicked();
    void on_btCopyTheme_clicked();
    void on_btRemoveTheme_clicked();
    void on_btImportTheme_clicked();
    void on_btExportTheme_clicked();

private:
    Ui::SettingsDialog *ui;
    Settings *mSettings;
    MainWindow *mMain;
    bool isModified = false;
    bool mInitializing = true;
    QList<ThemeWidget*> mColorWidgets;
    int mFixedThemeCount = 0;
    bool mMiroSettingsEnabled = true;

    void saveSettings();
    void loadSettings();
    void setModifiedStatus(bool status);
    void initColorPage();
    void setThemeEditable(bool editable);

};

}
}

#endif // SETTINGSDIALOG_H
