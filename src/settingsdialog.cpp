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
#include "theme.h"
#include "colors/palettemanager.h"
#include "settingsdialog.h"
#include "settings.h"
#include "ui_settingsdialog.h"
#include "miro/mirocommon.h"
#include "themewidget.h"
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
    ui->tabWidget->setCurrentIndex(0);

    // Themes
    ui->cbThemes->clear();
    for (int i = 0; i < Theme::instance()->themeCount(); ++i) {
        ui->cbThemes->insertItem(i, Theme::instance()->themes().at(i));
    }
    mFixedThemeCount = 2;
#ifdef _WIN32
    ui->cbThemes->insertItem(0, "Follow Operating System");
    ++mFixedThemeCount;
#elif __APPLE__
//    ui->cbThemes->setVisible(false);
#endif

    mSettings = Settings::settings();
    mSettings->block(); // prevent changes from outside this dialog
    initColorPage();
    loadSettings();

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
    connect(ui->cbThemes, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::setModified);
    connect(ui->cbThemes, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::appearanceIndexChanged);
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
    connect(ui->cb_autoclose, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->confirmNeosCheckBox, &QCheckBox::clicked, this, &SettingsDialog::setModified);
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
    mSettings->loadFile(Settings::scUser);
    mSettings->loadFile(Settings::scTheme);

    // general tab page
    ui->txt_workspace->setText(mSettings->toString(skDefaultWorkspace));
    ui->cb_skipwelcome->setChecked(mSettings->toBool(skSkipWelcomePage));
    ui->cb_restoretabs->setChecked(mSettings->toBool(skRestoreTabs));
    ui->cb_autosave->setChecked(mSettings->toBool(skAutosaveOnRun));
    ui->cb_openlst->setChecked(mSettings->toBool(skOpenLst));
    ui->cb_jumptoerror->setChecked(mSettings->toBool(skJumpToError));
    ui->cb_foregroundOnDemand->setChecked(mSettings->toBool(skForegroundOnDemand));

    // editor tab page
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

    // color page
    Theme::instance()->readUserThemes(mSettings->toList(SettingsKey::skUserThemes));
    ui->cbThemes->setCurrentIndex(mSettings->toInt(skEdAppearance));
    setThemeEditable(mSettings->toInt(skEdAppearance) >= mFixedThemeCount);

    // misc page
    ui->confirmNeosCheckBox->setChecked(mSettings->toBool(skNeosAutoConfirm));

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
        for (ThemeWidget *wid : mColorWidgets) {
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
    mSettings->setInt(skEdAppearance, ui->cbThemes->currentIndex());
    mSettings->setList(SettingsKey::skUserThemes, Theme::instance()->writeUserThemes());

    // misc page
    mSettings->setBool(skNeosAutoConfirm, ui->confirmNeosCheckBox->isChecked());

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
    filedialog.setOption(QFileDialog::ShowDirsOnly, true);
    filedialog.setFileMode(QFileDialog::Directory);

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
        emit themeModified();
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

void SettingsDialog::appearanceIndexChanged(int index)
{
    ViewHelper::changeAppearance(index);
//#ifndef __APPLE__
//#else
//    Q_UNUSED(index)
//#endif
    setThemeEditable(index >= mFixedThemeCount);
}

void SettingsDialog::editorBaseColorChanged()
{
    QColor color = Theme::color(Theme::Edit_text);
    Theme::setColor(Theme::Syntax_formula, color);
    Theme::setColor(Theme::Syntax_neutral, color);
}

void SettingsDialog::themeModified()
{
    emit setModified();
    emit themeChanged();
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

bool SettingsDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->cbThemes && ui->cbThemes->isEditable()) {
        bool rename = false;
        if (event->type() == QEvent::KeyPress) {
            auto ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Escape) {
                ui->cbThemes->setEditable(false);
                ui->cbThemes->removeEventFilter(this);
                return true;
            }
            if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                rename = true;
            }
        }
        if (event->type() == QEvent::FocusOut) {
            rename = true;
        }
        if (rename) {
            QString name = ui->cbThemes->currentText();
            name = Theme::instance()->renameActiveTheme(name);
            ui->cbThemes->setEditable(false);
            ui->cbThemes->removeEventFilter(this);
            int i = ui->cbThemes->currentIndex();
            ui->cbThemes->removeItem(i);
            i = Theme::instance()->themes().indexOf(name);
            int shift = mFixedThemeCount-2;
            ui->cbThemes->insertItem(i+shift, name);
            ui->cbThemes->setCurrentIndex(i+shift);
            return true;
        }
    }

    return false;
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
#ifndef __APPLE__
    ViewHelper::setAppearance(); // update ui
#endif
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
    QVector<QVector<Theme::ColorSlot>> slot2;
    QStringList names;
    ThemeWidget *wid = nullptr;
    int cols;
    int rows;

    // SYNTAX colors
    box = ui->syntaxColors;
    grid = qobject_cast<QGridLayout*>(box->layout());
    slot2 = {
        {Theme::Syntax_declaration},
        {Theme::Syntax_assignLabel},
        {Theme::Syntax_assignValue},
        {Theme::Syntax_directive},
        {Theme::Syntax_embedded},
        {Theme::Syntax_keyword},

        {Theme::Syntax_identifier},
        {Theme::Syntax_description},
        {},
        {Theme::Syntax_directiveBody},
        {Theme::Syntax_comment},
    };
    cols = 2;
    rows = ((slot2.count()-1) / cols) + 1;
    int sep = 3;
    for (int i = 0; i < slot2.size(); ++i) {
        if (slot2.at(i).isEmpty()) continue;
        int row = i % rows;
        int col = i / rows;
        wid = new ThemeWidget(slot2.at(i).at(0), box);
        wid->setCarrierDialog(this);
        wid->setAlignment(Qt::AlignRight);
        int effectiveRow = row + (row >= sep ? 2 : 1);

        grid->addWidget(wid, effectiveRow, col, Qt::AlignRight);
        connect(wid, &ThemeWidget::changed, this, &SettingsDialog::themeModified);
        mColorWidgets << wid;
    }
    grid->addWidget(ui->syntaxColorLine, sep+1, 0, 1, 2);

    for (int col = 0; col < cols; ++col)
        grid->setColumnStretch(col, 1);
    grid->setColumnStretch(cols, 0);

    // EDITOR colors
    box = ui->editorColors;
    grid = qobject_cast<QGridLayout*>(box->layout());
    slot2 = {
        {Theme::Edit_text,                  Theme::Edit_background,             Theme::invalid},
        {Theme::invalid,                    Theme::Edit_currentLineBg,          Theme::invalid},
        {Theme::invalid,                    Theme::Edit_errorBg,                Theme::invalid},
        {Theme::invalid,                    Theme::Edit_currentWordBg,          Theme::invalid},
        {Theme::invalid,                    Theme::Edit_matchesBg,              Theme::invalid},
        {Theme::Edit_parenthesesValidFg,    Theme::Edit_parenthesesValidBg,     Theme::Edit_parenthesesValidBgBlink},
        {Theme::Edit_parenthesesInvalidFg,  Theme::Edit_parenthesesInvalidBg,   Theme::Edit_parenthesesInvalidBgBlink},

        {Theme::Edit_foldLineFg,            Theme::Edit_foldLineBg,             Theme::invalid},
        {Theme::Edit_linenrAreaFg,          Theme::Edit_linenrAreaBg,           Theme::invalid},
        {Theme::Edit_linenrAreaMarkFg,      Theme::Edit_linenrAreaMarkBg,       Theme::invalid},
        {Theme::invalid,                    Theme::Edit_linenrAreaFoldBg,       Theme::invalid},
        {Theme::Mark_errorFg,               Theme::invalid,                     Theme::invalid},
        {Theme::Mark_listingFg,             Theme::invalid,                     Theme::invalid},
        {Theme::Mark_fileFg,                Theme::invalid,                     Theme::invalid},

    };
    cols = 2;
    rows = ((slot2.count()-1) / cols) + 1;
    for (int i = 0; i < slot2.size(); ++i) {
        if (slot2.at(i).isEmpty()) continue;
        Theme::ColorSlot fg = slot2.at(i).at(0);
        Theme::ColorSlot bg1 = slot2.at(i).count() > 1 ? slot2.at(i).at(1) : Theme::invalid;
        Theme::ColorSlot bg2 = slot2.at(i).count() > 2 ? slot2.at(i).at(2) : Theme::invalid;
        int row = i % rows;
        int col = i / rows;
        wid = new ThemeWidget(fg, bg1, bg2, box);
        wid->setCarrierDialog(this);
        wid->setAlignment(Qt::AlignRight);
        grid->addWidget(wid, row+1, col, Qt::AlignRight);
        if (fg == Theme::Edit_text)
            connect(wid, &ThemeWidget::changed, this, &SettingsDialog::editorBaseColorChanged);
        connect(wid, &ThemeWidget::changed, this, &SettingsDialog::themeModified);
        mColorWidgets << wid;
    }
    for (int col = 0; col < cols; ++col)
        grid->setColumnStretch(col, 1);

    // ICON colors
//    box = ui->groupIconColors;
//    grid = qobject_cast<QGridLayout*>(box->layout());
//    QVector<Theme::ColorSlot> slot1;
//    slot1 = {Theme::Icon_Gray, Theme::Disable_Gray, Theme::Active_Gray, Theme::Select_Gray,
//            Theme::Icon_Back, Theme::Disable_Back, Theme::Active_Back, Theme::Select_Back};
//    for (int i = 0; i < slot1.size(); ++i) {
//        ThemeWidget *wid = new ThemeWidget(slot1.at(i), box, true);
//        wid->setTextVisible(false);
//        grid->addWidget(wid, (i/4)+1, (i%4)+1);
//        connect(wid, &ThemeWidget::changed, this, &SettingsDialog::themeModified);
//        mColorWidgets.insert(slot1.at(i), wid);
    //    }
}

void SettingsDialog::setThemeEditable(bool editable)
{
    for (ThemeWidget *wid : mColorWidgets) {
        wid->setReadonly(!editable);
    }
    ui->btRename->setEnabled(editable);
    ui->btRemove->setEnabled(editable);
}

void SettingsDialog::on_btn_resetHistory_clicked()
{
    mMain->resetHistory();
    mSettings->setList(skHistory, QVariantList());
}

void SettingsDialog::on_btRename_clicked()
{
    ui->cbThemes->setEditable(true);
    ui->cbThemes->setFocus();
    ui->cbThemes->installEventFilter(this);
}

void SettingsDialog::on_btCopy_clicked()
{
    Theme *theme = Theme::instance();
    int i = theme->copyTheme(theme->activeTheme(), theme->activeThemeName());
    int shift = mFixedThemeCount-2;
    ui->cbThemes->insertItem(i+shift, Theme::instance()->themes().at(i));
    ui->cbThemes->setCurrentIndex(i+shift);
    for (ThemeWidget *wid : mColorWidgets) {
        wid->refresh();
    }
}

void SettingsDialog::on_btRemove_clicked()
{
    int old = Theme::instance()->activeTheme();
    int i = Theme::instance()->removeTheme(old);
    int shift = mFixedThemeCount-2;
    ui->cbThemes->removeItem(old+shift);
    ui->cbThemes->setCurrentIndex(i+shift);
    for (ThemeWidget *wid : mColorWidgets) {
        wid->refresh();
    }
}

}
}
