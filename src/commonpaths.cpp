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
#include "commonpaths.h"
#include "exception.h"

#ifdef __APPLE__
#include "../platform/macos/macospathfinder.h"
#endif

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace gams {
namespace studio {

#if defined(__APPLE__)
    const QString CommonPaths::DocumentationDir = "../../../Resources/docs";
#else
    const QString CommonPaths::DocumentationDir = "docs";
#endif

QString CommonPaths::SystemDir = QString();

#if defined(__APPLE__) || defined(__unix__)
    const QString CommonPaths::ConfigFile = "gmscmpun.txt";
#else
    const QString CommonPaths::ConfigFile = "gmscmpnt.txt";
#endif

const QString CommonPaths::LicenseFile = "gamslice.txt";

CommonPaths::CommonPaths()
{

}

const QString& CommonPaths::documentationDir()
{
    return DocumentationDir;
}

QString CommonPaths::helpDocumentsDir()
{
#ifdef __APPLE__
//    [sysdir]/GAMS Terminal.app/Contents/MacOS
//    [sysdir]/Resources/docs
    QDir helpDocDir(systemDir());
    QString dochelp = helpDocDir.cleanPath(helpDocDir.absoluteFilePath(DocumentationDir));
    return dochelp;
#else
    QDir helpDocDir(systemDir() + "/" + DocumentationDir);
    return helpDocDir.path();
#endif
}

const QString& CommonPaths::systemDir()
{
    return SystemDir;
}

void CommonPaths::setSystemDir(const QString &sysdir)
{
    if (!sysdir.isEmpty()) {
        auto path = QFileInfo(QStandardPaths::findExecutable("gams", {sysdir})).absolutePath();
        SystemDir = QDir::cleanPath(path);
        return;
    }

#ifdef __APPLE__
    SystemDir = MacOSPathFinder::systemDir();
#else
    QString gamsPath;
    const QString subPath = QString(QDir::separator()).append("..");
#ifdef __unix__
    QFileInfo fileInfo(qgetenv("APPIMAGE"));
    gamsPath = fileInfo.absoluteDir().path().append(subPath);
#else
    gamsPath = QCoreApplication::applicationDirPath().append(subPath);
#endif

    QString path = QStandardPaths::findExecutable("gams", {gamsPath});
    if (path.isEmpty()) {
        gamsPath = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    }

    SystemDir = QDir::cleanPath(gamsPath);
#endif
}

bool CommonPaths::isSystemDirValid()
{
    if (QStandardPaths::findExecutable("gams", {SystemDir}).isEmpty())
        return false;
    return true;
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

QString CommonPaths::canonicalPath(const QString &dir)
{
    if (dir.isEmpty())
        return "";
    QDir d(dir);
    return d.canonicalPath();
}

QString CommonPaths::configFile()
{
    QDir configFile(systemDir() + "/" + ConfigFile);
    return configFile.path();
}

QString CommonPaths::licenseFile()
{
    QDir licenseFile(systemDir() + "/" + LicenseFile);
    return licenseFile.path();
}

QString CommonPaths::changelog()
{
#ifdef __APPLE__
    return QDir::cleanPath(MacOSPathFinder::bundlePath().append("/Contents/Resources/Changelog"));
#elif __unix__
    return QDir::cleanPath(QCoreApplication::applicationDirPath().append("/../resources/Changelog"));
#else
    return QDir::cleanPath(QCoreApplication::applicationDirPath().append("/resources/Changelog"));
#endif
}

}
}
