/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "mirocommon.h"

#include <QDir>
#include <QFileInfo>
#include <QTextStream>

namespace gams {
namespace studio {
namespace miro {

QString MiroCommon::path(const QString &configMiroPath)
{
    if (!configMiroPath.isEmpty()) {
        if (configMiroPath.endsWith("GAMS MIRO.app") &&
            exists(configMiroPath + "/Contents/MacOS/GAMS MIRO")) {
            return configMiroPath + "/Contents/MacOS/GAMS MIRO";
        } else if (exists(configMiroPath)) {
            return configMiroPath;
        }
    }
    auto locations = standardLocations();
    return searchLocations(locations);
}

QString MiroCommon::assemblyFileName(const QString &modelName)
{
    return modelName + "_files.txt";
}

QString MiroCommon::assemblyFileName(const QString &modelLocation, const QString &modelName)
{
    return QFileInfo(modelLocation, miro::MiroCommon::assemblyFileName(modelName)).canonicalFilePath();
}

QStringList MiroCommon::unifiedAssemblyFileContent(const QString &assemblyFile, const QString &mainFile)
{
    QStringList directories;
    QStringList selectedFiles;
    QFileInfo fileInfo(assemblyFile);
    if (fileInfo.exists(assemblyFile)) {
        QFile file(assemblyFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while (!file.atEnd()) {
                auto line = file.readLine().trimmed();
                if (line.isEmpty())
                    continue;
                auto filePath = QDir(fileInfo.path()).absoluteFilePath(line);
                if (QFileInfo(filePath).isDir())
                    directories << line;
                selectedFiles << line;
            }
            file.close();
        }
    } else {
        selectedFiles << mainFile;
    }
    return removeReduntantFiles(directories, selectedFiles);
}

bool MiroCommon::writeAssemblyFile(const QString &assemblyFile, const QStringList &selectedFiles)
{
    QFile file(assemblyFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        for (auto selectedFile: selectedFiles)
            stream << selectedFile << "\n";
        file.close();
        return true;
    }
    return false;
}

bool MiroCommon::exists(const QString &miro)
{
    QFileInfo fileInfo(miro);
    return fileInfo.exists();
}

QString MiroCommon::searchLocations(const QStringList &locations)
{
    for (auto location: locations) {
        if (exists(location))
            return location;
    }
    return QString();
}

QStringList MiroCommon::standardLocations()
{
#if defined (__APPLE__)
    return { "/Applications/GAMS MIRO.app/Contents/MacOS/GAMS MIRO",
             "~/Applications/GAMS MIRO.app/Contents/MacOS/GAMS MIRO" };
#elif defined (__unix__)
    return QStringList();
#else
    return { R"(C:\Program Files\GAMS MIRO\GAMS MIRO.exe)",
             QDir::homePath() + R"(\AppData\Local\Programs\GAMS MIRO\GAMS MIRO.exe)" };
#endif
}

QStringList MiroCommon::removeReduntantFiles(const QStringList dirs, const QStringList &files)
{
    auto validFiles = files;
    for (auto dir: dirs) {
        for (auto file: files) {
            if (file.startsWith(dir))
                validFiles.removeAll(file);
        }
    }
    return validFiles << dirs;
}

}
}
}
