#include "abstractmiroprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"

#include <QDir>
#include <QDebug>

namespace gams {
namespace studio {

const QString AbstractMiroProcess::ConfFolderPrefix = "conf_";
const QString AbstractMiroProcess::DataFolderPrefix = "data_";

AbstractMiroProcess::AbstractMiroProcess(const QString &application, QObject *parent)
    : AbstractProcess(application, parent)
{
    // GAMS connections
    connect(&mProcess, &QProcess::stateChanged, this, &AbstractProcess::stateChanged);
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &AbstractMiroProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError, this, &AbstractMiroProcess::readStdErr);
    connect(&mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(subProcessCompleted(int)));

    // MIRO connections
    connect(this, &AbstractMiroProcess::executeMiro, this, &AbstractMiroProcess::executeNext);
    connect(&mMiro, &QProcess::stateChanged, this, &AbstractMiroProcess::stateChanged);
    connect(&mMiro, &QProcess::readyReadStandardOutput, this, &AbstractMiroProcess::readStdOut);
    connect(&mMiro, &QProcess::readyReadStandardError, this, &AbstractMiroProcess::readStdErr);
    connect(&mMiro, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(completed(int)));
}

void AbstractMiroProcess::execute()
{
    mProcess.setWorkingDirectory(workingDirectory());
#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(nativeAppPath(), parameters() + miroGamsParameters());
#else
    auto params = parameters() + miroGamsParameters();
    qDebug() << "#### PARAMS " << params;
    mProcess.setNativeArguments(params.join(" "));
    mProcess.setProgram(nativeAppPath());
    mProcess.start();
#endif
}

void AbstractMiroProcess::interrupt()
{
    terminate();
}

void AbstractMiroProcess::terminate()
{
    if (mProcess.state() == QProcess::Running || mProcess.state() == QProcess::Starting)
        mProcess.kill();
    if (mMiro.state() == QProcess::Running || mMiro.state() == QProcess::Starting)
        mMiro.kill();
}

QProcess::ProcessState AbstractMiroProcess::state() const
{
    if (mProcess.state() == QProcess::Running || mMiro.state() == QProcess::Running)
        return QProcess::Running;
    if (mProcess.state() == QProcess::Starting || mMiro.state() == QProcess::Starting)
        return QProcess::Starting;
    return QProcess::NotRunning;
}

void AbstractMiroProcess::setMiroPath(const QString &miroPath) {
    mMiroPath = miroPath;
}

QString AbstractMiroProcess::modelName() const
{
    return mModelName;
}

void AbstractMiroProcess::setModelName(const QString &modelFile)
{
    mModelName = modelFile;
}

QString AbstractMiroProcess::modelPath() const {
    return workingDirectory() + "/" + modelName() + ".gms";
}

void AbstractMiroProcess::readStdOut()
{
    if (mProcess.state() == QProcess::Running || mProcess.state() == QProcess::Starting)
        readStdChannel(mProcess, QProcess::StandardOutput);
    else if (mMiro.state() == QProcess::Running || mMiro.state() == QProcess::Starting)
        readStdChannel(mMiro, QProcess::StandardOutput);
}

void AbstractMiroProcess::readStdErr()
{
    if (mProcess.state() == QProcess::Running || mProcess.state() == QProcess::Starting)
        readStdChannel(mProcess, QProcess::StandardError);
    else if (mMiro.state() == QProcess::Running || mMiro.state() == QProcess::Starting)
        readStdChannel(mMiro, QProcess::StandardError);
}

void AbstractMiroProcess::completed(int exitCode)
{
    if (exitCode)
        SysLogLocator::systemLog()->append(QString("Could not run MIRO. Exit Code: %1")
                                           .arg(exitCode), LogMsgType::Error);
    emit finished(mGroupId, exitCode);
}

void AbstractMiroProcess::subProcessCompleted(int exitCode)
{
    if (exitCode) {
        SysLogLocator::systemLog()->append(QString("Could not run GAMS. Exit Code: %1")
                                           .arg(exitCode), LogMsgType::Error);
        return;
    }
    emit executeMiro();
}

void AbstractMiroProcess::executeNext()
{
    mMiro.setProgram(mMiroPath);
    mMiro.setWorkingDirectory(workingDirectory());
    mMiro.setProcessEnvironment(miroProcessEnvironment());
    mMiro.start();
}

QString AbstractMiroProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(AbstractProcess::nativeAppPath());
    return QDir::toNativeSeparators(appPath);
}

QString AbstractMiroProcess::confFolder() const
{
    return ConfFolderPrefix + modelName();
}

QString AbstractMiroProcess::dataFolder() const
{
    return DataFolderPrefix + modelName();
}

void AbstractMiroProcess::readStdChannel(QProcess &process, QProcess::ProcessChannel channel)
{
    mOutputMutex.lock();
    process.setReadChannel(channel);
    bool avail = process.bytesAvailable();
    mOutputMutex.unlock();

    while (avail) {
        mOutputMutex.lock();
        process.setReadChannel(channel);
        emit newStdChannelData(process.readLine().constData());
        avail = process.bytesAvailable();
        mOutputMutex.unlock();
    }
}

}
}
