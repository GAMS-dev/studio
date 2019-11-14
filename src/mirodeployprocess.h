#ifndef MIRODEPLOYPROCESS_H
#define MIRODEPLOYPROCESS_H

#include "abstractmiroprocess.h"

namespace gams {
namespace studio {

enum class MiroTargetEnvironment
{
    SingleUser      = 0,
    LocalMultiUser  = 1,
    MultiUser       = 2
};

class MiroDeployProcess : public AbstractMiroProcess
{
    Q_OBJECT

public:
    MiroDeployProcess(QObject *parent = nullptr);

    void setBaseMode(bool baseMode);
    void setHypercubeMode(bool hcubeMode);
    void setTestDeployment(bool testDeploy);
    void setTargetEnvironment(MiroTargetEnvironment targetEnvironment);

protected:
    QProcessEnvironment miroProcessEnvironment() override;
    QStringList miroGamsParameters() override;

private:
    QString deployMode();

private:
    bool mBaseMode;
    bool mHypercubeMode;
    bool mTestDeployment;
    MiroTargetEnvironment mTargetEnvironment;

    QProcess mMiro;
};

}
}

#endif // MIRODEPLOYPROCESS_H
