#include "enginestartdialog.h"
#include "ui_enginestartdialog.h"
#include "settings.h"

namespace gams {
namespace studio {
namespace engine {

EngineStartDialog::EngineStartDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EngineStartDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &EngineStartDialog::buttonClicked);
    ui->edHost->setText(Settings::settings()->toString(SettingsKey::skEngineHost));
    ui->edNamespace->setText(Settings::settings()->toString(SettingsKey::skEngineNamespace));
    ui->edUser->setText(Settings::settings()->toString(SettingsKey::skEngineUser));

//#ifdef _DEBUG
//    ui->edHost->setText("miro.gams.com");
//    ui->edNamespace->setText("studiotests");
//    ui->edUser->setText("studiotests");
//    ui->edPassword->setText("rercud-qinRa9-wagbew");
//#endif
}

EngineStartDialog::~EngineStartDialog()
{
    delete ui;
}

QString EngineStartDialog::host() const
{
    return ui->edHost->text();
}

QString EngineStartDialog::nSpace() const
{
    return ui->edNamespace->text();
}

QString EngineStartDialog::user() const
{
    return ui->edUser->text();
}

QString EngineStartDialog::password() const
{
    return ui->edPassword->text();
}

void EngineStartDialog::setLastPassword(QString lastPassword)
{
    ui->edPassword->setText(lastPassword);
}

QDialogButtonBox::StandardButton EngineStartDialog::standardButton(QAbstractButton *button) const
{
    return ui->buttonBox->standardButton(button);
}

} // namespace engine
} // namespace studio
} // namespace gams
