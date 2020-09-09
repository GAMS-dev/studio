#include "gmsunzipprocess.h"

namespace gams {
namespace studio {

GmsunzipProcess::GmsunzipProcess(QObject *parent) : AbstractGamsProcess("gmsunzip", parent)
{
    connect(&mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GmsunzipProcess::finished);
}

void GmsunzipProcess::execute()
{
    mProcess.setWorkingDirectory(workingDirectory());
#if defined(__unix__) || defined(__APPLE__)
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start(nativeAppPath(), parameters());
#else
    mProcess.setNativeArguments(parameters().join(" "));
    mProcess.setProgram(nativeAppPath());
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start();
#endif
}

} // namespace studio
} // namespace gams
