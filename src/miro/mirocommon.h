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
#ifndef MIROCOMMON_H
#define MIROCOMMON_H

#include <QString>

namespace gams {
namespace studio {
namespace miro {

enum class MiroDeployMode
{
    None      = 0,
    Base      = 1,
    Hypercube = 2
};

class MiroCommon
{
private:
    MiroCommon() {}

public:
    static QString path(const QString &configMiroPath);

    static QString assemblyFileName(const QString &modelName);
    static QString assemblyFileName(const QString &modelLocation, const QString &modelName);
    static QString deployFileName(const QString &modelName);

    static QStringList unifiedAssemblyFileContent(const QString &assemblyFile, const QString &mainFile);
    static bool writeAssemblyFile(const QString &assemblyFile, const QStringList &selectedFiles);

private:
    static bool exists(const QString &miro);
    static QString searchLocations(const QStringList &locations);
    static QStringList standardLocations();
    static QStringList removeReduntantFiles(const QStringList dirs, const QStringList &files);
};

}
}
}

#endif // MIROCOMMON_H
