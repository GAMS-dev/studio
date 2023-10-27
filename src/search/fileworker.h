#ifndef FILEWORKER_H
#define FILEWORKER_H

#include <QObject>
#include "search.h"

namespace gams {
namespace studio {
namespace search {

class FileWorker : public QObject
{
    Q_OBJECT

public:
    FileWorker(Search::SearchParameters parameters);
    void collectFiles();

signals:
    void filesCollected(QSet<QString> files);

private:
    Search::SearchParameters mParameters;

};

}
}
}

#endif // FILEWORKER_H
