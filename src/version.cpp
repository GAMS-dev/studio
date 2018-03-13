#include "version.h"
#include "gclgms.h"
#include "c4umcc.h"
#include "exception.h"
#include "gamspaths.h"

namespace gams {
namespace studio {

int versionToNumber()
{
    return QString(STUDIO_VERSION).replace('.', "", Qt::CaseInsensitive).toInt();
}

QString currentGAMSDistribVersion()
{// TODO(AF) use c4uThisRel with str return.
    c4uHandle_t c4uHandle;
    char buffer[GMS_SSSIZE];
    if (!c4uCreateD(&c4uHandle, GAMSPaths::systemDir().toLatin1(), buffer, GMS_SSSIZE)) {
        EXCEPT() << "Could not load c4u library: " << buffer;
    }
    QString distribVersion = QString::number(c4uThisRel(c4uHandle));
    c4uFree(&c4uHandle);
    return distribVersion;
}

}
}
