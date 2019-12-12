#ifndef ABSTRACTMIROPROCESS_H
#define ABSTRACTMIROPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {

class AbstractMiroProcess : public AbstractProcess
{
    Q_OBJECT

public:
    AbstractMiroProcess(const QString &application, QObject *parent = nullptr);

    void execute() override;
    void interrupt() override;
    void terminate() override;

    QProcess::ProcessState state() const override;

    void setMiroPath(const QString &miroPath);

    QString modelName() const;
    void setModelName(const QString &modelFile);

    QString modelPath() const;

signals:
    void executeMiro();

protected slots:
    void readStdOut() override;
    void readStdErr() override;
    void completed(int exitCode) override;
    virtual void subProcessCompleted(int exitCode);
    void executeNext();

protected:
    QString nativeAppPath() override;

    virtual QProcessEnvironment miroProcessEnvironment() = 0;
    virtual QStringList miroGamsParameters() = 0;

    QString confFolder() const;
    QString dataFolder() const;

protected:
    void readStdChannel(QProcess &process, QProcess::ProcessChannel channel);

private:
    QString mMiroPath;
    QString mModelName;
    QProcess mMiro;

    static const QString ConfFolderPrefix;
    static const QString DataFolderPrefix;
};

}
}

#endif // ABSTRACTMIROPROCESS_H
