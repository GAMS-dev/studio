#ifndef TESTFILEHANDLER_H
#define TESTFILEHANDLER_H

#include "abstractsearchfilehandler.h"

namespace gams {
namespace studio {
namespace search {

class TestFileHandler : public AbstractSearchFileHandler
{
public:
    FileMeta* fileMeta(QWidget* widget);
    FileMeta* fileMeta(FileId fileId);
    QList<FileMeta*> fileMetas();
    QList<FileMeta*> openFiles();
    PExFileNode* fileNode(QWidget* widget);
    PExFileNode* findFile(QString filepath);
};

}
}
}
#endif // TESTFILEHANDLER_H
