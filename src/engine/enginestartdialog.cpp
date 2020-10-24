#include "enginestartdialog.h"
#include "ui_enginestartdialog.h"
#include "settings.h"
#include "logger.h"
#include <QPushButton>
#include "engineprocess.h"

namespace gams {
namespace studio {
namespace engine {

const QString CUnavailable("-server not found-");

EngineStartDialog::EngineStartDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::EngineStartDialog), mProc(nullptr)
{
    ui->setupUi(this);
    QFont f = ui->laWarn->font();
    f.setBold(true);
    ui->laWarn->setFont(f);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &EngineStartDialog::buttonClicked);
    ui->edUrl->setText(Settings::settings()->toString(SettingsKey::skEngineUrl));
    ui->edNamespace->setText(Settings::settings()->toString(SettingsKey::skEngineNamespace));
    ui->edUser->setText(Settings::settings()->toString(SettingsKey::skEngineUser));
    connect(ui->edUrl, &QLineEdit::textEdited, this, &EngineStartDialog::urlEdited);
    connect(ui->edUrl, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edNamespace, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edUser, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    connect(ui->edPassword, &QLineEdit::textChanged, this, &EngineStartDialog::textChanged);
    GamsProcess gp;
    QString about = gp.aboutGAMS();
    QRegExp regex("^GAMS Release\\s*:\\s+(\\d\\d\\.\\d).*");
    if (regex.exactMatch(about))
        mLocalGamsVersion = regex.cap(regex.captureCount()).split('.');
    emit textChanged("");
}

EngineStartDialog::~EngineStartDialog()
{
    delete ui;
}

void EngineStartDialog::hiddenCheck()
{
    getVersion();
}

void EngineStartDialog::setProcess(EngineProcess *process)
{
    mProc = process;
    connect(mProc, &EngineProcess::reVersion, this, &EngineStartDialog::reVersion);
    connect(mProc, &EngineProcess::reVersionError, this, &EngineStartDialog::reVersionError);
    emit urlEdited(ui->edUrl->text());
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
    if (ui->edUrl->text().isEmpty()) ui->edUrl->setFocus();
    else if (ui->edNamespace->text().isEmpty()) ui->edNamespace->setFocus();
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

void EngineStartDialog::getVersion()
{
    ui->laEngineVersion->setText(CUnavailable);
    if (mPendingRequest) return;
    if (mProc) {
        mPendingRequest = true;
        mUrlChanged = false;
        mProc->setUrl(mUrl);
        mProc->getVersions();
    }
}

void EngineStartDialog::urlEdited(const QString &text)
{
    if (mOldUrl.endsWith("api", Qt::CaseInsensitive) && !text.endsWith("api", Qt::CaseInsensitive)
            && mOldUrl.length()-2 > text.length() && text.length() > 0) {
        mOldUrl = text.left(text.length()-1);
    } else {
        mOldUrl = text;
    }
    mUrl = mOldUrl;
    int pos = qMin(ui->edUrl->cursorPosition(), mUrl.length());
    ui->edUrl->setText(mUrl);
    ui->edUrl->setCursorPosition(pos);
    mUrlChanged = true;
    getVersion();
}

void EngineStartDialog::textChanged(const QString &/*text*/)
{
    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (!bOk) return;
    bool enabled = !ui->edUrl->text().isEmpty() && !ui->edNamespace->text().isEmpty()
            && !ui->edUser->text().isEmpty() && !ui->edPassword->text().isEmpty();
    if (enabled && ui->laEngineVersion->text() == CUnavailable) {
        getVersion();
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
    mPendingRequest = false;
    if (mUrlChanged) {
        getVersion();
        return;
    }
    ui->laEngineVersion->setText("Engine v"+engineVersion);
    ui->laEngGamsVersion->setText("GAMS v"+gamsVersion);
    if (mUrl != ui->edUrl->text()) {
        int pos = qMin(ui->edUrl->cursorPosition(), mUrl.length());
        int len = ui->edUrl->text().length();
        ui->edUrl->selectAll();
        ui->edUrl->insert(mUrl);
        if (pos < len) {
            ui->edUrl->setCursorPosition(len);
        } else {
            len = mUrl.length();
            ui->edUrl->setSelection(len, pos-len);
        }
    }

    if (!mProc->hasPreviousWorkOption()) {
        bool newerGamsVersion = false;
        QStringList engineGamsVersion = QString(gamsVersion).split('.');
        if (mLocalGamsVersion.at(0).toInt() > engineGamsVersion.at(0).toInt())
            newerGamsVersion = true;
        if (mLocalGamsVersion.at(0).toInt() == engineGamsVersion.at(0).toInt() &&
            mLocalGamsVersion.at(1).toInt() >= engineGamsVersion.at(1).toInt())
            newerGamsVersion = true;
        if (newerGamsVersion) {
            ui->laWarn->setText("Newer local GAMS: Please use \"previousWork=1\"");
        } else {
            ui->laWarn->setText("");
        }
    }
}

void EngineStartDialog::reVersionError(const QString &errorText)
{
    DEB() << "Network error: " << errorText;
    mPendingRequest = false;
    if (mUrlChanged) {
        getVersion();
        return;
    }
    if (mUrl == ui->edUrl->text()) {
        mUrl += (mUrl.endsWith('/') ? "api" : "/api");
        mUrlChanged = false;
        getVersion();
        return;
    }
    ui->laWarn->setText("");
    ui->laEngineVersion->setText(CUnavailable);
    ui->laEngGamsVersion->setText("");
    textChanged("");
}

} // namespace engine
} // namespace studio
} // namespace gams
