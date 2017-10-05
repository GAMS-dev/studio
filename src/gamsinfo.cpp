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
#include "gamsinfo.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QMessageBox>

namespace gams {
namespace studio {

GAMSInfo::GAMSInfo()
{

}

QString GAMSInfo::systemDir() {
#ifdef __linux__ // Linux AppImage
    return QDir::currentPath().append("/..");
#elif __APPLE__ // Apple MacOS dmg
    auto path = QDir::currentPath().append("/../../..");
    QMessageBox::information(nullptr, "Path", path);
    return path;
#else // Windows
    return QDir::currentPath().append("/..");
#endif
    //return "";//QStandardPaths::findExecutable("gams");
}

}
}
