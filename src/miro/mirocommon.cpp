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
#include "mirocommon.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QTextStream>

namespace gams {
namespace studio {
namespace miro {

const QString MiroCommon::ConfFolderPrefix = "conf_";
const QString MiroCommon::DataFolderPrefix = "data_";
const QString MiroCommon::DataContractPostfix = "_io.json";

QString MiroCommon::path(const QString &configMiroPath)
{
    if (!configMiroPath.isEmpty()) {
        if (configMiroPath.endsWith("GAMS MIRO.app") &&
            exists(configMiroPath + MIRO_MACOS_APP_BUNDLE_POSTFIX)) {
            return configMiroPath + MIRO_MACOS_APP_BUNDLE_POSTFIX;
        } else if (exists(configMiroPath)) {
            return configMiroPath;
        }
    }
    auto locations = standardLocations();
    return searchLocations(locations);
}

QString MiroCommon::confDirectory(const QString &modelName)
{
    return ConfFolderPrefix + modelName.toLower();
}

QString MiroCommon::dataDirectory(const QString &modelName)
{
    return DataFolderPrefix + modelName.toLower();
}

QString MiroCommon::dataContractFileName(const QString &modelName)
{
    return modelName + DataContractPostfix;
}

QString MiroCommon::assemblyFileName(const QString &modelName)
{
    return modelName.toLower() + "_files.txt";
}

QString MiroCommon::assemblyFileName(const QString &modelLocation,
                                     const QString &modelName)
{
    return QDir(modelLocation).filePath(miro::MiroCommon::assemblyFileName(modelName));
}

QString MiroCommon::deployFileName(const QString &modelName)
{
    if (modelName.isEmpty())
        return modelName;
    return modelName+".miroapp";
}

QStringList MiroCommon::unifiedAssemblyFileContent(const QString &assemblyFile,
                                                   const QString &mainFile)
{
    QSet<QString> selectedFiles;
    QFileInfo fileInfo(assemblyFile);
    if (fileInfo.exists(assemblyFile)) {
        QFile file(assemblyFile);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            while (!stream.atEnd()) {
                auto line = stream.readLine().trimmed();
                if (line.isEmpty())
                    continue;
                selectedFiles << line;
            }
            file.close();
        }
    } else {
        if (!mainFile.isEmpty())
            selectedFiles << mainFile;
    }
    return selectedFiles.values();
}

bool MiroCommon::writeAssemblyFile(const QString &assemblyFile,
                                   const QStringList &selectedFiles)
{
    if (assemblyFile.isEmpty())
        return false;
    QFile file(assemblyFile);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream stream(&file);
        for (const auto& selectedFile: selectedFiles)
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

}
}
}
