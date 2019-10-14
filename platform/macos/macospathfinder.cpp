#include "macospathfinder.h"

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>

#include <QDir>
#include <QStandardPaths>

const QString MacOSPathFinder::Sysdir = "/GAMS Terminal.app/Contents/MacOS";
const QString MacOSPathFinder::SubPath = "/.." + Sysdir;

MacOSPathFinder::MacOSPathFinder()
{

}

QString MacOSPathFinder::systemDir()
{
    auto path = bundlePath() + SubPath;
    if (QStandardPaths::findExecutable("gams", {path}).isEmpty()) {
        path = searchApplications();
        if (QStandardPaths::findExecutable("gams", {path}).isEmpty())
            path = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    }
    return QDir::cleanPath(path);
}

QString MacOSPathFinder::bundlePath()
{
    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef,
                                                  kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath,
                                                CFStringGetSystemEncoding());
    CFRelease(appUrlRef);
    CFRelease(macPath);
    return QString(pathPtr);
}

QString MacOSPathFinder::searchApplications()
{
    QString path = "/Applications/GAMS" GAMS_DISTRIB_VERSION_SHORT + Sysdir;
    if (!QDir(path).exists()) {
        QDir applications("/Applications");
        QRegExp regex("^GAMS(\\d\\d).(\\d)$");
        for (auto dir : applications.entryList({"GAMS*"}, QDir::Dirs)) {
           if (!regex.exactMatch(dir))
               continue;
           if (regex.cap(1).toInt() > GAMS_DISTRIB_MAJOR) {
               path = "/Applications/" + dir + Sysdir;
               break;
           }
           if (regex.cap(1).toInt() == GAMS_DISTRIB_MAJOR &&
               regex.cap(2).toInt() >= GAMS_DISTRIB_MINOR) {
               path = "/Applications/" + dir + Sysdir;
               break;
           }
        }
    }
    return path;
}
