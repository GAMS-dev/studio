#ifndef GAMS_STUDIO_GMSUNZIPPROCESS_H
#define GAMS_STUDIO_GMSUNZIPPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {

class GmsunzipProcess : public AbstractGamsProcess
{
    Q_OBJECT
public:
    explicit GmsunzipProcess(QObject *parent = nullptr);
    void execute() override;

signals:
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GMSUNZIPPROCESS_H
