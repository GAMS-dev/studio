/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "macospathfinder.h"

#include <QDir>
#include <QStandardPaths>

QString MacOSPathFinder::systemDir()
{
    auto sysdir = systemDir(false);
    if (!QStandardPaths::findExecutable("gams", {sysdir}).isEmpty())
        return QDir::cleanPath(sysdir);
    sysdir = systemDir(true);
    if (!QStandardPaths::findExecutable("gams", {sysdir}).isEmpty())
        return QDir::cleanPath(sysdir);
    sysdir = QFileInfo(QStandardPaths::findExecutable("gams")).canonicalPath();
    if (!sysdir.isEmpty())
        return QDir::cleanPath(sysdir);
    return QString();
}

QString MacOSPathFinder::systemDir(bool current)
{
    QString path = "/Library/Frameworks/GAMS.framework/Versions/%1/Resources";
    return path.arg(current ? "Current" : QString::number(GAMS_DISTRIB_MAJOR));
}
