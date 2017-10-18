#ifndef GAMSLIBPROCESS_H
#define GAMSLIBPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {

class GAMSLibProcess
        : public AbstractProcess
{
    Q_OBJECT

public:
    GAMSLibProcess(QObject *parent = Q_NULLPTR);

    void setApp(const QString &app);
    virtual QString app();

    virtual QString nativeAppPath();

    void setTargetDir(const QString &targetDir);
    QString targetDir() const;

    void setModelNumber(int modelNumber);
    int modelNumber() const;

    void setModelName(const QString &modelName);
    QString modelName() const;

    virtual void execute();

private:
    QString mApp;
    QString mTargetDir;
    int mModelNumber = -1;
    QString mModelName;
};

} // namespace studio
} // namespace gams

#endif // GAMSLIBPROCESS_H
