#include "gamsprocess.h"
#include "gamsinfo.h"

namespace gams {
namespace studio {

const QString GAMSProcess::App = "gams";

GAMSProcess::GAMSProcess()
    : mSystemDir(GAMSInfo::systemDir())
{

}

QString GAMSProcess::app()
{
    return App;
}

QString GAMSProcess::nativeAppPath()
{
    return GAMSProcess::nativeAppPath(mSystemDir, App);
}

QString GAMSProcess::nativeAppPath(const QString &dir, const QString &app)
{
    auto appPath = QDir(dir).filePath(app);
    return QDir::toNativeSeparators(appPath);
}

void GAMSProcess::setSystemDir(const QString &systemDir)
{
    mSystemDir = systemDir;
}

QString GAMSProcess::systemDir() const
{
    return mSystemDir;
}

void GAMSProcess::setWorkingDir(const QString &workingDir)
{
    mWorkingDir = workingDir;
}

QString GAMSProcess::workingDir() const
{
    return mWorkingDir;
}

void GAMSProcess::run()
{
    mProcess.setWorkingDirectory(mWorkingDir);
    QStringList args("lo=3");
    mProcess.start(nativeAppPath(), args);
}

QString GAMSProcess::aboutGAMS()
{
    QProcess process;
    QStringList args({"?", "lo=3"});
    process.start(nativeAppPath(GAMSInfo::systemDir(), App), args);
    QString about;
    if (process.waitForFinished()) {
        about = process.readAllStandardOutput();
    }
    return about;
}

} // namespace studio
} // namespace gams
