#ifndef GAMSMIROPROCESS_H
#define GAMSMIROPROCESS_H

#include "abstractmiroprocess.h"

namespace gams {
namespace studio {

enum class MiroMode
{
    Base,
    Hypercube,
    Configuration
};

class MiroProcess : public AbstractMiroProcess
{
    Q_OBJECT

public:
    MiroProcess(QObject *parent = nullptr);

    void execute() override;

    void setMiroMode(MiroMode mode);

    void setSkipModelExecution(bool skipModelExeution) {
        mSkipModelExecution = skipModelExeution;
    }

protected:
    QProcessEnvironment miroProcessEnvironment() override;
    QStringList miroGamsParameters() override;

private:
    void setupMiroEnvironment();

private:
    MiroMode mMiroMode;
    bool mSkipModelExecution;
};

}
}

#endif // GAMSMIROPROCESS_H
