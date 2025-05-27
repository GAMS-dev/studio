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
#include "process/gamsprocess.h"
#include "process/gamsgetkeyprocess.h"
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
{
    ui->setupUi(this);
    createLicenseFileFromClipboard(parent);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowTitle(title);
    ui->label->setText(gamsLicense());
    ui->gamslogo->setPixmap(Theme::icon(":/img/gams-w24").pixmap(ui->gamslogo->size()));
    QRegularExpression regex("^\\d+$");
    ui->cdEdit->setValidator(new QRegularExpressionValidator(regex, this));
    connect(ui->copyButton, &QPushButton::clicked, this, &GamsLicensingDialog::copyLicenseInfo);
    connect(ui->fileButton, &QPushButton::clicked, this, &GamsLicensingDialog::installFile);
    connect(ui->alpButton, &QPushButton::clicked, this, &GamsLicensingDialog::installAlp);
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

QString GamsLicensingDialog::gamsLicense()
{
    QStringList about;
    about << "<b><big>GAMS Distribution ";
    GamsLicenseInfo licenseInfo;
    about << licenseInfo.localDistribVersionString();
    about << "</big></b><br/><br/>";

    GamsProcess gproc;
    bool licenseLines = false;
    const auto lines = gproc.aboutGAMS().split("\n");
    for(const auto &line : lines) {
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

    return about.join("");
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

void GamsLicensingDialog::writeLicenseFile(GamsLicenseInfo &licenseInfo, QStringList &license,
                                           QWidget *parent, bool clipboard)
{
    auto liceFile = licenseInfo.gamsConfigLicenseLocation();
    if (liceFile.isEmpty()) {
        liceFile = licenseInfo.gamsDataLocations().constFirst() + "/" + CommonPaths::licenseFile();
    }
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
    QMessageBox::critical(parent, "Invalid License", msg);
}

void GamsLicensingDialog::createLicenseFileFromClipboard(QWidget *parent)
{
    GamsLicenseInfo licenseInfo;
    auto license = licenseInfo.licenseFromClipboard();
    if (license.isEmpty())
        return;
    if (!licenseInfo.isLicenseValid(license)) {
        auto msg = "The license is invalid and has not been installed. The license is";
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        SysLogLocator::systemLog()->append(license.join("\n"), LogMsgType::Error);
        return;
    }
    if (licenseInfo.isGamsLicense(license)) {
        writeLicenseFile(licenseInfo, license, parent, true);
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
    clip->setText(doc.toPlainText());
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
        writeLicenseFile(licenseInfo, license, this, false);
        ui->label->setText(gamsLicense());
    } else {
        showInvalidGamsPyMessageBox(this);
    }
}

void GamsLicensingDialog::installAlp()
{
    ui->errorLabel->clear();
    GamsGetKeyProcess proc;
    proc.setAlpId(ui->idEdit->text().trimmed());
    proc.setCheckouDuration(ui->cdEdit->text());
    auto license = proc.execute().split("\n", Qt::SkipEmptyParts);
    for (int i=0; i<license.size(); ++i) {
        license[i] = license[i].trimmed();
    }
    GamsLicenseInfo licenseInfo;
    if (license.isEmpty() || !licenseInfo.isLicenseValid(license)) {
        auto str = license.join(" ");
        ui->errorLabel->setText(str);
        SysLogLocator::systemLog()->append(str, LogMsgType::Error);
        return;
    }
    if (licenseInfo.isGamsLicense(license)) {
        writeLicenseFile(licenseInfo, license, this, false);
        ui->label->setText(gamsLicense());
    } else {
        showInvalidGamsPyMessageBox(this);
    }
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
