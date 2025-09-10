/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "commonpaths.h"
#include "gamslicenseinfo.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "process/gamsaboutprocess.h"
#include "process/gamsgetkeyprocess.h"
#include "process/gamsprobeprocess.h"
#include "theme.h"

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QSortFilterProxyModel>
#include <QFontDatabase>
#include <QFileDialog>
#include <QTextDocument>

namespace gams {
namespace studio {
namespace support {

GamsLicensingDialog::GamsLicensingDialog(const QString &title, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GamsLicensingDialog)
    , mGamsAboutProc(new GamsAboutProcess(this))
    , mGamsGetKeyProc(new GamsGetKeyProcess(this))
{
    ui->setupUi(this);
    createLicenseFileFromClipboard(parent);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowTitle(title);
    fetchGamsLicense();
    ui->gamslogo->setPixmap(Theme::icon(":/img/gams-w24").pixmap(ui->gamslogo->size()));
    QRegularExpression regex("^\\d+$");
    ui->cdEdit->setValidator(new QRegularExpressionValidator(regex, this));
    connect(ui->copyButton, &QPushButton::clicked, this, &GamsLicensingDialog::copyLicenseInfo);
    connect(ui->fileButton, &QPushButton::clicked, this, &GamsLicensingDialog::installFile);
    connect(ui->alpButton, &QPushButton::clicked, this, &GamsLicensingDialog::requestAlpLicense);
    connect(ui->idEdit, &QLineEdit::returnPressed, this, &GamsLicensingDialog::requestAlpLicense);
    connect(ui->cdEdit, &QLineEdit::returnPressed, this, &GamsLicensingDialog::requestAlpLicense);
    connect(ui->opEdit, &QLineEdit::returnPressed, this, &GamsLicensingDialog::requestAlpLicense);
    connect(mGamsAboutProc.get(), &GamsAboutProcess::finished, this, &GamsLicensingDialog::updateAboutLabel);
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

void GamsLicensingDialog::fetchGamsLicense()
{
    ui->label->setText("Fetching GAMS system information. Please wait...");
    mGamsAboutProc->clearState();
    mGamsAboutProc->execute();
}

void GamsLicensingDialog::setSolverLines(QStringList& about)
{
    about << "<br/>Licensed GAMS solvers:";
    GamsLicenseInfo liceInfo;
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
    about << "</ul><br/>";
}

void GamsLicensingDialog::writeLicenseFile(QStringList &license, QWidget *parent, bool clipboard)
{
    auto liceFile = GamsLicenseInfo::licenseLocation();
    QFile licenseFile(liceFile);
    if (licenseFile.exists()) {
        QString text;
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
        auto result = QMessageBox::question(parent, "Overwrite current GAMS license file?", text);
        if (result == QMessageBox::No)
            return;
    } else {
        QString text;
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
        auto result = QMessageBox::question(parent, "Create GAMS license file?", text);
        if (result == QMessageBox::No)
            return;
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
        fetchGamsLicense();
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
            fetchGamsLicense();
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
        fetchGamsLicense();
        return;
    }
    if (licenseInfo.isGamsLicense(license)) {
        writeLicenseFile(license, this, false);
        fetchGamsLicense();
    } else {
        showInvalidGamsPyMessageBox(this);
    }
}

void GamsLicensingDialog::updateAboutLabel(int exitCode)
{
    if (exitCode) {
        ui->label->setText("Error: Fetching GAMS system information. Please check the system log.");
        SysLogLocator::systemLog()->append(mGamsAboutProc->logMessages(), LogMsgType::Error);
    }
    QStringList about;
    about << "<b><big>GAMS Distribution ";
    GamsLicenseInfo licenseInfo;
    about << licenseInfo.localDistribVersionString();
    about << "</big></b><br/><br/>";

    bool licenseLines = false;
    auto lines = mGamsAboutProc->content().split('\n', Qt::SkipEmptyParts, Qt::CaseInsensitive);
    if (lines.size() >= 3) {
        lines.removeFirst();
        lines.removeLast();
        lines.removeLast();
    }
    for(const auto &line : std::as_const(lines)) {
        if (licenseLines) {
            if (line.startsWith("#L")) {
                continue;
            } else if (line.startsWith("Licensed platform:")) {
                licenseLines = false;
                about << "</pre>" " \r""<br/>" << line << "<br/>";
            } else {
                about << line + "\n";
            }
        } else if (line.startsWith("License ")) {
            about << line << "<br/>";
            licenseLines = true;
            about << "<pre style=\"font-family:'Courier New',monospace\">";
        } else if (line.contains("License file not found")) {
            about << "<br/>" << line  << "<br/>";
        } else if (line.contains("gamslice.txt")) {
            about << line;
        } else {
            about << line << "<br/>";
        }
    }
    setSolverLines(about);

    ui->label->setText(about.join(""));
}

QString GamsLicensingDialog::header()
{
    return "<b><big>GAMS Studio " + QApplication::applicationVersion() + "</big></b>";
}

QString GamsLicensingDialog::aboutStudio()
{
    QString about = studioInfo();
    about += "Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com><br/>";
    about += "Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com><br/><br/>";
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

}
}
}
