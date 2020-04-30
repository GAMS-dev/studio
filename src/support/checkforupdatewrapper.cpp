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
#include "checkforupdatewrapper.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"
#include "common.h"
#include "gclgms.h"
#include "c4umcc.h"

#include <cstring>

namespace gams {
namespace studio {
namespace support {

CheckForUpdateWrapper::CheckForUpdateWrapper()
{
    c4uSetExitIndicator(0); // switch of exit() call
    c4uSetScreenIndicator(0);
    c4uSetErrorCallback(CheckForUpdateWrapper::errorCallback);

    char buffer[GMS_SSSIZE];
    if (!c4uCreateD(&mC4U, CommonPaths::systemDir().toLatin1(), buffer, GMS_SSSIZE)) {
        mMessages << "Could not load c4u library: " << buffer;
        mValid = false;
    }
    if (isValid() && !c4uCorrectLibraryVersion(buffer, GMS_SSSIZE)) {
        mMessages << "Incompatible GAMS distribution: " << buffer;
        c4uFree(&mC4U);
        mValid = false;
    }
}

CheckForUpdateWrapper::~CheckForUpdateWrapper()
{
    if (isValid()) c4uFree(&mC4U);
}

bool CheckForUpdateWrapper::isValid() const
{
    return mValid;
}

QString CheckForUpdateWrapper::message() const
{
    return mMessages.join("\n");
}

void CheckForUpdateWrapper::clearMessages()
{
    mMessages.clear();
}

QString CheckForUpdateWrapper::checkForUpdate()
{
    if (!isValid())
        return QString();

    char buffer[GMS_SSSIZE];
    c4uReadLiceStd(mC4U, CommonPaths::systemDir().toStdString().c_str(), false);
    c4uCreateMsg(mC4U);

    int messageIndex=0;
    mMessages << "GAMS Distribution";
    getMessages(messageIndex, buffer);

    mMessages << "\nGAMS Studio";
    c4uCheck4NewStudio2(mC4U, STUDIO_MAJOR_VERSION, STUDIO_MINOR_VERSION, STUDIO_PATCH_LEVEL);
    getMessages(messageIndex, buffer);

    return message();
}

int CheckForUpdateWrapper::currentDistribVersion()
{
    if (isValid())
        return c4uThisRel(mC4U);
    return -1;
}

QString CheckForUpdateWrapper::currentDistribVersionShort()
{
    if (!isValid())
        return QString();
    char buffer[16];
    c4uThisRelStr(mC4U, buffer);
    QString version(buffer);
    int index = version.lastIndexOf('.');
    return version.remove(index, version.size());
}

int CheckForUpdateWrapper::lastDistribVersion()
{
    if (isValid() && c4uCheck4Update(mC4U))
        return c4uLastRel(mC4U);
    return -1;
}

QString CheckForUpdateWrapper::lastDistribVersionShort()
{
    if (!isValid())
        return QString();
    char buffer[16];
    c4uLastRelStr(mC4U, buffer);
    QString version(buffer);
    int index = version.lastIndexOf('.');
    return version.remove(index, version.size());
}

bool CheckForUpdateWrapper::distribIsLatest()
{
    int lastDistrib = lastDistribVersion();
    if (currentDistribVersion() < 0 || lastDistrib < 0)
        return false;
    return currentDistribVersion() == lastDistrib;
}

int CheckForUpdateWrapper::studioVersion()
{
    return QString(STUDIO_VERSION).replace(".", "", Qt::CaseInsensitive).toInt();
}

QString CheckForUpdateWrapper::distribVersionString()
{
    CheckForUpdateWrapper c4uWrapper;
    if (c4uWrapper.isValid()) {
        char version[16];
        return c4uWrapper.distribVersionString(version, 16);
    }
    return QString();
}

char* CheckForUpdateWrapper::distribVersionString(char *version, size_t length)
{
    char buffer[GMS_SSSIZE];
    c4uThisRelStr(mC4U, buffer);
    std::strncpy(version, buffer, GMS_SSSIZE<length ? GMS_SSSIZE : length);
    return version;
}

void CheckForUpdateWrapper::getMessages(int &messageIndex, char *buffer)
{
    for (int c=c4uMsgCount(mC4U); messageIndex<c; ++messageIndex) {
        if (c4uGetMsg(mC4U, messageIndex, buffer))
            mMessages.append(buffer);
    }
}

int CheckForUpdateWrapper::errorCallback(int count, const char *message)
{
    Q_UNUSED(count)
    auto logger = SysLogLocator::systemLog();
    logger->append(InvalidGAMS, LogMsgType::Error);
    logger->append(message, LogMsgType::Error);
    return 0;
}

}
}
}
