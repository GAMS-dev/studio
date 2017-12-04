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
    loadSettings();
    setModifiedStatus(false);

    // connections to track modified status
    connect(ui->txt_workspace, &QLineEdit::textChanged, this, &SettingsDialog::setModified);
    connect(ui->cb_skipwelcome, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_restoretabs, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_autosave, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_openlst, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->cb_jumptoerror, &QCheckBox::clicked, this, &SettingsDialog::setModified);
    connect(ui->fontComboBox, &QFontComboBox::currentFontChanged, this, &SettingsDialog::setModified);
    connect(ui->sb_fontsize, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::setModified);
    connect(ui->cb_showlinenr, &QCheckBox::clicked, this, &SettingsDialog::setModified);
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
//    ui->cb_replacetabs->setChecked(mSettings->replaceTabsWithSpaces());
//    ui->sb_tabsize->setValue(mSettings->tabSize());
    ui->cb_linewrap->setChecked(mSettings->lineWrap());
}

void SettingsDialog::setModified()
{
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::setModifiedStatus(bool status)
{
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(status);
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
//    mSettings->setReplaceTabsWithSpaces(ui->cb_replacetabs->isChecked());
//    mSettings->setTabSize(ui->sb_tabsize->value());
    mSettings->setLineWrap(ui->cb_linewrap->isChecked());

    // done
    mSettings->saveSettings();
    setModifiedStatus(false);
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
}

void SettingsDialog::on_fontComboBox_currentIndexChanged(const QString &arg1)
{
    mSettings->updateEditors(arg1, ui->sb_fontsize->value());
}

void SettingsDialog::on_sb_fontsize_valueChanged(int arg1)
{
    mSettings->updateEditors(ui->fontComboBox->currentFont().family(), arg1);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

}
}

