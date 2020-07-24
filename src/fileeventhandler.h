#ifndef FILEEVENTHANDLER_H
#define FILEEVENTHANDLER_H

#include <QObject>
#include <QSharedPointer>
#include <QVector>

#include "file/fileevent.h"

class QMessageBox;

namespace gams {
namespace studio {

struct FileEventData;
class MainWindow;

class FileEventHandler : public QObject
{
    Q_OBJECT

public:
    enum Type { Change, Deletion, None };

    FileEventHandler(MainWindow *mainWindow, QObject *parent = nullptr);

    void process(Type type, const QVector<FileEventData> &events);

private slots:
    void messageBoxFinished(int result);

private:
    void process();

    void closeAllDeletedFiles();
    void closeFirstDeletedFile();
    void keepAllDeletedFiles();
    void keepFirstDeletedFile();

    void reloadAllChangedFiles();
    void reloadFirstChangedFile();
    void keepAllChangedFiles();
    void keepFirstChangedFile();

    void openMessageBox(QString filePath, bool deleted, bool modified, int count);

private:
    MainWindow *mMainWindow;
    QSharedPointer<QMessageBox> mMessageBox;
    QVector<FileEventData> mEvents;
    Type mType = None;
    bool mOpenMessageBox = false;
};

}
}

#endif // FILEEVENTHANDLER_H
