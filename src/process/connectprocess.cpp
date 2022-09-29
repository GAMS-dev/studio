#include "connectprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

#include <QDir>
#include <commonpaths.h>

namespace gams {
namespace studio {

ConnectProcess::ConnectProcess(QObject *parent)
    : AbstractGamsProcess("GMSPython/python", parent)
{
    connect(this, &AbstractProcess::newProcessCall,
            [](const QString &text, const QString &call){ SysLogLocator::systemLog()->append(text + " " + call, LogMsgType::Info); });
    connect(this, &AbstractProcess::newStdChannelData,
            [](const QString &text){ SysLogLocator::systemLog()->append(text, LogMsgType::Info); });

}

void ConnectProcess::execute()
{
    mProcess.setWorkingDirectory(workingDirectory());
    QStringList param;
    param << QDir::toNativeSeparators(CommonPaths::systemDir() + "/connectdriver.py");
    param << QDir::toNativeSeparators(CommonPaths::systemDir());
    param << parameters();
    setParameters(param);
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start(nativeAppPath(), parameters());
}

void ConnectProcess::stop(int waitMSec)
{
    mProcess.kill();
    if (waitMSec>0)
        mProcess.waitForFinished(waitMSec);
}

} // namespace studio
} // namespace gams
