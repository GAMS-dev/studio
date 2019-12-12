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
#include "gamslibprocess.h"

#include <QDir>

namespace gams {
namespace studio {

GamsLibProcess::GamsLibProcess(QObject *parent)
    : AbstractGamsProcess("gamslib", parent)
{

}

void GamsLibProcess::setModelNumber(int modelNumber)
{
    mModelNumber = modelNumber;
}

int GamsLibProcess::modelNumber() const
{
    return mModelNumber;
}

void GamsLibProcess::setModelName(const QString &modelName)
{
    mModelName = modelName;
}

QString GamsLibProcess::modelName() const
{
    return mModelName;
}

void GamsLibProcess::execute()
{
    QStringList args;
    args << "-lib";
    args << QDir::toNativeSeparators(mGlbFile);
    args << (mModelName.isEmpty() ? QString::number(mModelNumber) : mModelName);
    args << QDir::toNativeSeparators(workingDirectory());
    mProcess.start(nativeAppPath(), args);
}

void GamsLibProcess::setGlbFile(const QString &glbFile)
{
    mGlbFile = glbFile;
}

} // namespace studio
} // namespace gams
