/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QMessageBox>

namespace gams {
namespace studio {

GAMSPaths::GAMSPaths()
{

}

QString GAMSPaths::systemDir() {
#if defined(DISTRIB_BUILD) // For the GAMS distribution build
#ifdef __linux__ // Linux AppImage
    return QDir::currentPath().append("/..");
#elif __APPLE__ // Apple MacOS dmg
    auto path = QDir::currentPath();
    QMessageBox::information(nullptr, "Path", path);
    return path;
#else // Windows
    return QDir::currentPath().append("/..");
#endif
#else // Just a simple way for developers to find a GAMS distribution... if the PATH is set.
    QString ret = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
        if (ret == "") {
            EXCEPT() << "GAMS not found in path.";
        }
    return QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
#endif
}

QString GAMSPaths::defaultWorkingDir()
{
    const QString currentDir = ".";
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (dir.isEmpty())
        return currentDir;
    QDir workingDir = QDir::cleanPath(dir + "/GAMSStudio");
    if (workingDir.mkpath(workingDir.path()))
        return QDir::toNativeSeparators(workingDir.path());
    return currentDir;
}

}
}
