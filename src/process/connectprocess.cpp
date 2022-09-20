#include "connectprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

namespace gams {
namespace studio {

ConnectProcess::ConnectProcess(QObject *parent)
    : AbstractGamsProcess(appName(), parent)
{
    connect(this, &AbstractProcess::newProcessCall,
            [](const QString &text, const QString &call){ SysLogLocator::systemLog()->append(text + " " + call, LogMsgType::Info); });
    connect(this, &AbstractProcess::newStdChannelData,
            [](const QString &text){ SysLogLocator::systemLog()->append(text, LogMsgType::Info); });

}

void ConnectProcess::execute()
{
    mProcess.setWorkingDirectory(workingDirectory());
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start(nativeAppPath(), parameters());
}

QString ConnectProcess::appName()
{
#ifdef _WIN32
    return "gamsconnect.cmd";
#else
    return "gamsconnect.sh";
#endif
}

} // namespace studio
} // namespace gams
