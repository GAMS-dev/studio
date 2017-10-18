#include "gamsprocess.h"
#include "gamsinfo.h"
#include "filegroupcontext.h"

#include <QDebug>
#include <QDir>

namespace gams {
namespace studio {

const QString GAMSProcess::App = "gams";

GAMSProcess::GAMSProcess(QObject *parent)
    : AbstractProcess(parent)
{
}

QString GAMSProcess::app()
{
    return App;
}

QString GAMSProcess::nativeAppPath()
{
    return AbstractProcess::nativeAppPath(mSystemDir, App);
}

void GAMSProcess::setWorkingDir(const QString &workingDir)
{
    mWorkingDir = workingDir;
}

QString GAMSProcess::workingDir() const
{
    return mWorkingDir;
}

void GAMSProcess::setContext(FileGroupContext *context)
{
    mContext = context;
}

FileGroupContext* GAMSProcess::context() const
{
    return mContext;
}

void GAMSProcess::execute()
{
    qDebug() << "GAMSProcess::execute()";
    mProcess.setWorkingDirectory(mWorkingDir);
    auto gms = QDir::toNativeSeparators(mInputFile);
    //TODO(CW)
    // we need this at least on wnidows in order to write explicitly to stdout.
    // As soon as we allow user input for options, this needs to be adjusted
    QStringList args({gms, "lo=3"});
    mProcess.start(nativeAppPath(), args);
}

QString GAMSProcess::aboutGAMS()
{
    QProcess process;
    QStringList args({"?", "lo=3"});
    process.start(AbstractProcess::nativeAppPath(GAMSInfo::systemDir(), App), args);
    QString about;
    if (process.waitForFinished()) {
        about = process.readAllStandardOutput();
    }
    return about;
}

} // namespace studio
} // namespace gams
