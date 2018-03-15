#include "version.h"
#include "gclgms.h"
#include "c4umcc.h"
//#include "exception.h"
#include "gamspaths.h"

#include <cstring>

namespace gams {
namespace studio {

int versionToNumber()
{
    return QString(STUDIO_VERSION).replace('.', "", Qt::CaseInsensitive).toInt();
}

char* currentGAMSDistribVersion(char *version, int length)
{
    c4uHandle_t c4uHandle;
    char buffer[GMS_SSSIZE];
    if (!c4uCreateD(&c4uHandle, GAMSPaths::systemDir().toLatin1(), buffer, GMS_SSSIZE)) {
        //EXCEPT() << "Could not load c4u library: " << buffer;
    }
    c4uThisRelStr(c4uHandle, buffer);
    std::strncpy(version, buffer, GMS_SSSIZE<length ? GMS_SSSIZE : length);
    c4uFree(&c4uHandle);
    return version;
}

}
}
