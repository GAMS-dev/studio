#include "mirodeployprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

namespace gams {
namespace studio {

MiroDeployProcess::MiroDeployProcess(QObject *parent)
    : AbstractMiroProcess("gams", parent)
{
}

void MiroDeployProcess::setBaseMode(bool baseMode)
{
    mBaseMode = baseMode;
}

void MiroDeployProcess::setHypercubeMode(bool hcubeMode)
{
    mHypercubeMode = hcubeMode;
}

void MiroDeployProcess::setTestDeployment(bool testDeploy)
{
    mTestDeployment = testDeploy;
}

void MiroDeployProcess::setTargetEnvironment(MiroTargetEnvironment targetEnvironment)
{
    mTargetEnvironment = targetEnvironment;
}

QProcessEnvironment MiroDeployProcess::miroProcessEnvironment()
{
    auto environment = QProcessEnvironment::systemEnvironment();
    auto miroBuild = mTestDeployment ? "false" : "true";
    auto miroTestDeploy = mTestDeployment ? "true" : "false";


    switch (mTargetEnvironment) {
    case MiroTargetEnvironment::SingleUser:
        environment.insert("MIRO_MODE", deployMode());
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "false");
        environment.insert("MIRO_BUILD_ARCHIVE", "false");
        environment.insert("MIRO_BUILD",miroBuild);
        environment.insert("MIRO_TEST_DEPLOY", miroTestDeploy);
        break;
    case MiroTargetEnvironment::LocalMultiUser:
        environment.insert("MIRO_MODE", deployMode());
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "true");
        environment.insert("MIRO_BUILD_ARCHIVE", "false");
        environment.insert("MIRO_BUILD", miroBuild);
        environment.insert("MIRO_TEST_DEPLOY", miroTestDeploy);
        break;
    case MiroTargetEnvironment::MultiUser:
        environment.insert("MIRO_MODE", deployMode());
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "true");
        environment.insert("MIRO_BUILD_ARCHIVE", "true");
        environment.insert("MIRO_BUILD", miroBuild);
        environment.insert("MIRO_TEST_DEPLOY", miroTestDeploy);
        break;
    }
    return environment;
}

QStringList MiroDeployProcess::miroGamsParameters()
{
    return { QString("IDCJSON=%1/%2_io.json").arg(confFolder()).arg(modelName()),
             "a=c" };
}

QString MiroDeployProcess::deployMode()
{
    if (mBaseMode && mHypercubeMode)
        return "full";
    if (mBaseMode)
        return "base";
    if (mHypercubeMode)
        return "hcube";
    return QString();
}

}
}
