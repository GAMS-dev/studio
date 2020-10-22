#include "enginestartdialog.h"
#include "ui_enginestartdialog.h"
#include "settings.h"
#include "logger.h"
#include <QPushButton>
#include "engineprocess.h"

namespace gams {
namespace studio {
namespace engine {

const QString CUnavailable("-unavailable-");

EngineStartDialog::EngineStartDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::EngineStartDialog), mProc(nullptr)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &EngineStartDialog::buttonClicked);
    ui->edUrl->setText(Settings::settings()->toString(SettingsKey::skEngineUrl));
    ui->edNamespace->setText(Settings::settings()->toString(SettingsKey::skEngineNamespace));
    ui->edUser->setText(Settings::settings()->toString(SettingsKey::skEngineUser));
    connect(ui->edUrl, &QLineEdit::textEdited, this, &EngineStartDialog::textEdited);
    connect(ui->edUrl, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edNamespace, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edUser, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edPassword, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    emit textChanged("");
}

EngineStartDialog::~EngineStartDialog()
{
    delete ui;
}

void EngineStartDialog::setProcess(EngineProcess *process)
{
    mProc = process;
    connect(mProc, &EngineProcess::reVersion, this, &EngineStartDialog::reVersion);
    connect(mProc, &EngineProcess::reVersionError, this, &EngineStartDialog::reVersionError);
}

EngineProcess *EngineStartDialog::process() const
{
    return mProc;
}

QString EngineStartDialog::url() const
{
    return ui->edUrl->text();
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

void EngineStartDialog::focusEmptyField()
{
    if (ui->edNamespace->text().isEmpty()) ui->edNamespace->setFocus();
    else if (ui->edUser->text().isEmpty()) ui->edUser->setFocus();
    else if (ui->edPassword->text().isEmpty()) ui->edPassword->setFocus();
}

void EngineStartDialog::setEngineVersion(QString version)
{
    ui->laEngineVersion->setText(version);
}

QDialogButtonBox::StandardButton EngineStartDialog::standardButton(QAbstractButton *button) const
{
    if (button == ui->bAlways)
        return QDialogButtonBox::YesToAll;
    return ui->buttonBox->standardButton(button);
}

void EngineStartDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    setFixedSize(size());
}

void EngineStartDialog::buttonClicked(QAbstractButton *button)
{
    bool always = button == ui->bAlways;
    bool start = always || ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok;
    emit ready(start, always);
}

void EngineStartDialog::textEdited(const QString &/*text*/)
{
    mUrl = ui->edUrl->text();
    ui->laEngineVersion->setText(CUnavailable);
    if (mProc) {
        mProc->setUrl(mUrl);
        mProc->setNamespace(ui->edNamespace->text());
        mProc->authenticate(ui->edUser->text(), ui->edPassword->text());
        mProc->getVersions();
    }
}

void EngineStartDialog::textChanged(const QString &/*text*/)
{
    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (!bOk) return;
    bool enabled = !ui->edUrl->text().isEmpty() && !ui->edNamespace->text().isEmpty()
            && !ui->edUser->text().isEmpty() && !ui->edPassword->text().isEmpty()
            && ui->laEngineVersion->text() != CUnavailable;
    if (enabled && ui->laEngineVersion->text() == CUnavailable) {
        mProc->setUrl(mUrl);
        mProc->setNamespace(ui->edNamespace->text());
        mProc->authenticate(ui->edUser->text(), ui->edPassword->text());
        mProc->getVersions();
        enabled = false;
    }
    if (enabled != bOk->isEnabled())
        bOk->setEnabled(enabled);
    if (enabled != ui->bAlways->isEnabled())
        ui->bAlways->setEnabled(enabled);
}

void EngineStartDialog::on_bAlways_clicked()
{
    emit buttonClicked(ui->bAlways);
}

void EngineStartDialog::reVersion(const QString &engineVersion, const QString &gamsVersion)
{
    ui->laEngineVersion->setText(engineVersion);
    ui->laUrl->setText(mUrl);
    mGamsVersion = gamsVersion;
}

void EngineStartDialog::reVersionError(const QString &errorText)
{
    DEB() << "Network error: " << errorText;
    if (mUrl == ui->edUrl->text()) {
        mUrl += (mUrl.endsWith('/') ? "api" : "/api");
        mProc->setUrl(mUrl);
        mProc->setNamespace(ui->edNamespace->text());
        mProc->authenticate(ui->edUser->text(), ui->edPassword->text());
        mProc->getVersions();
        return;
    }
    ui->laEngineVersion->setText(CUnavailable);
    mGamsVersion = QString();
    textChanged("");
}

} // namespace engine
} // namespace studio
} // namespace gams
