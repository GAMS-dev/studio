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
#include "commonpaths.h"
#include "exception.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace gams {
namespace studio {

CommonPaths::CommonPaths()
{

}

QString CommonPaths::systemDir() {
    QString gamsPath;
    const QString subPath = QString(QDir::separator()).append("..");
#if __APPLE__
    gamsPath = "/Applications/GAMS" GAMS_DISTRIB_VERSION_SHORT "/sysdir";
    if (!QDir(gamsPath).exists())
        gamsPath = "/Applications/GAMS" GAMS_DISTRIB_VERSION_NEXT_SHORT "/sysdir";
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

    return QDir::cleanPath(gamsPath);
}

QString CommonPaths::userDocumentsDir()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (dir.isEmpty())
        FATAL() << "Unable to access user documents location";
    QDir userDocumentsDir = QDir::cleanPath(dir + "/GAMSStudio");
    if(!userDocumentsDir.exists())
        userDocumentsDir.mkpath(".");
    return userDocumentsDir.path();
}

QString CommonPaths::userModelLibraryDir()
{
    QDir userModelLibraryDir(userDocumentsDir() + "/modellibs");
    if(!userModelLibraryDir.exists())
        userModelLibraryDir.mkpath(".");
    return userModelLibraryDir.path();
}

QString CommonPaths::defaultWorkingDir()
{
    QDir defWorkingDir(userDocumentsDir() + "/workspace");
    if(!defWorkingDir.exists())
        defWorkingDir.mkpath(".");
    return defWorkingDir.path();
}

QString CommonPaths::absolutFilePath(const QString &filePath)
{
    QFileInfo fi(filePath);
    return fi.absoluteFilePath();
}

QString CommonPaths::absolutPath(const QString &dir)
{
    if (dir.isEmpty())
        return "";
    QDir d(dir);
    return d.absolutePath();
}

}
}
