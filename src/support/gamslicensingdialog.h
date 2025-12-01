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
#ifndef GAMSLICENSINGDIALOG_H
#define GAMSLICENSINGDIALOG_H

#include <QDialog>

namespace gams {
namespace studio {

class GamsAboutProcess;
class GamsGetKeyProcess;

namespace support {

namespace Ui {
class GamsLicensingDialog;
}
class GamsLicenseInfo;

///
/// \brief The GamsLicensingDialog class represents the implementation of the
///        <a href="https://gams.com/latest/docs/T_STUDIO.html#STUDIO_GAMS_LICENSING">GAMS Licensing</a>
///        dialog.
///
class GamsLicensingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GamsLicensingDialog(const QString &title, QWidget *parent = nullptr);
    ~GamsLicensingDialog();

    static QString studioInfo();
    static QString aboutStudio();
    static QString header();
    static void createLicenseFileFromClipboard(QWidget *parent);

private slots:
    void copyLicenseInfo();
    void installFile();
    void requestAlpLicense();
    void installAlp(int exitCode);
    void updateAboutLabel(int exitCode);

private:
    void fetchGamsLicense();

    void setSolverLines(QStringList &about);

    QString getCurdirForAboutProcess();

    static void writeLicenseFile(QStringList &license, QWidget *parent, bool clipboard);

    static void showInvalidGamsPyMessageBox(QWidget *parent);

private:
    Ui::GamsLicensingDialog *ui;
    QScopedPointer<GamsAboutProcess> mGamsAboutProc;
    QScopedPointer<GamsGetKeyProcess> mGamsGetKeyProc;
};

}
}
}

#endif // GAMSLICENSINGDIALOG_H
