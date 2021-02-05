#ifndef FILEEVENTHANDLER_H
#define FILEEVENTHANDLER_H

#include <QObject>
#include <QSharedPointer>
#include <QMap>
#include <QVector>

#include "file/fileevent.h"

class QCheckBox;

namespace gams {
namespace studio {

struct FileEventData;
class FileMeta;
class MainWindow;
class FileChangeDialog;

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
    bool filter(const QVector<FileEventData> &events);
    void process();
    void processChange(int result);
    void processDeletion(int result);

    void closeAllDeletedFiles();
    void closeFirstDeletedFile();
    void keepAllDeletedFiles();
    void keepFirstDeletedFile();

    void reloadAllChangedFiles(bool always = false);
    void reloadFirstChangedFile(bool always = false);
    void keepAllChangedFiles();
    void keepFirstChangedFile();

    void reloadFile(FileMeta *file);
    void removeFile(FileMeta *file);

private:
    MainWindow *mMainWindow;
    QCheckBox *mCheckBox;
    FileChangeDialog *mDialog;
    QVector<FileEventData> mCurrentEvents;
    QMap<Type, QMap<int, FileEventData>> mQueuedEvents;
    Type mCurrentType = None;
    bool mOpenMessageBox = false;
    FileMeta *mCurrentFile;
};

}
}

#endif // FILEEVENTHANDLER_H
