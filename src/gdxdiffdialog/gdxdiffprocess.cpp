#include "gdxdiffprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

#include <QDir>

namespace gams {
namespace studio {
namespace gdxdiffdialog {

GdxDiffProcess::GdxDiffProcess(QObject *parent)
    : AbstractGamsProcess("gdxdiff", parent)
{
    connect(this, &AbstractProcess::newProcessCall,
            [](const QString &text, const QString &call){ SysLogLocator::systemLog()->append(text + " " + call, LogMsgType::Info); });
    connect(this, &AbstractProcess::newStdChannelData, this, &GdxDiffProcess::appendSystemLog);
}

void GdxDiffProcess::execute()
{
    mProcess.setWorkingDirectory(workingDirectory());
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start(nativeAppPath(), parameters());
}

QString GdxDiffProcess::diffFile() const
{
    return mDiffFile;
}

void GdxDiffProcess::stop(int waitMSec)
{
    mProcess.kill();
    if (waitMSec>0)
        mProcess.waitForFinished(waitMSec);
}

void GdxDiffProcess::appendSystemLog(const QString &text)
{
    SysLogLocator::systemLog()->append(text, LogMsgType::Info);
    if (text.contains("Output:")) {
        mDiffFile = text.split("Output:").last().trimmed();
        if (QFileInfo(mDiffFile).isRelative())
            mDiffFile = QDir::cleanPath(workingDirectory() + QDir::separator() + mDiffFile);
    }
}

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams
