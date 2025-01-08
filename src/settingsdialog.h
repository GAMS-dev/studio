/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include <QDate>

namespace Ui {
class SettingsDialog;
}

class QAbstractButton;

namespace gams {
namespace studio {

namespace support {
class CheckForUpdate;
}

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
    bool preventThemeChanging();
    bool hasDelayedBaseThemeChange();
    int engineInitialExpire() const;

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void editorFontChanged(int fontSize, const QString &fontFamily);
    void editorLineWrappingChanged();
    void themeChanged(bool refreshSyntax);
    void userGamsTypeChanged();
    void editorTabSizeChanged(int size);
    void reactivateEngineDialog();
    void persistToken();
    void rehighlight();
    void evalGspFileCount();
    void updateExtraSelections();
    void cleanupWorkspace(const QStringList&);

public slots:
    void delayBaseThemeChange(bool valid);
    void focusUpdateTab();
    void open() override;

private slots:
    void setModified();
    void prepareModifyTheme();
    void themeModified();
    bool setAndCheckUserLib(const QString &path);
    void appearanceIndexChanged(int index);
    void editorBaseColorChanged();
    void afterLoad();

    void on_buttonBox_clicked(QAbstractButton *button);
    void on_tabWidget_currentChanged(int index);
    void on_fontComboBox_currentIndexChanged(int index);
    void on_sb_fontsize_valueChanged(int size);
    void on_sb_tabsize_valueChanged(int size);

    void on_btn_browse_clicked();
    void on_btn_export_clicked();
    void on_btn_import_clicked();
    void on_btn_resetView_clicked();
    void on_cb_writeLog_toggled(bool checked);
    void on_sb_nrLogBackups_valueChanged(int value);
    void on_miroBrowseButton_clicked();
    void miroPathTextChanged(const QString &text);
    void on_btn_resetHistory_clicked();
    void on_btRenameTheme_clicked();
    void on_btCopyTheme_clicked();
    void on_btRemoveTheme_clicked();
    void on_btImportTheme_clicked();
    void on_btExportTheme_clicked();
    void on_btEngineDialog_clicked();

    void on_tb_userLibSelect_clicked();
    void on_tb_userLibRemove_clicked();
    void on_tb_userLibOpen_clicked();

    void checkGamsVersion(const QString &text);
    void anchorClicked(const QUrl &link);

    void on_rb_decSepCustom_toggled(bool checked);
    void on_rb_decSepLocale_toggled(bool checked);
    void on_rb_decSepStudio_toggled(bool checked);
    void updateNumericalPrecision();

    void checkForUpdates();

private:
    void saveSettings();
    void loadSettings();
    void setModifiedStatus(bool status);
    void initColorPage();
    void setThemeEditable(bool editable);
    void prependUserLib();

    QString changeSeparators(const QString &commaSeparatedList, const QString &newSeparator);
    QDate nextCheckDate() const;

    void setCheckForUpdateState();
    void setCheckForUpdateSettingsState();

private:
    Ui::SettingsDialog *ui;
    Settings *mSettings;
    MainWindow *mMain;
    bool isModified = false;
    bool mNeedRehighlight = false;
    bool mEvalGspFileCount = false;
    bool mInitializing = true;
    QList<ThemeWidget*> mColorWidgets;
    int mFixedThemeCount = 0;
    bool mDelayedBaseThemeChange = false;
    bool mMiroSettingsEnabled = true;
    int mEngineInitialExpire = 0;
    QDate mLastCheckDate;

    bool mRestoreSqZeroes = false;

    QScopedPointer<support::CheckForUpdate> mC4U;
};

}
}

#endif // SETTINGSDIALOG_H
