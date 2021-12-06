#include "testfilehandler.h"
#include "file/filemetarepo.h"

namespace gams {
namespace studio {
namespace search {

TestFileHandler::TestFileHandler() {
    QFileInfo file("trnsport.gms");

    qDebug() << QTime::currentTime() << file.filePath(); // rogo: delete

//    mFileRepo = new FileMetaRepo(nullptr);
//    mGms = mFileRepo->findOrCreateFileMeta()
//    mLst;

//    mGmsList;
//    mLstList;
//    mMixedList;

//    mFileNode;
}

FileMeta* TestFileHandler::fileMeta(QWidget* widget)
{
    return mGms;
}

FileMeta* TestFileHandler::fileMeta(FileId fileId)
{
    return mGms;
}

QList<FileMeta*> TestFileHandler::fileMetas()
{
    return mMixedList;
}

QList<FileMeta*> TestFileHandler::openFiles()
{
    return mMixedList;
}

PExFileNode* TestFileHandler::fileNode(QWidget* widget)
{
    return mFileNode;
}

PExFileNode* TestFileHandler::findFile(QString filepath)
{
    return mFileNode;
}

}
}
}
