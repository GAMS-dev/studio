#ifndef GAMS_STUDIO_GAMSINSTPROCESS_H
#define GAMS_STUDIO_GAMSINSTPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {
namespace process {

class GamsInstProcess : public AbstractGamsProcess
{
    Q_OBJECT

public:
    GamsInstProcess(QObject *parent = nullptr);
    void execute() override;

    QStringList configPaths() const {
        return mConfig;
    }

    QStringList dataPaths() const {
        return mData;
    }

    QStringList defaultParameters() const override {
        return {"-listdirs"};
    }

private slots:
    void newData(const QByteArray &data);

private:
    bool isData = false;
    QStringList mData;
    QStringList mConfig;
};

} // namespace process
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GAMSINSTPROCESS_H
