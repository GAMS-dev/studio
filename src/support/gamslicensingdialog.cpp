/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "gamslicensingdialog.h"
#include "ui_gamslicensingdialog.h"
#include "gamslicenseinfo.h"
#include "licensefetcher.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "file/uncpath.h"
#include "process/gamsaboutprocess.h"
#include "process/gamsgetkeyprocess.h"
#include "process/gamsprobeprocess.h"
#include "theme.h"
#include "logger.h"

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QFontDatabase>
#include <QFileDialog>
#include <QTextDocument>

using namespace gams::studio::support::LicenseStateEnum;

namespace gams {
namespace studio {
namespace support {

GamsLicensingDialog::GamsLicensingDialog(const QString &title, LicenseFetcher *licenseFetcher, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GamsLicensingDialog)
    , mLicenseFetcher(licenseFetcher)
    , mGamsGetKeyProc(new GamsGetKeyProcess(this))
{
    ui->setupUi(this);
    createLicenseFileFromClipboard(parent);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowTitle(title);
    getGamsLicenseText();
    ui->gamslogo->setPixmap(Theme::icon(":/img/gams-w24").pixmap(ui->gamslogo->size()));
    QRegularExpression regex("^\\d+$");
    ui->cdEdit->setValidator(new QRegularExpressionValidator(regex, this));
    connect(ui->copyButton, &QPushButton::clicked, this, &GamsLicensingDialog::copyLicenseInfo);
    connect(ui->fileButton, &QPushButton::clicked, this, &GamsLicensingDialog::installFile);
    connect(ui->alpButton, &QPushButton::clicked, this, &GamsLicensingDialog::requestAlpLicense);
    connect(ui->idEdit, &QLineEdit::returnPressed, this, &GamsLicensingDialog::requestAlpLicense);
    connect(ui->cdEdit, &QLineEdit::returnPressed, this, &GamsLicensingDialog::requestAlpLicense);
    connect(ui->opEdit, &QLineEdit::returnPressed, this, &GamsLicensingDialog::requestAlpLicense);
    connect(mLicenseFetcher, &LicenseFetcher::stateChanged, this, &GamsLicensingDialog::updateAboutLabel);
    connect(mGamsGetKeyProc.get(), &GamsGetKeyProcess::finished, this, &GamsLicensingDialog::installAlp);
}

GamsLicensingDialog::~GamsLicensingDialog()
{
    delete ui;
}

QString GamsLicensingDialog::studioInfo()
{
    QString ret = "Release: GAMS Studio " + QApplication::applicationVersion() + " ";
    ret += QString(sizeof(void*)==8 ? "64" : "32") + " bit<br/>";
    ret += "Build Date: " __DATE__ " " __TIME__ "<br/><br/>";
    return ret;
}

void GamsLicensingDialog::getGamsLicenseText(bool forceFetch)
{
    if (mLicenseFetcher->state() == lsChecking) {
        if (forceFetch)
            mLicenseFetcher->stopFetching(); // skip fetching
        else
            return; // Wait until the slot is activated
    }
    mLicenseFetcher->fetchGamsLicense();
    ui->label->setText(mLicenseFetcher->formattedContent());
}

void GamsLicensingDialog::setSolverLines(GamsLicenseInfo &liceInfo, QStringList& about) const
{
    about << "<br/>Licensed GAMS solvers:";
    QHash<QString, QStringList> groups;
    auto solverNames = liceInfo.solverNames().values();
    for (const auto& name : std::as_const(solverNames)) {
        auto id = liceInfo.solverId(name);
        auto group = liceInfo.solverLicense(name, id);
        if (groups.contains(group)) {
            groups[group].append(name);
        } else {
            QStringList solvers { name };
            groups[group] = solvers;
        }
    }
    about << "<ul>";
    if (groups.contains("Full")) {
        about << QString("<li>Full: %2</li>").arg(groups["Full"].join(", "));
    }
    if (groups.contains("Evaluation")) {
        about << QString("<li>Time limited: %2</li>").arg(groups["Evaluation"].join(", "));
    }
    if (groups.contains("Community")) {
        about << QString("<li>Community: %2</li>").arg(groups["Community"].join(", "));
    }
    if (groups.contains("Demo")) {
        about << QString("<li>Demo: %2</li>").arg(groups["Demo"].join(", "));
    }
    if (groups.contains("Expired")) {
        about << QString("<li>Expired: %2</li>").arg(groups["Expired"].join(", "));
    }
    about << "</ul>";
}

void GamsLicensingDialog::setNonSolverLines(GamsLicenseInfo &liceInfo, QStringList &about) const
{
    QStringList lines;
    if (liceInfo.hasMiroConnector())
        lines << "<li>MIRO Connector</li>";
    if (liceInfo.hasSecureModule())
        lines << "<li>Secure Module</li>";
    if (!lines.isEmpty()) {
        about << "Licensed non-solver components:<ul>";
        about << lines;
        about << "</ul>";
    }
}

QString GamsLicensingDialog::getCurdirForAboutProcess()
{// TODO (AF): Remove workaround when gams curdir param has Unicode support
    QString curdir;
#ifdef _WIN64
    GamsLicenseInfo liceInfo;
    auto dataLocations = liceInfo.gamsDataLocations();
    if (dataLocations.isEmpty())
        curdir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    else {
        curdir = dataLocations.first();
        if (curdir.startsWith("\\\\")) {
            curdir = file::UncPath::unc()->toMappedPath(curdir, true);
            if (curdir.isEmpty())
                curdir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        }
    }
#else
    curdir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#endif
    SysLogLocator::systemLog()->append("CURDIR: "+curdir, LogMsgType::Info);
    return curdir;
}

void GamsLicensingDialog::writeLicenseFile(QStringList &license, QWidget *parent, bool clipboard, bool quiet = false)
{
    auto liceFile = GamsLicenseInfo::licenseLocation();
    QFile licenseFile(liceFile);
    if (licenseFile.exists()) {
        QString text;
        if (!quiet) {
            if (clipboard) {
                text.append("It looks like there is a GAMS license in the clipboard. ");
                text.append("Do you want to overwrite your current license file from this clipboard text? ");
                text.append("Your current license location is: ");
                text.append(QDir::toNativeSeparators(licenseFile.fileName()));
            } else {
                text.append("Do you want to overwrite your current license file with the selected license? ");
                text.append("Your current license location is: ");
                text.append(QDir::toNativeSeparators(licenseFile.fileName()));
            }
        }
        if (!text.isEmpty()) {
            auto result = QMessageBox::question(parent, "Overwrite current GAMS license file?", text);
            if (result == QMessageBox::No)
                return;
        }
    } else {
        QString text;
        if (!quiet) {
            if (clipboard) {
                text.append("It looks like there is a GAMS license in the clipboard. ");
                text.append("Do you want to create a license file from this clipboard text? ");
                text.append("Your GAMS license location will be: ");
                text.append(QDir::toNativeSeparators(licenseFile.fileName()));
            } else {
                text.append("Do you want to create a license file based on the selected license? ");
                text.append("Your GAMS license location will be: ");
                text.append(QDir::toNativeSeparators(licenseFile.fileName()));
            }
        }
        if (!text.isEmpty()) {
            auto result = QMessageBox::question(parent, "Create GAMS license file?", text);
            if (result == QMessageBox::No)
                return;
        }
        auto licensePath = QFileInfo(licenseFile).absolutePath();
        bool success = QDir(licensePath).mkpath(".");
        if (!success) {
            auto msg = QString("Could not create license file path (%1).").arg(licensePath);
            SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
            QMessageBox::critical(parent, "Unable to create license file path", msg);
            return;
        }
    }

    if (licenseFile.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream stream(&licenseFile);
        stream << license.join("\n") << "\n";
        licenseFile.close();
    } else {
        auto msg = QString("Unable to write %1 : %2").arg(QDir::toNativeSeparators(licenseFile.fileName()),
                                                          licenseFile.errorString());
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        QMessageBox::critical(parent, "Unable to write gamslice.txt", msg);
    }
}

void GamsLicensingDialog::showInvalidGamsPyMessageBox(QWidget *parent)
{
    auto msg = "The selected license seems to be a GAMSPy or GAMSPy++ license and will not be installed. Please retry with a GAMS license.";
    SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
    QMessageBox::critical(parent, "Invalid License", msg);
}

void GamsLicensingDialog::createLicenseFileFromClipboard(QWidget *parent)
{
    GamsLicenseInfo licenseInfo;
    auto license = licenseInfo.licenseFromClipboard();
    if (license.isEmpty()) {
        return;
    }
    if (!licenseInfo.isLicenseValid(license)) {
        auto msg = "The license is invalid and has not been installed. The license is";
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        SysLogLocator::systemLog()->append(license.join("\n"), LogMsgType::Error);
        return;
    }
    if (licenseInfo.isGamsLicense(license)) {
        writeLicenseFile(license, parent, true);
    } else {
        auto msg = "The license in the clipboard seams to be a GAMSPy or GAMSPy++ license and will not be installed. Please retry with a GAMS license.";
        QMessageBox::critical(parent, "Invalid License", msg);
    }
}

void GamsLicensingDialog::copyLicenseInfo()
{
    ui->errorLabel->clear();
    QClipboard *clip = QGuiApplication::clipboard();
    QTextDocument doc;
    doc.setHtml(ui->label->text());
    QStringList stats { doc.toPlainText() };
    GamsprobeProcess proc;
    auto data = proc.execute();
    auto error = proc.errorMessage();
    if (!proc.errorMessage().isEmpty()) {
        auto msg = "The gamsprobe content can not be added to the license information.";
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        SysLogLocator::systemLog()->append(error, LogMsgType::Error);
    } else {
        stats << data;
    }
    clip->setText(stats.join(""));
}

void GamsLicensingDialog::installFile()
{
    ui->errorLabel->clear();
    auto fileName = QFileDialog::getOpenFileName(this,
                                                 "Open License File",
                                                 QDir::homePath(),
                                                 tr("Text Files (*.txt)"));
    if (fileName.isEmpty() || !fileName.endsWith(".txt"))
        return;
    GamsLicenseInfo licenseInfo;
    auto license = licenseInfo.licenseFromFile(fileName);
    if (license.isEmpty() || !licenseInfo.isLicenseValid(license)) {
        auto msg = "The selected license file is invalid and has not been installed.";
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        QMessageBox::critical(this, "Invalid License", msg);
        return;
    }
    if (licenseInfo.isGamsLicense(license)) {
        writeLicenseFile(license, this, false);
        getGamsLicenseText(true);
    } else {
        showInvalidGamsPyMessageBox(this);
    }
}

void GamsLicensingDialog::requestAlpLicense()
{
    ui->errorLabel->clear();
    ui->label->setText("Fetching license. Please wait...");
    mGamsGetKeyProc->clearState();
    mGamsGetKeyProc->setAlpId(ui->idEdit->text().trimmed());
    mGamsGetKeyProc->setCheckoutDuration(ui->cdEdit->text());
    mGamsGetKeyProc->setOnPremSever(ui->opEdit->text().trimmed());
    mGamsGetKeyProc->setOnPremCertPath(GamsLicenseInfo::licenseDirectory());
    mGamsGetKeyProc->execute();
}

void GamsLicensingDialog::installAlp(int exitCode)
{
    ui->errorLabel->clear();
    auto license = mGamsGetKeyProc->content().split("\n", Qt::SkipEmptyParts);
    if (license.size() > 8) {
        QStringList temp;
        temp.reserve(9);
        for (int i = license.size() - 8; i < license.size(); ++i)
            temp << license.at(i);
        license = temp;
    }
    auto log = mGamsGetKeyProc->logMessages();
    if (!log.isEmpty()) {
        if (exitCode) {
            QString msg { "The license has not been installed." };
            SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
            QString errmsg("Errors occured while running gamsgetkey (exit code %1). " \
                           "Please check the system log or use the Studio command line option --network-log.");
            ui->errorLabel->setText(errmsg.arg(QString::number(exitCode)));
            mGamsGetKeyProc->writeLogToFile(msg);
            mGamsGetKeyProc->writeLogToFile(errmsg.arg(QString::number(exitCode)));
            SysLogLocator::systemLog()->append(log, LogMsgType::Error);
            mGamsGetKeyProc->writeLogToFile(log);
            getGamsLicenseText(true);
            return;
        } else {
            SysLogLocator::systemLog()->append(log, LogMsgType::Info);
            mGamsGetKeyProc->writeLogToFile(log);
        }
    }
    for (int i=0; i<license.size(); ++i) {
        license[i] = license[i].trimmed();
    }
    GamsLicenseInfo licenseInfo;
    if (!licenseInfo.isLicenseValid(license)) {
        QString msg = "The license is invalid and has not been installed. The license is";
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        mGamsGetKeyProc->writeLogToFile(msg);
        if (license.isEmpty()) {
            mGamsGetKeyProc->writeLogToFile("<empty license>");
            SysLogLocator::systemLog()->append("<empty license>", LogMsgType::Error);
        } else {
            mGamsGetKeyProc->writeLogToFile(license.join("\n"));
            SysLogLocator::systemLog()->append(license.join("\n"), LogMsgType::Error);
        }
        ui->errorLabel->setText("The license is invalid and has not been installed. " \
                                "Please check the system log or use the Studio command line option --network-log.");
        getGamsLicenseText(true);
        return;
    }
    if (licenseInfo.isGamsLicense(license)) {
        bool quiet = !ui->cdEdit->text().isEmpty();
        writeLicenseFile(license, this, false, quiet);
        getGamsLicenseText(true);
    } else {
        showInvalidGamsPyMessageBox(this);
    }
}

void GamsLicensingDialog::updateAboutLabel()
{
    int exitCode = mLicenseFetcher->lastExitCode();
    if (exitCode) {
        ui->label->setText("Error: Fetching GAMS system information. Please check the system log.");
        SysLogLocator::systemLog()->append(mLicenseFetcher->lastErrorMessage(), LogMsgType::Error);
    }
    QStringList about;
    about << "<b><big>GAMS Distribution ";
    GamsLicenseInfo licenseInfo;
    about << licenseInfo.localDistribVersionString();
    about << "</big></b><br/><br/>";
    about << mLicenseFetcher->formattedContent();
    setSolverLines(licenseInfo, about);
    setNonSolverLines(licenseInfo, about);
    about << "<br/>";

    ui->idEdit->setText(mLicenseFetcher->accessCode());
    ui->cdEdit->setEnabled((mLicenseFetcher->state() >= lsNet && mLicenseFetcher->state() < lsNetCheckout)
                           || mLicenseFetcher->state() == LicenseStateEnum::lsNetCheckoutInvalid);
    if (ui->cdEdit->isEnabled())
        ui->cdEdit->setPlaceholderText("Checkout duration in hours (optional)");
    else if (mLicenseFetcher->state() >= lsNetCheckout)
        ui->cdEdit->setPlaceholderText("Network license already checked out");
    else
        ui->cdEdit->setPlaceholderText("Checkout duration (for network license only)");
    if (!ui->cdEdit->isEnabled())
        ui->cdEdit->setText("");
    on_cdEdit_textChanged(ui->cdEdit->text());
    ui->label->setText(about.join(""));
}

QString GamsLicensingDialog::header()
{
    return "<b><big>GAMS Studio " + QApplication::applicationVersion() + "</big></b>";
}

QString GamsLicensingDialog::aboutStudio()
{
    QString about = studioInfo();
    about += "Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com><br/>";
    about += "Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com><br/><br/>";
    about += "This program is free software: you can redistribute it and/or modify ";
    about += "it under the terms of the GNU General Public License as published by ";
    about += "the Free Software Foundation, either version 3 of the License, or ";
    about += "(at your option) any later version.<br/><br/>";
    about += "This program is distributed in the hope that it will be useful, ";
    about += "but WITHOUT ANY WARRANTY; without even the implied warranty of ";
    about += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the ";
    about += "GNU General Public License for more details.<br/><br/>";
    about += "You should have received a copy of the GNU General Public License ";
    about += "along with this program. If not, see ";
    about += "<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.<br/><br/>";
    about += "The source code of the program can be accessed at ";
    about += "<a href=\"https://github.com/GAMS-dev/studio\">https://github.com/GAMS-dev/studio/</a></p>.";
    return about;
}

void GamsLicensingDialog::on_cdEdit_textChanged(const QString &text)
{
    QList<LicenseState> checkoutStates = {lsNet, lsNetEnd, lsNetCheckoutInvalid};
    ui->alpButton->setText((checkoutStates.contains(mLicenseFetcher->state()) && !text.isEmpty()) ? "Checkout"
                                                                                                  : "Install License");
}


}
}
}
