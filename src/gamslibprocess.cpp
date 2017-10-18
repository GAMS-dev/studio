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
#include "gamslibprocess.h"

#include <QDebug>
#include <QDir>

namespace gams {
namespace studio {

GAMSLibProcess::GAMSLibProcess(QObject *parent)
    : AbstractProcess(parent)
{

}

void GAMSLibProcess::setApp(const QString &app)
{
    mApp = app;
}

QString GAMSLibProcess::app()
{
    return mApp;
}

QString GAMSLibProcess::nativeAppPath()
{
    return AbstractProcess::nativeAppPath(mSystemDir, mApp);
}

void GAMSLibProcess::setTargetDir(const QString &targetDir)
{
    mTargetDir = targetDir;
}

QString GAMSLibProcess::targetDir() const
{
    return mTargetDir;
}

void GAMSLibProcess::setModelNumber(int modelNumber)
{
    mModelNumber = modelNumber;
}

int GAMSLibProcess::modelNumber() const
{
    return mModelNumber;
}

void GAMSLibProcess::setModelName(const QString &modelName)
{
    mModelName = modelName;
}

QString GAMSLibProcess::modelName() const
{
    return mModelName;
}

void GAMSLibProcess::execute()
{// TODO(AF) improve the gamslib output messages... currently they don't make sense for the GAMS Studio!
    QStringList args;
    args << (mModelName.isEmpty() ? QString::number(mModelNumber) : mModelName);
    args << QDir::toNativeSeparators(mTargetDir);
    mProcess.start(nativeAppPath(), args);
}

} // namespace studio
} // namespace gams
