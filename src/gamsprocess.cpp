#include "gamsprocess.h"
#include "gamsinfo.h"
#include "filegroupcontext.h"

#include <QDebug>
#include <QDir>

namespace gams {
namespace studio {

const QString GAMSProcess::App = "gams";

GAMSProcess::GAMSProcess(QObject *parent)
    : QObject(parent),
      mSystemDir(GAMSInfo::systemDir())
{
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &GAMSProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError, this, &GAMSProcess::readStdErr);
    connect(&mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(completed(int)));
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

void GAMSProcess::setInputFile(const QString &file)
{
    mInputFile = file;
}

QString GAMSProcess::inputFile() const
{
    return mInputFile;
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
    process.start(nativeAppPath(GAMSInfo::systemDir(), App), args);
    QString about;
    if (process.waitForFinished()) {
        about = process.readAllStandardOutput();
    }
    return about;
}

void GAMSProcess::completed(int exitCode)
{
    emit finished(exitCode);
}

void GAMSProcess::readStdOut()
{
    readStdChannel(QProcess::StandardOutput);
}

void GAMSProcess::readStdErr()
{
    readStdChannel(QProcess::StandardError);
}

void GAMSProcess::readStdChannel(QProcess::ProcessChannel channel)
{
    mOutputMutex.lock();
    mProcess.setReadChannel(channel);
    bool avail = mProcess.bytesAvailable();
    mOutputMutex.unlock();

    while (avail) {
        mOutputMutex.lock();
        mProcess.setReadChannel(channel);
        emit newStdChannelData(channel, mProcess.readLine());
        avail = mProcess.bytesAvailable();
        mOutputMutex.unlock();
    }
}

} // namespace studio
} // namespace gams
