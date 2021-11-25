#include "searchfilehandler.h"

namespace gams {
namespace studio {
namespace search {

SearchFileHandler::SearchFileHandler(MainWindow *main) : mMain(main)
{ }

FileMeta* SearchFileHandler::fileMeta(QWidget *widget)
{
     return mMain->fileRepo()->fileMeta(widget);
}

QList<FileMeta*> SearchFileHandler::fileMetas()
{
    return mMain->fileRepo()->fileMetas();
}

QList<FileMeta*> SearchFileHandler::openFiles()
{
    return QList<FileMeta*>::fromVector(mMain->fileRepo()->openFiles());
}

PExFileNode* SearchFileHandler::fileNode(QWidget *widget)
{
    return mMain->projectRepo()->findFileNode(widget);
}

}
}
}
