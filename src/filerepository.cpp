#include "filerepository.h"

namespace gams {
namespace ide {

FileRepository::FileRepository(QObject *parent) : QObject(parent)
{
    connect(&mFsWatcher, &QFileSystemWatcher::fileChanged, this, &FileRepository::fsFileChanged);
    connect(&mFsWatcher, &QFileSystemWatcher::directoryChanged, this, &FileRepository::fsDirChanged);
}

FileContext* FileRepository::addContext(QString filepath)
{
    FileContext *fc = new FileContext(mId++, filepath);
    connect(fc, &FileContext::nameChangedById, this, &FileRepository::onNameChanged);
    mFileData.insert(mId-1, fc);
    emit contextCreated(mId-1);
    return context(mId-1);
}

FileContext* FileRepository::context(int id)
{
    return mFileData.value(id, nullptr);
}

void FileRepository::fsFileChanged(const QString& path)
{
    QFileInfo fc(path);
    // TODO(JM) search affected FileContext to emit signal
//    FileData::iterator it = mFileData.begin();
//    while (it != mFileData.end()) {

//    }
}

void FileRepository::fsDirChanged(const QString& path)
{
    QFileInfo fc(path);
    // TODO(JM) check if we need this
}

void FileRepository::onContextRead(int id)
{
    emit contextRead(id);
}

void FileRepository::onFileInfoChanged(QString newFilePath)
{
    // TODO(JM) search for file existance
    //    emit contextUpdated(id, UpdateScope::storage);
}

void FileRepository::onNameChanged(int id, QString newName)
{
    qDebug() << "Name changed: " << newName;

    // TODO(JM) implement the short Name (with asterix when edited)
}

void FileRepository::onContextUpdated(int id)
{
    emit contextUpdated(id, UpdateScope::all);
}

void FileRepository::onContextDeleted(int id)
{
    mFileData.remove(id);
    emit contextDeleted(id);
}

} // namespace ide
} // namespace gams
