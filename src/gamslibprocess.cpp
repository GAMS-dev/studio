#include "gamslibprocess.h"
#include "gamsinfo.h"

#include <QDebug>
#include <QDir>

namespace gams {
namespace studio {

GAMSLibProcess::GAMSLibProcess(QObject *parent)
    : AbstractProcess(parent)
{

}

void GAMSLibProcess::setApp(const QString &app)
{
    mApp = app;
}

QString GAMSLibProcess::app()
{
    return mApp;
}

QString GAMSLibProcess::nativeAppPath()
{
    return AbstractProcess::nativeAppPath(mSystemDir, mApp);
}

void GAMSLibProcess::setTargetDir(const QString &targetDir)
{
    mTargetDir = targetDir;
}

QString GAMSLibProcess::targetDir() const
{
    return mTargetDir;
}

void GAMSLibProcess::setModelNumber(int modelNumber)
{
    mModelNumber = modelNumber;
}

int GAMSLibProcess::modelNumber() const
{
    return mModelNumber;
}

void GAMSLibProcess::setModelName(const QString &modelName)
{
    mModelName = modelName;
}

QString GAMSLibProcess::modelName() const
{
    return mModelName;
}

void GAMSLibProcess::execute()
{// TODO(AF) improve the gamslib output messages... currently they don't make sense for the GAMS Studio!
    QStringList args;
    args << (mModelName.isEmpty() ? QString::number(mModelNumber) : mModelName);
    args << QDir::toNativeSeparators(mTargetDir);
    mProcess.start(nativeAppPath(), args);
}

} // namespace studio
} // namespace gams
