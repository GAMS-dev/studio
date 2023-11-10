#include "fileworker.h"

namespace gams {
namespace studio {
namespace search {

FileWorker::FileWorker(Search::SearchParameters parameters) : mParameters(parameters)
{ }

void FileWorker::collectFiles()
{
    QSet<QString> files;
    QDir dir(mParameters.path);


    QDirIterator::IteratorFlag options = mParameters.includeSubdirs
                                             ? QDirIterator::Subdirectories
                                             : QDirIterator::NoIteratorFlags;
    QDirIterator it(dir.path(), QDir::Files, options);
    while (it.hasNext()) {
        if (thread()->isInterruptionRequested()) break;

        QString path = it.next();
        if (path.isEmpty()) break;

        files.insert(path);
    }
    qDebug()/*rogo:delete*/<<QTime::currentTime()<< "found " << files.count() << " files";

    emit filesCollected(files);
}

}
}
}
