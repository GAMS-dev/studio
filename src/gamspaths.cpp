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
    // TODO(AF) macOS stuff
    QStringList paths = { QDir::currentPath().append("/..") };
    QString path = QFileInfo(QStandardPaths::findExecutable("gams", paths)).absolutePath();
    if (!path.isEmpty())
        return path;

    path = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    if (path.isEmpty()) EXCEPT() << "GAMS not found in path.";

    // TODO(AF) this does not work in all cases, e.g. compressed gams archive, custom installation, distrib build => use GAMS API call
    int bitness = sizeof(int*);
    if (bitness == 4 && path.contains("win64"))
        FATAL() << "GAMS Studio (32bit) can't be executed with " << path;
    if (bitness == 8 && path.contains("win32"))
        FATAL() << "GAMS Studio (64bit) can't be executed with " << path;
    return path;
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
