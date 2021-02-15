/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
 */
#include "gamsinstprocess.h"

namespace gams {
namespace studio {
namespace process {

GamsInstProcess::GamsInstProcess(QObject *parent)
    : AbstractGamsProcess("gamsinst", parent)
{
    connect(this, &GamsInstProcess::newStdChannelData, this, &GamsInstProcess::newData);
}

void GamsInstProcess::execute()
{
    auto params = defaultParameters() + parameters();
#if defined(__unix__) || defined(__APPLE__)
    emit newProcessCall("Running:", appCall(nativeAppPath(), params));
    mProcess.start(nativeAppPath(), params);
#else
    mProcess.setNativeArguments(params.join(" "));
    mProcess.setProgram(nativeAppPath());
    emit newProcessCall("Running:", appCall(nativeAppPath(), params));
    mProcess.start();
#endif
}

void GamsInstProcess::newData(const QByteArray &data)
{
    if (data.startsWith("Config")) isData = false;
    else if (data.startsWith("Data")) isData = true;
    else {
        QString line = data.trimmed();
        if (line.isEmpty()) return;
        if (isData) mData << line;
        else mConfig << line;
    }
}

} // namespace process
} // namespace studio
} // namespace gams
