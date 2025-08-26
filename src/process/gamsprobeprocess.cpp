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
#include "gamsprobeprocess.h"
#include "commonpaths.h"

#include <QDir>

namespace gams {
namespace studio {

GamsprobeProcess::GamsprobeProcess()
    : mApplication("gamsprobe")
{

}

bool GamsprobeProcess::verboseOutput() const
{
    return mVerboseOutput;
}

void GamsprobeProcess::setVerboseOutput(bool enable)
{
    mVerboseOutput = enable;
}

QString GamsprobeProcess::execute()
{
    auto app = nativeAppPath();
    if (app.isEmpty()) {
        mErrorMessage = "Could not locate gamsprobe.";
        return QString();
    }
    QStringList args;
    if (mVerboseOutput) {
        args << "-v";
    }

#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(app, args);
#else
    mProcess.setNativeArguments(args.join(" "));
    mProcess.setProgram(app);
    mProcess.start();
#endif

    QString content;
    if (mProcess.waitForFinished()) {
        content = mProcess.readAllStandardOutput();
        if (content.isEmpty())
            mErrorMessage = mProcess.readAllStandardError();
    }
    return content;
}

QString GamsprobeProcess::errorMessage() const
{
    return mErrorMessage;
}

QString GamsprobeProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(QDir::toNativeSeparators(mApplication));
    return QDir::toNativeSeparators(appPath);
}

} // namespace studio
} // namespace gams
