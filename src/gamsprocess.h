#ifndef GAMSPROCESS_H
#define GAMSPROCESS_H

#include <QDir>
#include <QProcess>

namespace gams {
namespace studio {

class GAMSProcess
{
public:
    GAMSProcess();

    QString app();
    QString nativeAppPath();
    static QString nativeAppPath(const QString &dir, const QString &app);

    void setSystemDir(const QString &systemDir);
    QString systemDir() const;

    void setWorkingDir(const QString &workingDir);
    QString workingDir() const;

    void run();
    static QString aboutGAMS();

private:
    static const QString App;
    QString mSystemDir;
    QString mWorkingDir;
    QProcess mProcess;
};

} // namespace studio
} // namespace gams

#endif // GAMSPROCESS_H
