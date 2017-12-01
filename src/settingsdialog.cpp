#include "settingsdialog.h"
#include "ui_settingsdialog.h"

namespace gams {
namespace studio {


SettingsDialog::SettingsDialog(StudioSettings *settings, QWidget *parent) :
    QDialog(parent), mSettings(settings), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setFixedSize(size());

// load from settings to UI

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
    ui->cb_replacetabs->setChecked(mSettings->replaceTabsWithSpaces());
    ui->sb_tabsize->setValue(mSettings->tabSize());
    ui->cb_linewrap->setChecked(mSettings->lineWrap());
}

void SettingsDialog::saveSettings()
{
    // save settings from ui to qsettings
    mSettings->setDefaultWorkspace(ui->txt_workspace->text());
    mSettings->setSkipWelcomePage(ui->cb_skipwelcome->isChecked());
    mSettings->setRestoreTabs(ui->cb_restoretabs->isChecked());
    mSettings->setAutosaveOnRun(ui->cb_autosave->isChecked());
    mSettings->setOpenLst(ui->cb_openlst->isChecked());
    mSettings->setJumpToError(ui->cb_jumptoerror->isChecked());

    mSettings->setFontFamily(ui->fontComboBox->currentFont().family());
    mSettings->setFontSize(ui->sb_fontsize->value());
    mSettings->setShowLineNr(ui->cb_showlinenr->isChecked());
    mSettings->setReplaceTabsWithSpaces(ui->cb_replacetabs->isChecked());
    mSettings->setTabSize(ui->sb_tabsize->value());
    mSettings->setLineWrap(ui->cb_linewrap->isChecked());

    // done
    mSettings->saveSettings();
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button == ui->buttonBox->button(QDialogButtonBox::Apply)) {
        saveSettings();
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        saveSettings();
    } else { // reject
        // do nothing
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

}
}
