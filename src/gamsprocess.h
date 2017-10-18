#ifndef GAMSPROCESS_H
#define GAMSPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {

class FileGroupContext;

class GAMSProcess
        : public AbstractProcess
{
    Q_OBJECT

public:
    GAMSProcess(QObject *parent = Q_NULLPTR);

    virtual QString app();
    virtual QString nativeAppPath();

    void setWorkingDir(const QString &workingDir);
    QString workingDir() const;

    void setContext(FileGroupContext *context);
    FileGroupContext* context() const;

    virtual void execute();

    static QString aboutGAMS();

private:
    static const QString App;
    QString mWorkingDir;
    FileGroupContext *mContext = nullptr;
};

} // namespace studio
} // namespace gams

#endif // GAMSPROCESS_H
