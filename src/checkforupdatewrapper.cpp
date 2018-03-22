#include "checkforupdatewrapper.h"
#include "gclgms.h"
#include "gamspaths.h"
#include "tool.h"

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

void CheckForUpdateWrapper::getMessages(int &messageIndex, char *buffer)
{
    for (int c=c4uMsgCount(mC4UHandle); messageIndex<c; ++messageIndex) {
        if (c4uGetMsg(mC4UHandle, messageIndex, buffer))
            mMessages.append(buffer);
    }
}

}
}
