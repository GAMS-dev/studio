#include "tool.h"
#include "logger.h"
#include "gclgms.h"
#include "exception.h"
#include "gamspaths.h"
#include "checkforupdatewrapper.h"

#include <cstring>

namespace gams {
namespace studio {

int Tool::findAlphaNum(const QString &text, int start, bool back)
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

QString Tool::absolutePath(const QString &path)
{
    QFileInfo fi(path);
    return fi.exists() ? fi.canonicalFilePath() : fi.absoluteFilePath();
}

} // namespace studio
} // namespace gams
