#include "miropaths.h"

#include <QFileInfo>

namespace gams {
namespace studio {

MiroPaths::MiroPaths(const QString &configMiroPath)
    : mConfiguredMiroPath(configMiroPath)
{

}

QString MiroPaths::error() const
{
    return mError;
}

QString MiroPaths::path()
{
    if (!mConfiguredMiroPath.isEmpty()) {
        if (mConfiguredMiroPath.endsWith("GAMS MIRO.app") &&
            exists(mConfiguredMiroPath + "/Contents/MacOS/GAMS MIRO")) {
            return mConfiguredMiroPath + "/Contents/MacOS/GAMS MIRO";
        } else if (exists(mConfiguredMiroPath)) {
            return mConfiguredMiroPath;
        }
    }
    auto locations = standardLocations();
    auto path = searchLocations(locations);
    return verifyPath(path, locations);
}

bool MiroPaths::exists(const QString &miro)
{
    QFileInfo fileInfo(miro);
    return fileInfo.exists();
}

QString MiroPaths::searchLocations(const QStringList &locations)
{
    for (auto location: locations) {
        if (exists(location))
            return location;
    }
    return QString();
}

QStringList MiroPaths::standardLocations()
{
#if defined (__APPLE__)
    return { "/Applications/GAMS MIRO.app/Contents/MacOS/GAMS MIRO",
             "~/Applications/GAMS MIRO.app/Contents/MacOS/GAMS MIRO" };
#elif defined (__unix__)
    return QStringList();
#else
    // TODO (AF) make USER dynamic QDir::home()
    return { R"(C:\Program Files\GAMS MIRO\GAMS MIRO.exe)",
             R"(C:\Users\distrib\AppData\Local\Programs\GAMS MIRO\GAMS MIRO.exe)" };
#endif
}

QString MiroPaths::verifyPath(const QString &path, const QStringList &searched)
{
    if (path.isEmpty()) {
        mError = "MIRO could not be found in " + searched.join(", ");
        return QString();
    }
    return path;
}

}
}
