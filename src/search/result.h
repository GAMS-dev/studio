#ifndef RESULT_H
#define RESULT_H

#include <QString>

namespace gams {
namespace studio {

class Result
{
    friend class SearchResultList;
public:
    int locLineNr() const;
    int locCol() const;
    QString locFile() const;
    QString node() const;

private:
    int mLocLineNr;
    int mLocCol;
    QString mLocFile;
    QString mNode;
    explicit Result(int locLineNr, int locCol, QString locFile, QString node = "");
};

}
}

#endif // RESULT_H
