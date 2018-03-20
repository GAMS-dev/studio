#include "tool.h"
#include "logger.h"
#include "gclgms.h"
#include "c4umcc.h"
#include "exception.h"
#include "gamspaths.h"

#include <cstring>

namespace gams {
namespace studio {

int Version::versionToNumber()
{
    return QString(STUDIO_VERSION).replace(".", "", Qt::CaseInsensitive).toInt();
}

char* Version::currentGAMSDistribVersion(char *version, int length)
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

int Tool::findAlphaNum(QString text, int start, bool back)
{
    QChar c = ' ';
    int pos = (back && start == text.length()) ? start-1 : start;
    while (pos >= 0 && pos < text.length()) {
        c = text.at(pos);
        if (!c.isLetterOrNumber() && c != '_' && (pos != start || !back)) break;
        pos = pos + (back?-1:1);
    }
    pos = pos - (back?-1:1);
    if (pos == start) {
        c = (pos >= 0 && pos < text.length()) ? text.at(pos) : ' ';
        if (!c.isLetterOrNumber() && c != '_') return -1;
    }
    if (pos >= 0 && pos < text.length()) { // must not start with number
        c = text.at(pos);
        if (!c.isLetter() && c != '_') return -1;
    }
    return pos;
}

QString Tool::absolutePath(QString path)
{
    QFileInfo fi(path);
    return fi.exists() ? fi.canonicalFilePath() : fi.absoluteFilePath();
}

} // namespace studio
} // namespace gams
