#ifndef GAMSPROCESS_H
#define GAMSPROCESS_H

#include <QObject>
#include <QProcess>
#include <QMutex>

namespace gams {
namespace studio {

class FileGroupContext;

class GAMSProcess
        : public QObject
{
    Q_OBJECT

public:
    GAMSProcess(QObject *parent = Q_NULLPTR);

    QString app();
    QString nativeAppPath();
    static QString nativeAppPath(const QString &dir, const QString &app);

    void setSystemDir(const QString &systemDir);
    QString systemDir() const;

    void setWorkingDir(const QString &workingDir);
    QString workingDir() const;

    void setInputFile(const QString &file);
    QString inputFile() const;

    void setContext(FileGroupContext *context);
    FileGroupContext* context() const;

    void execute();
    static QString aboutGAMS();

signals:
    void finished(int exitCode);
    void newStdChannelData(QProcess::ProcessChannel channel, const QString &data);

private slots:
    void completed(int exitCode);
    void readStdOut();
    void readStdErr();
    void readStdChannel(QProcess::ProcessChannel channel);

private:
    static const QString App;
    QString mSystemDir;
    QString mWorkingDir;
    QString mInputFile;
    QProcess mProcess;
    QMutex mOutputMutex;
    FileGroupContext *mContext = nullptr;
};

} // namespace studio
} // namespace gams

#endif // GAMSPROCESS_H
