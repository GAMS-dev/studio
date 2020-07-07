#ifndef GAMS_STUDIO_NEOS_NEOSPROCESS_H
#define GAMS_STUDIO_NEOS_NEOSPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {
namespace neos {

class NeosProcess final : public AbstractGamsProcess
{
    Q_OBJECT

public:
    NeosProcess(QObject *parent = nullptr);
    void setGmsFile(QString gmsFile);

    void execute() override;
    void interrupt() override;

protected:
    void readStdChannel(QProcess::ProcessChannel channel) override;

private slots:
    void readSubStdOut();
    void readSubStdErr();
    void subFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool prepareNeosParameters();
    bool prepareKill(QStringList &tempParams);
    void scanForCredentials(const QByteArray &data);
    QString rawData(QString runFile, QString parameters, QString workdir);
    QString rawKill();

    QString mRunFile;
    QString mJobNumber;
    QString mJobPassword;
    QProcess *mSubProc;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSPROCESS_H
