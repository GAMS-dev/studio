/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#ifndef ABOUTGAMSDIALOG_H
#define ABOUTGAMSDIALOG_H

#include <QDialog>

namespace gams {
namespace studio {
namespace support {

namespace Ui {
class AboutGAMSDialog;
}

class AboutGAMSDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutGAMSDialog(const QString &title, QWidget *parent = nullptr);
    ~AboutGAMSDialog();

    static QString studioInfo();
    static QString aboutStudio();
    static QString header();

private:
    QString gamsLicense();
    void createLicenseFile(QWidget *parent);

private slots:
    void on_copylicense_clicked();

private:
    Ui::AboutGAMSDialog *ui;
};

}
}
}
#endif // ABOUTGAMSDIALOG_H
