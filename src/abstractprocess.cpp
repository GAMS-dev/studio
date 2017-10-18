#include "abstractprocess.h"
#include "gamsinfo.h"

#include <QDir>

namespace gams {
namespace studio {

AbstractProcess::AbstractProcess(QObject *parent)
    : QObject (parent),
      mSystemDir(GAMSInfo::systemDir()),
      mProcess(this)
{
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &AbstractProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError, this, &AbstractProcess::readStdErr);
    connect(&mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(completed(int)));
}

QString AbstractProcess::nativeAppPath(const QString &dir, const QString &app)
{
    auto appPath = QDir(dir).filePath(app);
    return QDir::toNativeSeparators(appPath);
}

void AbstractProcess::setSystemDir(const QString &systemDir)
{
    mSystemDir = systemDir;
}

QString AbstractProcess::systemDir() const
{
    return mSystemDir;
}

void AbstractProcess::setInputFile(const QString &file)
{
    mInputFile = file;
}

QString AbstractProcess::inputFile() const
{
    return mInputFile;
}

void AbstractProcess::completed(int exitCode)
{
    emit finished(exitCode);
}

void AbstractProcess::readStdOut()
{
    readStdChannel(QProcess::StandardOutput);
}

void AbstractProcess::readStdErr()
{
    readStdChannel(QProcess::StandardError);
}

void AbstractProcess::readStdChannel(QProcess::ProcessChannel channel)
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
