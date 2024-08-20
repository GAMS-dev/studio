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
#include "gamsgetkeyprocess.h"
#include "gamsgetkeyprocess.h"
#include "commonpaths.h"

#include <QDir>

namespace gams {
namespace studio {

GamsGetKeyProcess::GamsGetKeyProcess()
    : mApplication("gamsgetkey")
{

}

QString GamsGetKeyProcess::execute()
{
    QStringList args({mAlpId});
    auto appPath = nativeAppPath();
    if (appPath.isEmpty())
        return QString();

#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(nativeAppPath(), args);
#else
    mProcess.setNativeArguments(args.join(" "));
    mProcess.setProgram(nativeAppPath());
    mProcess.start();
#endif

    QString content;
    if (mProcess.waitForFinished()) {
        content = mProcess.readAllStandardOutput();
        if (content.isEmpty())
            content = mProcess.readAllStandardError();
    }
    return content;
}

QString GamsGetKeyProcess::alpId() const
{
    return mAlpId;
}

void GamsGetKeyProcess::setAlpId(const QString &id)
{
    mAlpId = id;
}

QString GamsGetKeyProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(QDir::toNativeSeparators(mApplication));
    return QDir::toNativeSeparators(appPath);
}

}
}
