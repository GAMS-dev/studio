#include "gmszipprocess.h"

namespace gams {
namespace studio {

GmszipProcess::GmszipProcess(QObject *parent) : AbstractGamsProcess("gmszip", parent)
{
    connect(&mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GmszipProcess::finished);
}

void GmszipProcess::execute()
{
    mProcess.setWorkingDirectory(workingDirectory());
    mProcess.setArguments(parameters());
    mProcess.setProgram(nativeAppPath());
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start();
}

} // namespace studio
} // namespace gams
