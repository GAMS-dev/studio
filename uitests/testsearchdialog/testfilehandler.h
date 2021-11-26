#ifndef TESTFILEHANDLER_H
#define TESTFILEHANDLER_H

#include "abstractsearchfilehandler.h"

namespace gams {
namespace studio {
namespace search {

class TestFileHandler : public AbstractSearchFileHandler
{
public:
    TestFileHandler();

    FileMeta* fileMeta(QWidget* widget);
    FileMeta* fileMeta(FileId fileId);
    QList<FileMeta*> fileMetas();
    QList<FileMeta*> openFiles();
    PExFileNode* fileNode(QWidget* widget);
    PExFileNode* findFile(QString filepath);

private:
    FileMetaRepo* mFileRepo;

    FileMeta* mGms;
    FileMeta* mLst;

    QList<FileMeta*> mGmsList;
    QList<FileMeta*> mLstList;
    QList<FileMeta*> mMixedList;

    PExFileNode* mFileNode;
};

}
}
}
#endif // TESTFILEHANDLER_H
