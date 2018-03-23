/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "gclgms.h"
#include "gamspaths.h"
#include "tool.h"

#include <cstring>
#include <QStringList>

namespace gams {
namespace studio {

CheckForUpdateWrapper::CheckForUpdateWrapper()
{
    char buffer[GMS_SSSIZE];
    if (!c4uCreateD(&mC4UHandle, GAMSPaths::systemDir().toLatin1(), buffer, GMS_SSSIZE)) {
        mMessages << "Could not load c4u library: " << buffer;
        mValid = false;
    }
    if (isValid() && !c4uCorrectLibraryVersion(buffer, GMS_SSSIZE)) {
        mMessages << "Incompatible GAMS versions: " << buffer;
        mValid = false;
    }
}

CheckForUpdateWrapper::~CheckForUpdateWrapper()
{
    c4uFree(&mC4UHandle);
}

bool CheckForUpdateWrapper::isValid() const
{
    return mValid;
}

QString CheckForUpdateWrapper::message() const
{
    return mMessages.join("\n");
}

QString CheckForUpdateWrapper::checkForUpdate()
{
    char buffer[GMS_SSSIZE];
    c4uReadLice(mC4UHandle, GAMSPaths::systemDir().toLatin1(), "gamslice.txt", false);
    if (!c4uIsValid(mC4UHandle)) {
        c4uCreateMsg(mC4UHandle);
    }

    int messageIndex=0;
    mMessages << "GAMS Distribution";
    getMessages(messageIndex, buffer);

    mMessages << "\nGAMS Studio";
    c4uCheck4NewStudio(mC4UHandle, gams::studio::Version::versionToNumber());
    getMessages(messageIndex, buffer);

    return message();
}

char* CheckForUpdateWrapper::distribVersion(char *version, size_t length)
{
    char buffer[GMS_SSSIZE];
    c4uThisRelStr(mC4UHandle, buffer);
    std::strncpy(version, buffer, GMS_SSSIZE<length ? GMS_SSSIZE : length);
    return version;
}

int CheckForUpdateWrapper::currentDistribVersion()
{
    if (isValid())
        return c4uThisRel(mC4UHandle);
    return -1;
}

int CheckForUpdateWrapper::lastDistribVersion()
{
    if (c4uCheck4Update(mC4UHandle))
        return c4uLastRel(mC4UHandle);
    return -1;
}

bool CheckForUpdateWrapper::distribIsLatest()
{
    int lastDistrib = lastDistribVersion();
    if (currentDistribVersion() < 0 || lastDistrib < 0)
        return false;
    return currentDistribVersion() == lastDistrib;
}

void CheckForUpdateWrapper::getMessages(int &messageIndex, char *buffer)
{
    for (int c=c4uMsgCount(mC4UHandle); messageIndex<c; ++messageIndex) {
        if (c4uGetMsg(mC4UHandle, messageIndex, buffer))
            mMessages.append(buffer);
    }
}

}
}
