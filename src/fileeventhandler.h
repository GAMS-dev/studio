/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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
