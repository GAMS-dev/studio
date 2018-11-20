#include "aboutgamsdialog.h"
#include "ui_aboutgamsdialog.h"
#include "gamsprocess.h"
#include "checkforupdatewrapper.h"

#include <QClipboard>
#include <QDebug>

namespace gams {
namespace studio {

AboutGAMSDialog::AboutGAMSDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutGAMSDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowTitle("License Information");
    ui->label->setAlignment(Qt::AlignLeft);
    ui->LicenseInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->label->setText(licenseInformation());
    ui->horizontalLayout_2->addItem(new QSpacerItem(1, 7, QSizePolicy::Fixed, QSizePolicy::Fixed));
    ui->verticalLayout->addStretch();
    ui->gamslogo->setPixmap(QPixmap(":/img/gams-w24"));
    ui->gamslogo->adjustSize();
    ui->gamslogo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui->label->setContentsMargins(4, 2, 3, 7);
    ui->gamslogo->setContentsMargins(0, 3, 7, 7);
}

AboutGAMSDialog::~AboutGAMSDialog()
{
    delete ui;
}

QString AboutGAMSDialog::studioInfo()
{
    QString ret = "Release: GAMS Studio " + QApplication::applicationVersion() + " ";
    ret += QString(sizeof(void*)==8 ? "64" : "32") + " bit<br/>";
    ret += "Build Date: " __DATE__ " " __TIME__ "<br/><br/>";
    return ret;
}


QString AboutGAMSDialog::licenseInformation()
{
    QString about = "<b><big>GAMS Distribution ";
    about += CheckForUpdateWrapper::distribVersionString();
    about += "</big></b><br/><br/>";
    GamsProcess gproc;
    about += gproc.aboutGAMS().replace("\n", "<br/>");
    about += "<br/><br/>For further information about GAMS please visit ";
    about += "<a href=\"https://www.gams.com\">https://www.gams.com</a>.<br/>";
    return about;
}

void AboutGAMSDialog::on_copylicense_clicked()
{
    GamsProcess gproc;
    QClipboard *clip = QGuiApplication::clipboard();
    clip->setText(studioInfo().replace("<br/>", "\n") + gproc.aboutGAMS());
}

QString AboutGAMSDialog:: aboutStudio()
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

void AboutGAMSDialog::on_close_clicked()
{
    close();
}

}
}
