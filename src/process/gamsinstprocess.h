#ifndef GAMS_STUDIO_GAMSINSTPROCESS_H
#define GAMS_STUDIO_GAMSINSTPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {

class GamsInstProcess : public AbstractGamsProcess
{
    Q_OBJECT
public:
    GamsInstProcess(QObject *parent = nullptr);
    void execute() override;
    QStringList configPaths();
    QStringList dataPaths();
private slots:
    void newData(const QByteArray &data);
private:
    bool isData = false;
    QStringList mData;
    QStringList mConfig;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GAMSINSTPROCESS_H
