#include "abouthandler.h"
#include "ui_abouthandler.h"
#include "gamsprocess.h"
#include <QMessageBox>
#include <QSpacerItem>
#include <QGridLayout>
#include <QClipboard>
#include <QDebug>
#include <QTextStream>

namespace gams {
namespace studio {

AboutHandler::AboutHandler(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutHandler)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QSpacerItem* horizontalSpacer = new QSpacerItem(560, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
    QGridLayout* layout = (QGridLayout*)AboutHandler::layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    ui->LicenseInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->label->setText(licenseInformation());
    this->setWindowTitle("License Information");
    ui->label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->label->setWordWrap(true);
    ui->gamslogo->setPixmap(QPixmap(":/img/gams-w24"));
    ui->label->setScaledContents(false); //check this later after setting up a legit license text
    ui->label->adjustSize();
    ui->gamslogo->adjustSize();
}

QString AboutHandler::studioInfo()
{
    QString ret = "Release: GAMS Studio " + QApplication::applicationVersion() + " ";
    ret += QString(sizeof(void*)==8 ? "64" : "32") + " bit<br/>";
    ret += "Build Date: " __DATE__ " " __TIME__ "<br/><br/>";
    return ret;
}

AboutHandler::~AboutHandler()
{
    delete ui;
}

QString AboutHandler::licenseInformation()
{
    QString about = "<br/><br/><b><big>GAMS Distribution ";
    about += "</big></b><br/><br/>";
    GamsProcess gproc;
    about += gproc.aboutGAMS().replace("\n", "<br/>");
    about += "<br/><br/>For further information about GAMS please visit ";
    about += "<a href=\"https://www.gams.com\">https://www.gams.com</a>.<br/>";

//    QString about ="Release: GAMS Studio 0.9.4 64 bit<br/>";
//    about +="Build Date: Oct  2 2018 16:28:38<br/>";
//    about +="<br/>";
//    about +="GAMS Release     : 25.2.0 r2a23d76d LEX-LEG x86 64bit/Linux<br/>";
//    about +="Release Date     :  2Aug18";
//    about +="To use this GAMS release without any limitations, you must";
//    about +="have a valid license file for this platform with maintenance";
//    about +="expiration date later than Jul 26, 2018.";
//    about += "<br/>";
//    about +="System Directory : /home/rogo/gams/gams25.2_linux_x64_64_sfx/<br/>";
//    about += "<br/>";
//    about +="License          : /home/rogo/gams/gams25.2_linux_x64_64_sfx/gamslice.txt<br/>";
//    about +="License file not found (RC=2)<br/>";
//    about +="SysMsg: No such file or directory<br/>";
    return about;
}

void AboutHandler::on_copylicense_clicked()
{
    GamsProcess gproc;
    QClipboard *clip = QGuiApplication::clipboard();
    clip->setText(studioInfo().replace("<br/>", "\n") + gproc.aboutGAMS());
}

QString AboutHandler:: aboutStudio()
{
    QString about = "<b><big>GAMS Studio " + QApplication::applicationVersion() + "</big></b><br/><br/>";
    about += studioInfo();
    about += "Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com><br/>";
    about += "Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com><br/><br/>";
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
    about += "<a href=\"https://github.com/GAMS-dev/studio\">https://github.com/GAMS-dev/studio/</a>.";
    return about;
}

void AboutHandler::on_close_clicked()
{
    close();
}

}
}
