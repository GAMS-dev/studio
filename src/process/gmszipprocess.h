#ifndef GAMS_STUDIO_GMSZIPPROCESS_H
#define GAMS_STUDIO_GMSZIPPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {

class GmszipProcess : public AbstractGamsProcess
{
    Q_OBJECT
public:
    explicit GmszipProcess(QObject *parent = nullptr);
    void execute() override;

signals:
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GMSZIPPROCESS_H
