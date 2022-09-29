#ifndef GAMS_STUDIO_CONNECTPROCESS_H
#define GAMS_STUDIO_CONNECTPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {

class ConnectProcess final : public AbstractGamsProcess
{
    Q_OBJECT

public:
    ConnectProcess(QObject *parent = nullptr);
    void execute() override;
    void stop(int waitMSec=0);
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECTPROCESS_H
