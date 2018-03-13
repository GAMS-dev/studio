/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "gamspaths.h"
#include "exception.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QMessageBox>

namespace gams {
namespace studio {

GAMSPaths::GAMSPaths()
{

}

// TODO(AF) linux
QString GAMSPaths::systemDir() {
    QString gamsPath;
    const QString subPath = QString(QDir::separator()).append("..");
#if __APPLE__
    const QString appDirPath = QApplication::applicationDirPath();
    QRegExp pathRegExp("^((?:.\\w+)*\\d+\\.\\d+).*");
    if (pathRegExp.indexIn(appDirPath) != -1) {
        gamsPath = pathRegExp.cap(1) + QDir::separator() + "sysdir";
    }
#elif __unix__
    QFileInfo fileInfo(qgetenv("APPIMAGE"));
    gamsPath = fileInfo.absoluteDir().path().append(subPath);
#else
    gamsPath = QApplication::applicationDirPath().append(subPath);
#endif
    QString path = QStandardPaths::findExecutable("gams", {gamsPath});
    if (path.isEmpty()) {
        gamsPath = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
        if (gamsPath.isEmpty()) EXCEPT() << "GAMS not found in PATH.";
    }

#ifdef _WIN32
    QFileInfo joat64(gamsPath + QDir::separator() + "joatdclib64.dll");
    bool is64 = (sizeof(int*) == 8) ? true : false;
    if (!is64 && joat64.exists())
        EXCEPT() << "GAMS Studio is 32 bit but 64 bit GAMS installation found. System directory: " << gamsPath;
    if (is64 && !joat64.exists())
        EXCEPT() << "GAMS Studio is 64 bit but 32 bit GAMS installation found. System directory: " << gamsPath;
#endif

    return gamsPath;
}

QString GAMSPaths::systemDocumentsDir()
{
    QDir sysDocDir = QDir::cleanPath(systemDir() + "/docs");
    return sysDocDir.path();
}

QString GAMSPaths::userDocumentsDir()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (dir.isEmpty())
        FATAL() << "Unable to access user documents location";
    QDir userDocumentsDir = QDir::cleanPath(dir + "/GAMSStudio");
    if(!userDocumentsDir.exists())
        userDocumentsDir.mkpath(".");
    return userDocumentsDir.path();
}

QString GAMSPaths::userModelLibraryDir()
{
    QDir userModelLibraryDir(userDocumentsDir() + "/modellibs");
    if(!userModelLibraryDir.exists())
        userModelLibraryDir.mkpath(".");
    return userModelLibraryDir.path();
}

QString GAMSPaths::defaultWorkingDir()
{
    QDir defWorkingDir(userDocumentsDir() + "/workspace");
    if(!defWorkingDir.exists())
        defWorkingDir.mkpath(".");
    return defWorkingDir.path();
}

}
}
