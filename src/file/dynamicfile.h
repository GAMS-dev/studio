#ifndef DYNAMICFILE_H
#define DYNAMICFILE_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QMutex>

namespace gams {
namespace studio {

class DynamicFile : public QObject
{
    Q_OBJECT
public:
    DynamicFile(QString fileName, int backups = 0, QObject *parent = nullptr);
    virtual ~DynamicFile();
    void appendLine(QString line);

private slots:
    void closeFile();

private:
    void openFile();
    void handleExisting(int backups);

private:
    QMutex mMutex;
    QFile mFile;
    QTimer mCloseTimer;
};

} // namespace studio
} // namespace gams

#endif // DYNAMICFILE_H
