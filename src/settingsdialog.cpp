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
#include "settingsdialog.h"
#include "studiosettings.h"
#include "settingslocator.h"
#include "ui_settingsdialog.h"
#include "miro/mirocommon.h"
#include "schemewidget.h"

namespace gams {
namespace studio {


SettingsDialog::SettingsDialog(MainWindow *parent) :
    QDialog(parent), ui(new Ui::SettingsDialog), mMain(parent)
{
    ui->setupUi(this);

    mSettings = SettingsLocator::settings();

    // load from settings to UI

    // TODO(JM) temporarily removed
//    initColorPage();
    ui->tabWidget->removeTab(3);

    loadSettings();

    setModifiedStatus(false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->combo_editorTheme->addItems(Scheme::instance()->schemes());
    ui->combo_editorTheme->setCurrentIndex(Scheme::instance()->activeScheme());

    // TODO(JM) Disabled until feature #1145 is implemented
    ui->cb_linewrap_process->setVisible(false);

    // connections to track modified status
    connect(ui->txt_workspace, &QLineEdit::textChanged, this, &SettingsDialog::setModified);
    connect(ui->cb_skipwelcome, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_restoretabs, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_autosave, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_openlst, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_jumptoerror, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_bringontop, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->combo_editorTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::setModified);
    connect(ui->combo_editorTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), Scheme::instance(), QOverload<int>::of(&Scheme::setActiveScheme));
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
    ui->cb_bringontop->setChecked(mSettings->foregroundOnDemand());

    // editor tab page
    ui->combo_editorTheme->setCurrentIndex(mSettings->colorSchemeIndex());
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
    ui->cb_writeLog->setChecked(mSettings->writeLog());
    ui->sb_nrLogBackups->setValue(mSettings->nrLogBackups());
    ui->cb_autoclose->setChecked(mSettings->autoCloseBraces());

    // MIRO page
    ui->miroEdit->setText(QDir::toNativeSeparators(mSettings->miroInstallationLocation()));
    if (ui->miroEdit->text().isEmpty()) {
        auto path = QDir::toNativeSeparators(miro::MiroCommon::path(""));
        ui->miroEdit->setText(path);
        mSettings->setMiroInstallationLocation(path);
    }

    // misc tab page
    ui->sb_historySize->setValue(mSettings->historySize());
    // solver option editor
    ui->overrideExistingOptionCheckBox->setChecked(mSettings->overridExistingOption());
    ui->addCommentAboveCheckBox->setChecked(mSettings->addCommentDescriptionAboveOption());
    ui->addEOLCommentCheckBox->setChecked(mSettings->addEOLCommentDescriptionOption());
    ui->deleteCommentAboveCheckbox->setChecked(mSettings->deleteAllCommentsAboveOption());

    // scheme data
    reloadColors();
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
    mSettings->setDefaultWorkspace(ui->txt_workspace->text());
    mSettings->setSkipWelcomePage(ui->cb_skipwelcome->isChecked());
    mSettings->setRestoreTabs(ui->cb_restoretabs->isChecked());
    mSettings->setAutosaveOnRun(ui->cb_autosave->isChecked());
    mSettings->setOpenLst(ui->cb_openlst->isChecked());
    mSettings->setJumpToError(ui->cb_jumptoerror->isChecked());
    mSettings->setForegroundOnDemand(ui->cb_bringontop->isChecked());

    // editor page
    mSettings->setColorSchemeIndex(ui->combo_editorTheme->currentIndex());
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
    mSettings->setWriteLog(ui->cb_writeLog->isChecked());
    mSettings->setNrLogBackups(ui->sb_nrLogBackups->value());
    mSettings->setAutoCloseBraces(ui->cb_autoclose->isChecked());

    // MIRO page
    mSettings->setMiroInstallationLocation(ui->miroEdit->text());

    // colors page
    mSettings->writeScheme();

    // misc page
    mSettings->setHistorySize(ui->sb_historySize->value());
    // solver option editor
    mSettings->setOverrideExistingOption(ui->overrideExistingOptionCheckBox->isChecked());
    mSettings->setAddCommentDescriptionAboveOption(ui->addCommentAboveCheckBox->isChecked());
    mSettings->setAddEOLCommentDescriptionOption(ui->addEOLCommentCheckBox->isChecked());
    mSettings->setDeleteAllCommentsAboveOption(ui->deleteCommentAboveCheckbox->isChecked());

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

void SettingsDialog::schemeModified()
{
    emit setModified();
    Scheme::instance()->invalidate();
//    reloadColors();
    for (QWidget *wid: mColorWidgets) {
        static_cast<SchemeWidget*>(wid)->refresh();
    }
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

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("You are about to overwrite your local settings. Are you sure?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();

    if (answer == QMessageBox::Ok) {
        mSettings->importSettings(filePath, mMain);
    }

    emit editorLineWrappingChanged();
    emit editorFontChanged(mSettings->fontFamily(), mSettings->fontSize());
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
    mSettings->setWriteLog(checked);
}

void SettingsDialog::on_sb_nrLogBackups_valueChanged(int arg1)
{
    mSettings->setNrLogBackups(arg1);
}

void SettingsDialog::on_miroBrowseButton_clicked()
{
    QString dir;
    if (mSettings->miroInstallationLocation().isEmpty())
        dir = mSettings->defaultWorkspace();
    else
        dir = mSettings->miroInstallationLocation();
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
        {Scheme::invalid, Scheme::Edit_blockSelectBg},
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
    slot1 = {Scheme::Icon_Line, Scheme::Disable_Line, Scheme::Active_Line, Scheme::Select_Line,
            Scheme::Icon_Back, Scheme::Disable_Back, Scheme::Active_Back, Scheme::Select_Back};
    for (int i = 0; i < slot1.size(); ++i) {
        SchemeWidget *wid = new SchemeWidget(slot1.at(i), box, true);
        wid->setTextVisible(false);
        grid->addWidget(wid, (i/4)+1, (i%4)+1);
        connect(wid, &SchemeWidget::changed, this, &SettingsDialog::schemeModified);
        mColorWidgets.insert(slot1.at(i), wid);
    }
}

void SettingsDialog::reloadColors()
{
    mSettings->readScheme();
}

void SettingsDialog::on_cbSchemes_currentIndexChanged(int index)
{
    if (!mInitializing) {
        Scheme::instance()->setActiveScheme(index);
        schemeModified();
    }
}


}
}


