#ifndef FILEREPOSITORY_H
#define FILEREPOSITORY_H

#include <QtCore>
#include "filecontext.h"

namespace gams {
namespace ide {

class FileRepository : public QObject
{
    Q_OBJECT
public:
    explicit FileRepository(QObject *parent = nullptr);
    FileContext* addContext(QString filepath = QString());
    FileContext* context(int id);

signals:
    void contextCreated(int id);
    void contextRead(int id);
    void contextUpdated(int id, UpdateScope updateScope);
    void contextDeleted(int id);

public slots:

private slots:
    void fsFileChanged(const QString &path);
    void fsDirChanged(const QString &path);

    void onContextRead(int id);
    void onFileInfoChanged(QString newFilePath);
    void onNameChanged(int id, QString newName);
    void onContextUpdated(int id);
    void onContextDeleted(int id);

private:
    typedef QHash<int, FileContext*> FileData;

    QFileSystemWatcher mFsWatcher;
    FileData mFileData;
    int mId = 0;
};

} // namespace ide
} // namespace gams

#endif // FILEREPOSITORY_H
