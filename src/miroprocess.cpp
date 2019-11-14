#include "miroprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

#include <QDir>

namespace gams {
namespace studio {

MiroProcess::MiroProcess(QObject *parent)
    : AbstractMiroProcess("gams", parent)
{
}

void MiroProcess::setMiroMode(MiroMode mode)
{
    mMiroMode = mode;
}

void MiroProcess::execute()
{
    setupMiroEnvironment();
    if (mSkipModelExecution)
        emit executeMiro();
    else
        AbstractMiroProcess::execute();
}

QProcessEnvironment MiroProcess::miroProcessEnvironment()
{
    auto environment = QProcessEnvironment::systemEnvironment();
    switch (mMiroMode) {
    case MiroMode::Base:
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "false");
        environment.insert("MIRO_MODE", "base");
        break;
    case MiroMode::Hypercube:
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "true");
        environment.insert("MIRO_MODE", "hcube");
        break;
    case MiroMode::Configuration:
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "false");
        environment.insert("MIRO_MODE", "config");
        break;
    }
    return environment;
}

QStringList MiroProcess::miroGamsParameters()
{
    return { QString("IDCGenerateJSON=%1/%2_io.json").arg(confFolder()).arg(modelName()),
             QString("IDCGenerateGDX=%1/default.gdx").arg(dataFolder()) };
}

void MiroProcess::setupMiroEnvironment()
{
    QDir confDir(workingDirectory()+"/"+confFolder());
    if (!confDir.exists() && !confDir.mkpath(confDir.path())) {
        SysLogLocator::systemLog()->append(QString("Could not create the configuration folder: %1")
                                           .arg(confDir.path()), LogMsgType::Error);
    }

    QDir dataDir(workingDirectory()+"/"+dataFolder());
    if (!dataDir.exists() && !dataDir.mkpath(dataDir.path())) {
        SysLogLocator::systemLog()->append(QString("Could not create the data folder: %1")
                                           .arg(dataDir.path()), LogMsgType::Error);
    }
}

}
}
