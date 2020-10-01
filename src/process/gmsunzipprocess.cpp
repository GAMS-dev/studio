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
    mProcess.setArguments(parameters());
    mProcess.setProgram(nativeAppPath());
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start();
}

} // namespace studio
} // namespace gams
