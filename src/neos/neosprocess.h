#ifndef GAMS_STUDIO_NEOS_NEOSPROCESS_H
#define GAMS_STUDIO_NEOS_NEOSPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {
namespace neos {

class NeosProcess : public AbstractProcess
{
    Q_OBJECT
public:
    explicit NeosProcess(QObject *parent = nullptr);

    virtual void execute() override;
    virtual QProcess::ProcessState state() const override;

signals:

protected slots:
    virtual void readStdOut() override;
    virtual void readStdErr() override;

};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSPROCESS_H
