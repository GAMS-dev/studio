/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "scheme.h"
#include "colors/palettemanager.h"
#include "settingsdialog.h"
#include "settings.h"
#include "ui_settingsdialog.h"
#include "miro/mirocommon.h"
#include "schemewidget.h"
#include "viewhelper.h"

#ifdef __APPLE__
#include "../platform/macos/macoscocoabridge.h"
#endif

namespace gams {
namespace studio {


SettingsDialog::SettingsDialog(MainWindow *parent) :
    QDialog(parent), ui(new Ui::SettingsDialog), mMain(parent)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Themes
#ifdef _WIN32
    ui->combo_appearance->blockSignals(true);
    ui->combo_appearance->insertItem(0, "Follow Operating System");
    ui->combo_appearance->blockSignals(false);
#elif __APPLE__
    ui->label_4->setVisible(false); // theme chooser is deactived on macos, as the way of setting the light palette doesnt work there
    ui->combo_appearance->setVisible(false);
#endif

    mSettings = Settings::settings();
    mSettings->block(); // prevent changes from outside this dialog
    loadSettings();

    // TODO(JM) temporarily removed
//    initColorPage();
    ui->tabWidget->removeTab(3);

    // TODO(JM) Disabled until feature #1145 is implemented
    ui->cb_linewrap_process->setVisible(false);

    // connections to track modified status
    connect(ui->txt_workspace, &QLineEdit::textChanged, this, &SettingsDialog::setModified);
    connect(ui->cb_skipwelcome, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_restoretabs, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_autosave, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_openlst, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_jumptoerror, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_foregroundOnDemand, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->combo_appearance, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::setModified);
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
    connect(ui->cb_writeLog, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->sb_nrLogBackups, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::setModified);
    connect(ui->sb_historySize, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::setModified);
    connect(ui->cb_autoclose, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->overrideExistingOptionCheckBox, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->addCommentAboveCheckBox, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->addEOLCommentCheckBox, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->deleteCommentAboveCheckbox, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->miroEdit, &QLineEdit::textChanged, this, &SettingsDialog::setModified);
    adjustSize();

    setModifiedStatus(false);
}

void SettingsDialog::loadSettings()
{
    mSettings->load(Settings::scUser);

    // general tab page
    ui->txt_workspace->setText(mSettings->toString(skDefaultWorkspace));
    ui->cb_skipwelcome->setChecked(mSettings->toBool(skSkipWelcomePage));
    ui->cb_restoretabs->setChecked(mSettings->toBool(skRestoreTabs));
    ui->cb_autosave->setChecked(mSettings->toBool(skAutosaveOnRun));
    ui->cb_openlst->setChecked(mSettings->toBool(skOpenLst));
    ui->cb_jumptoerror->setChecked(mSettings->toBool(skJumpToError));
    ui->cb_foregroundOnDemand->setChecked(mSettings->toBool(skForegroundOnDemand));

    // editor tab page
    ui->combo_appearance->setCurrentIndex(mSettings->toInt(skEdAppearance));
    ui->fontComboBox->setCurrentFont(QFont(mSettings->toString(skEdFontFamily)));
    ui->sb_fontsize->setValue(mSettings->toInt(skEdFontSize));
    ui->cb_showlinenr->setChecked(mSettings->toBool(skEdShowLineNr));
    ui->sb_tabsize->setValue(mSettings->toInt(skEdTabSize));
    ui->cb_linewrap_editor->setChecked(mSettings->toBool(skEdLineWrapEditor));
    ui->cb_linewrap_process->setChecked(mSettings->toBool(skEdLineWrapProcess));
    ui->cb_clearlog->setChecked(mSettings->toBool(skEdClearLog));
    ui->cb_highlightUnderCursor->setChecked(mSettings->toBool(skEdWordUnderCursor));
    ui->cb_highlightcurrent->setChecked(mSettings->toBool(skEdHighlightCurrentLine));
    ui->cb_autoindent->setChecked(mSettings->toBool(skEdAutoIndent));
    ui->cb_writeLog->setChecked(mSettings->toBool(skEdWriteLog));
    ui->sb_nrLogBackups->setValue(mSettings->toInt(skEdLogBackupCount));
    ui->cb_autoclose->setChecked(mSettings->toBool(skEdAutoCloseBraces));

    // MIRO page
    ui->miroEdit->setText(QDir::toNativeSeparators(mSettings->toString(skMiroInstallPath)));
    if (ui->miroEdit->text().isEmpty()) {
        auto path = QDir::toNativeSeparators(miro::MiroCommon::path(""));
        ui->miroEdit->setText(path);
        mSettings->setString(skMiroInstallPath, path);
    }

    // misc tab page
    ui->sb_historySize->setValue(mSettings->toInt(skHistorySize));
    // solver option editor
    ui->overrideExistingOptionCheckBox->setChecked(mSettings->toBool(skSoOverrideExisting));
    ui->addCommentAboveCheckBox->setChecked(mSettings->toBool(skSoAddCommentAbove));
    ui->addEOLCommentCheckBox->setChecked(mSettings->toBool(skSoAddEOLComment));
    ui->deleteCommentAboveCheckbox->setChecked(mSettings->toBool(skSoDeleteCommentsAbove));
}

void SettingsDialog::on_tabWidget_currentChanged(int index)
{
    if (mInitializing && ui->tabWidget->widget(index) == ui->tabColors) {
        mInitializing = false;
        for (SchemeWidget *wid : mColorWidgets) {
            wid->refresh();
        }
    }
}

void SettingsDialog::setModified()
{
    setModifiedStatus(true);
}

void SettingsDialog::setModifiedStatus(bool status)
{
    isModified = status;
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(status);
}

bool SettingsDialog::miroSettingsEnabled() const
{
    return mMiroSettingsEnabled;
}

void SettingsDialog::setMiroSettingsEnabled(bool enabled)
{
    mMiroSettingsEnabled = enabled;
    ui->miroEdit->setEnabled(enabled);
    ui->miroBrowseButton->setEnabled(enabled);
}

    // save settings from ui to qsettings
void SettingsDialog::saveSettings()
{
    // general page
    mSettings->setString(skDefaultWorkspace, ui->txt_workspace->text());
    QDir workspace(ui->txt_workspace->text());
    if (!workspace.exists())
        workspace.mkpath(".");

    mSettings->setBool(skSkipWelcomePage, ui->cb_skipwelcome->isChecked());
    mSettings->setBool(skRestoreTabs, ui->cb_restoretabs->isChecked());
    mSettings->setBool(skAutosaveOnRun, ui->cb_autosave->isChecked());
    mSettings->setBool(skOpenLst, ui->cb_openlst->isChecked());
    mSettings->setBool(skJumpToError, ui->cb_jumptoerror->isChecked());
    mSettings->setBool(skForegroundOnDemand, ui->cb_foregroundOnDemand->isChecked());

    // editor page
    mSettings->setInt(skEdAppearance, ui->combo_appearance->currentIndex());
    mSettings->setString(skEdFontFamily, ui->fontComboBox->currentFont().family());
    mSettings->setInt(skEdFontSize, ui->sb_fontsize->value());
    mSettings->setBool(skEdShowLineNr, ui->cb_showlinenr->isChecked());
    mSettings->setInt(skEdTabSize, ui->sb_tabsize->value());
    mSettings->setBool(skEdLineWrapEditor, ui->cb_linewrap_editor->isChecked());
    mSettings->setBool(skEdLineWrapProcess, ui->cb_linewrap_process->isChecked());
    mSettings->setBool(skEdClearLog, ui->cb_clearlog->isChecked());
    mSettings->setBool(skEdWordUnderCursor, ui->cb_highlightUnderCursor->isChecked());
    mSettings->setBool(skEdHighlightCurrentLine, ui->cb_highlightcurrent->isChecked());
    mSettings->setBool(skEdAutoIndent, ui->cb_autoindent->isChecked());
    mSettings->setBool(skEdWriteLog, ui->cb_writeLog->isChecked());
    mSettings->setInt(skEdLogBackupCount, ui->sb_nrLogBackups->value());
    mSettings->setBool(skEdAutoCloseBraces, ui->cb_autoclose->isChecked());

    // MIRO page
    mSettings->setString(skMiroInstallPath, ui->miroEdit->text());

    // colors page
//    mSettings->saveScheme();

    // misc page
    mSettings->setInt(skHistorySize, ui->sb_historySize->value());

    // solver option editor
    mSettings->setBool(skSoOverrideExisting, ui->overrideExistingOptionCheckBox->isChecked());
    mSettings->setBool(skSoAddCommentAbove, ui->addCommentAboveCheckBox->isChecked());
    mSettings->setBool(skSoAddEOLComment, ui->addEOLCommentCheckBox->isChecked());
    mSettings->setBool(skSoDeleteCommentsAbove, ui->deleteCommentAboveCheckbox->isChecked());

    // done
    mSettings->unblock();
    mSettings->save();
    mSettings->block();
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

void SettingsDialog::on_fontComboBox_currentIndexChanged(const QString &value)
{
    emit editorFontChanged(value, ui->sb_fontsize->value());
}

void SettingsDialog::on_sb_fontsize_valueChanged(int size)
{
    emit editorFontChanged(ui->fontComboBox->currentFont().family(), size);
}

void SettingsDialog::on_combo_appearance_currentIndexChanged(int index)
{
#ifndef __APPLE__
    ViewHelper::changeAppearance(index);
#else
    Q_UNUSED(index)
#endif
}

void SettingsDialog::schemeModified()
{
    emit setModified();
    Scheme::instance()->invalidate();
    for (QWidget *wid: mColorWidgets) {
        static_cast<SchemeWidget*>(wid)->refresh();
    }
}

void SettingsDialog::on_btn_openUserLibLocation_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(mSettings->toString(skUserModelLibraryDir)));
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
    mSettings->unblock();
    delete ui;
}

void SettingsDialog::on_btn_export_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export Settings",
                                                    mSettings->toString(skDefaultWorkspace)
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
                                                    mSettings->toString(skDefaultWorkspace),
                                                    tr("GAMS user settings (*.gus);;"
                                                       "All files (*.*)"));
    if (filePath == "") return;

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("You are about to overwrite your local settings. Are you sure?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();

    if (answer == QMessageBox::Ok) {
        mSettings->importSettings(filePath);
    }

    emit editorLineWrappingChanged();
    emit editorFontChanged(mSettings->toString(skEdFontFamily),
                           mSettings->toInt(skEdFontSize));
    close();
}

void SettingsDialog::on_btn_resetView_clicked()
{
    mMain->resetViews();
}

void SettingsDialog::on_cb_writeLog_toggled(bool checked)
{
    ui->lbl_nrBackups->setEnabled(checked);
    ui->sb_nrLogBackups->setEnabled(checked);
    mSettings->setBool(skEdWriteLog, checked);
}

void SettingsDialog::on_sb_nrLogBackups_valueChanged(int value)
{
    mSettings->setInt(skEdLogBackupCount, value);
}

void SettingsDialog::on_miroBrowseButton_clicked()
{
    QString dir;
    if (mSettings->toString(skMiroInstallPath).isEmpty())
        dir = mSettings->toString(skDefaultWorkspace);
    else
        dir = mSettings->toString(skMiroInstallPath);
    auto miro = QFileDialog::getOpenFileName(this, tr("MIRO location"), dir);

    if (miro.isEmpty()) return;

    ui->miroEdit->setText(QDir::toNativeSeparators(miro));
}

void SettingsDialog::initColorPage()
{
    if (!mColorWidgets.isEmpty()) return;

    QWidget *box = nullptr;
    QGridLayout *grid = nullptr;
    QVector<QVector<Scheme::ColorSlot>> slot2;
    QStringList names;

    // EDIT first colors
    box = ui->editP1;
    grid = qobject_cast<QGridLayout*>(box->layout());
    SchemeWidget *wid = nullptr;

    slot2 = {
        {Scheme::Edit_linenrAreaFg, Scheme::Edit_linenrAreaBg},
        {Scheme::Edit_linenrAreaMarkFg, Scheme::Edit_linenrAreaMarkBg},
        {},

        {Scheme::invalid, Scheme::Edit_currentLineBg},
        {},

        {Scheme::invalid, Scheme::Edit_currentWordBg},
        {Scheme::invalid, Scheme::Edit_matchesBg},
        {Scheme::invalid, Scheme::Edit_errorBg},
    };
    int cols = 3;
    int rows = ((slot2.count()-1) / cols) + 1;
    for (int i = 0; i < slot2.size(); ++i) {
        if (slot2.at(i).isEmpty()) continue;
        int row = i % rows;
        int col = i / rows;
        wid = (slot2.at(i).size() == 1) ? new SchemeWidget(slot2.at(i).at(0), box)
                                        : new SchemeWidget(slot2.at(i).at(0), slot2.at(i).at(1), box);
        wid->setAlignment(Qt::AlignRight);
        grid->addWidget(wid, row+1, col, Qt::AlignRight);
        connect(wid, &SchemeWidget::changed, this, &SettingsDialog::schemeModified);
        mColorWidgets << wid;
    }
    for (int col = 0; col < 3; ++col)
        grid->setColumnStretch(col, 1);

    // EDIT second colors
    box = ui->editP2;
    grid = qobject_cast<QGridLayout*>(box->layout());
    slot2 = {
        {Scheme::Edit_parenthesesValidFg, Scheme::Edit_parenthesesValidBg, Scheme::Edit_parenthesesValidBgBlink},
        {Scheme::Edit_parenthesesInvalidFg, Scheme::Edit_parenthesesInvalidBg, Scheme::Edit_parenthesesInvalidBgBlink},
        {},
        {},
    };
    rows = ((slot2.count()-1) / cols) + 1;
    for (int i = 0; i < slot2.size(); ++i) {
        if (slot2.at(i).isEmpty()) continue;
        int row = i % rows;
        int col = i / rows;
        if (slot2.at(i).size()) {
            wid = new SchemeWidget(slot2.at(i).at(0), slot2.at(i).at(1), slot2.at(i).at(2), box);
            wid->setAlignment(Qt::AlignRight);
            grid->addWidget(wid, row+1, col, Qt::AlignRight);
            connect(wid, &SchemeWidget::changed, this, &SettingsDialog::schemeModified);
            mColorWidgets << wid;
        } else {
            grid->addWidget(new QWidget(box), row+1, col);
        }
    }

    // SYNTAX colors
    box = ui->syntax;
    grid = qobject_cast<QGridLayout*>(box->layout());
    slot2 = {
        {Scheme::Syntax_directive},
        {Scheme::Syntax_directiveBody},
        {Scheme::Syntax_assign},
        {Scheme::Syntax_declaration},
        {Scheme::Syntax_keyword},

        {Scheme::Syntax_identifier},
        {Scheme::Syntax_identifierAssign},
        {Scheme::Syntax_assignLabel},
        {Scheme::Syntax_assignValue},
        {Scheme::Syntax_tableHeader},

        {Scheme::Syntax_comment},
        {Scheme::Syntax_title},
        {Scheme::Syntax_description},
        {Scheme::Syntax_embedded},
    };
    cols = 3;
    rows = ((slot2.count()-1) / cols) + 1;
    for (int i = 0; i < slot2.size(); ++i) {
        if (slot2.at(i).isEmpty()) continue;
        int row = i % rows;
        int col = i / rows;
        wid = new SchemeWidget(slot2.at(i).at(0), box);
        wid->setAlignment(Qt::AlignRight);
        grid->addWidget(wid, row+1, col, Qt::AlignRight);
        connect(wid, &SchemeWidget::changed, this, &SettingsDialog::schemeModified);
        mColorWidgets << wid;
    }
    for (int col = 0; col < 3; ++col)
        grid->setColumnStretch(col, 1);

    // ICON colors
    box = ui->groupIconColors;
    grid = qobject_cast<QGridLayout*>(box->layout());
    QVector<Scheme::ColorSlot> slot1;
    slot1 = {Scheme::Icon_Gray, Scheme::Disable_Gray, Scheme::Active_Gray, Scheme::Select_Gray,
            Scheme::Icon_Back, Scheme::Disable_Back, Scheme::Active_Back, Scheme::Select_Back};
    for (int i = 0; i < slot1.size(); ++i) {
        SchemeWidget *wid = new SchemeWidget(slot1.at(i), box, true);
        wid->setTextVisible(false);
        grid->addWidget(wid, (i/4)+1, (i%4)+1);
        connect(wid, &SchemeWidget::changed, this, &SettingsDialog::schemeModified);
        mColorWidgets.insert(slot1.at(i), wid);
    }
}


}
}
