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
#include "gamsprocess.h"
#include "gamspaths.h"
#include "filegroupcontext.h"

#include <QDebug>
#include <QDir>

namespace gams {
namespace studio {

const QString GAMSProcess::App = "gams";

GAMSProcess::GAMSProcess(QObject *parent)
    : AbstractProcess(parent)
{
}

QString GAMSProcess::app()
{
    return App;
}

QString GAMSProcess::nativeAppPath()
{
    return AbstractProcess::nativeAppPath(mSystemDir, App);
}

void GAMSProcess::setWorkingDir(const QString &workingDir)
{
    mWorkingDir = workingDir;
}

QString GAMSProcess::workingDir() const
{
    return mWorkingDir;
}

void GAMSProcess::setContext(FileGroupContext *context)
{
    mContext = context;
}

FileGroupContext* GAMSProcess::context() const
{
    return mContext;
}

void GAMSProcess::execute()
{
    qDebug() << "GAMSProcess::execute()";
    mProcess.setWorkingDirectory(mWorkingDir);
    QString gms = QDir::toNativeSeparators(mInputFile);
    gms.replace(" ", "\\ ");

    //TODO(CW)
    // we need this at least on windows in order to write explicitly to stdout.
    // As soon as we allow user input for options, this needs to be adjusted
    QStringList args({gms, "lo=3", "ide=1", "er=99"});
    mProcess.start(nativeAppPath(), args);
}

QString GAMSProcess::aboutGAMS()
{
    QProcess process;
    QStringList args({"?", "lo=3"});
    process.start(AbstractProcess::nativeAppPath(GAMSPaths::systemDir(), App), args);
    QString about;
    if (process.waitForFinished()) {
        about = process.readAllStandardOutput();
    }
    return about;
}

} // namespace studio
} // namespace gams
