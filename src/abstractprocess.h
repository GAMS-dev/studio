#ifndef ABSTRACTPROCESS_H
#define ABSTRACTPROCESS_H

#include <QObject>
#include <QProcess>
#include <QMutex>

namespace gams {
namespace studio {

class AbstractProcess
        : public QObject
{
    Q_OBJECT

public:
    AbstractProcess(QObject *parent = Q_NULLPTR);

    virtual QString app() = 0;

    virtual QString nativeAppPath() = 0;
    static QString nativeAppPath(const QString &dir, const QString &app);

    void setSystemDir(const QString &systemDir);
    QString systemDir() const;

    void setInputFile(const QString &file);
    QString inputFile() const;

    virtual void execute() = 0;

signals:
    void finished(int exitCode);
    void newStdChannelData(QProcess::ProcessChannel channel, const QString &data);

protected slots:
    void completed(int exitCode);
    void readStdOut();
    void readStdErr();
    void readStdChannel(QProcess::ProcessChannel channel);

protected:
    QString mSystemDir;
    QString mInputFile;
    QProcess mProcess;
    QMutex mOutputMutex;
};

} // namespace studio
} // namespace gams

#endif // ABSTRACTPROCESS_H
