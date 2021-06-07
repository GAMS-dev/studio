/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "process.h"
#include "checkforupdatewrapper.h"
#include "solvertablemodel.h"
#include "commonpaths.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "gamslicenseinfo.h"
#include "theme.h"

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QFontDatabase>

namespace gams {
namespace studio {
namespace support {

GamsLicensingDialog::GamsLicensingDialog(const QString &title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GamsLicensingDialog)
{
    ui->setupUi(this);

    createLicenseFile(parent);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowTitle(title);
    ui->label->setText(gamsLicense());
    ui->gamslogo->setPixmap(Theme::icon(":/img/gams-w24").pixmap(ui->gamslogo->size()));
    ui->solverTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    auto dataModel = new SolverTableModel(this);
    auto sortModel = new QSortFilterProxyModel(this);
    sortModel->setSourceModel(dataModel);
    ui->solverTable->setModel(sortModel);
    ui->solverTable->verticalHeader()->setMinimumSectionSize(1);
    ui->solverTable->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
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
    about << CheckForUpdateWrapper::distribVersionString();
    about << "</big></b><br/><br/>";

    GamsProcess gproc;
    bool licenseLines = false;
    for (auto line : gproc.aboutGAMS().split("\n")) {
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

    return about.join("");
}

void GamsLicensingDialog::createLicenseFile(QWidget *parent)
{
    GamsLicenseInfo licenseInfo;
    auto license = licenseInfo.licenseFromClipboard();
    if (license.isEmpty() || !licenseInfo.isLicenseValid(license))
        return;

    QFile licenseFile(CommonPaths::gamsLicenseFilePath(licenseInfo.gamsDataLocations()));
    if (licenseFile.exists()) {
        auto result = QMessageBox::question(parent,
                                            "Overwrite current GAMS license file?",
                                            "It looks like there is a GAMS license in the clipboard. "
                                            "Do you want to overwrite your current license file from this clipboard text? "
                                            "Your current license location is: " +
                                            QDir::toNativeSeparators(licenseFile.fileName()));
        if (result == QMessageBox::No)
            return;
    } else {
        auto result = QMessageBox::question(parent,
                                            "Create GAMS license file?",
                                            "It looks like there is a GAMS license in the clipboard. "
                                            "Do you want to create a license file from this clipboard text? "
                                            "Your GAMS license location will be: " +
                                            QDir::toNativeSeparators(licenseFile.fileName()));
        if (result == QMessageBox::No)
            return;

        auto licensePath = QFileInfo(licenseFile).absolutePath();
        QDir(licensePath).mkpath(".");
    }

    if (licenseFile.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream stream(&licenseFile);
        stream << license.join("\n");
        licenseFile.close();
    } else {
        QMessageBox::critical(parent,
                              "Unable to write gamslice.txt",
                              "Unable to write " +
                              QDir::toNativeSeparators(licenseFile.fileName()) +
                              ": " + licenseFile.errorString());
    }
}

void GamsLicensingDialog::on_copylicense_clicked()
{
    GamsProcess gproc;
    QClipboard *clip = QGuiApplication::clipboard();
    clip->setText(studioInfo().replace("<br/>", "\n") + gproc.aboutGAMS().replace("#L", ""));
}

QString GamsLicensingDialog::header()
{
    return "<b><big>GAMS Studio " + QApplication::applicationVersion() + "</big></b>";
}

QString GamsLicensingDialog::aboutStudio()
{
    QString about = studioInfo();
    about += "Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com><br/>";
    about += "Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com><br/><br/>";
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
