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
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include "mainwindow.h"
#include "settingsdialog.h"
#include "studiosettings.h"
#include "ui_settingsdialog.h"

namespace gams {
namespace studio {


SettingsDialog::SettingsDialog(StudioSettings *settings, MainWindow *parent) :
    QDialog(parent), ui(new Ui::SettingsDialog), mSettings(settings), mMain(parent)
{
    ui->setupUi(this);

    // load from settings to UI
    loadSettings();
    setModifiedStatus(false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // connections to track modified status
    connect(ui->txt_workspace, &QLineEdit::textChanged, this, &SettingsDialog::setModified);
    connect(ui->cb_skipwelcome, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_restoretabs, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_autosave, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_openlst, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_jumptoerror, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->fontComboBox, &QFontComboBox::currentFontChanged, this, &SettingsDialog::setModified);
    connect(ui->sb_fontsize, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::setModified);
    connect(ui->sb_tabsize, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::setModified);
    connect(ui->cb_showlinenr, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_linewrap_editor, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_linewrap_process, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_clearlog, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_highlightUnderCursor, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_highlightcurrent, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_autoindent, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->sb_historySize, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::setModified);
    adjustSize();
}

void SettingsDialog::loadSettings()
{
    // general tab page
    ui->txt_workspace->setText(mSettings->defaultWorkspace());
    ui->cb_skipwelcome->setChecked(mSettings->skipWelcomePage());
    ui->cb_restoretabs->setChecked(mSettings->restoreTabs());
    ui->cb_autosave->setChecked(mSettings->autosaveOnRun());
    ui->cb_openlst->setChecked(mSettings->openLst());
    ui->cb_jumptoerror->setChecked(mSettings->jumpToError());

    // editor tab page
    ui->fontComboBox->setCurrentFont(QFont(mSettings->fontFamily()));
    ui->sb_fontsize->setValue(mSettings->fontSize());
    ui->cb_showlinenr->setChecked(mSettings->showLineNr());
    ui->sb_tabsize->setValue(mSettings->tabSize());
    ui->cb_linewrap_editor->setChecked(mSettings->lineWrapEditor());
    ui->cb_linewrap_process->setChecked(mSettings->lineWrapProcess());
    ui->cb_clearlog->setChecked(mSettings->clearLog());
    ui->cb_highlightUnderCursor->setChecked(mSettings->wordUnderCursor());
    ui->cb_highlightcurrent->setChecked(mSettings->highlightCurrentLine());
    ui->cb_autoindent->setChecked(mSettings->autoIndent());

    // misc tab page
    ui->sb_historySize->setValue(mSettings->historySize());
}

void SettingsDialog::setModified()
{
    isModified = true;
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::setModifiedStatus(bool status)
{
    isModified = status;
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(status);
}

    // save settings from ui to qsettings
void SettingsDialog::saveSettings()
{
    // general page
    mSettings->setDefaultWorkspace(ui->txt_workspace->text());
    mSettings->setSkipWelcomePage(ui->cb_skipwelcome->isChecked());
    mSettings->setRestoreTabs(ui->cb_restoretabs->isChecked());
    mSettings->setAutosaveOnRun(ui->cb_autosave->isChecked());
    mSettings->setOpenLst(ui->cb_openlst->isChecked());
    mSettings->setJumpToError(ui->cb_jumptoerror->isChecked());

    // editor page
    mSettings->setFontFamily(ui->fontComboBox->currentFont().family());
    mSettings->setFontSize(ui->sb_fontsize->value());
    mSettings->setShowLineNr(ui->cb_showlinenr->isChecked());
    mSettings->setTabSize(ui->sb_tabsize->value());
    mSettings->setLineWrapEditor(ui->cb_linewrap_editor->isChecked());
    mSettings->setLineWrapProcess(ui->cb_linewrap_process->isChecked());
    mSettings->setClearLog(ui->cb_clearlog->isChecked());
    mSettings->setWordUnderCursor(ui->cb_highlightUnderCursor->isChecked());
    mSettings->setHighlightCurrentLine(ui->cb_highlightcurrent->isChecked());
    mSettings->setAutoIndent(ui->cb_autoindent->isChecked());

    // misc page
    mSettings->setHistorySize(ui->sb_historySize->value());

    // done
    setModifiedStatus(false);
}

void SettingsDialog::on_btn_browse_clicked()
{
    QString workspace = ui->txt_workspace->text();
    QFileDialog filedialog(this, "Choose default working directory", workspace);
    filedialog.setFileMode(QFileDialog::DirectoryOnly);

    if (filedialog.exec())
        workspace = filedialog.selectedFiles().first();

    ui->txt_workspace->setText(workspace);
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button == ui->buttonBox->button(QDialogButtonBox::Apply)) {
        saveSettings();
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        saveSettings();
    } else { // reject
        loadSettings(); // reset changes (mostly font and -size)
    }
    emit editorLineWrappingChanged();
}

void SettingsDialog::on_fontComboBox_currentIndexChanged(const QString &arg1)
{
    emit editorFontChanged(arg1, ui->sb_fontsize->value());
}

void SettingsDialog::on_sb_fontsize_valueChanged(int arg1)
{
    emit editorFontChanged(ui->fontComboBox->currentFont().family(), arg1);
}

void SettingsDialog::on_btn_openUserLibLocation_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(mSettings->userModelLibraryDir()));
}

void SettingsDialog::closeEvent(QCloseEvent *event) {
    if (isModified) {
        QMessageBox msgBox;
        msgBox.setText("You have unsaved changes.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        int answer = msgBox.exec();

        if (answer == QMessageBox::Save) {
            saveSettings();
        } else if (answer == QMessageBox::Discard) {
            loadSettings();
        } else {
            event->setAccepted(false);
        }
    }
    emit editorLineWrappingChanged();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_btn_export_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export Settings",
                                                    mSettings->defaultWorkspace()
                                                    + "/studiosettings.gus",
                                                    tr("GAMS user settings (*.gus);;"
                                                       "All files (*.*)"));
    QFileInfo fi(filePath);
    if (fi.suffix().isEmpty()) filePath += ".gus";

    mSettings->exportSettings(filePath);
}

void SettingsDialog::on_btn_import_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Import Settings",
                                                    mSettings->defaultWorkspace(),
                                                    tr("GAMS user settings (*.gus);;"
                                                       "All files (*.*)"));
    if (filePath == "") return;
    mSettings->importSettings(filePath, mMain);
    emit editorLineWrappingChanged();
    emit editorFontChanged(mSettings->fontFamily(), mSettings->fontSize());
    close();
}

void SettingsDialog::on_btn_resetView_clicked()
{
    mMain->resetViews();
}

}
}

