#ifndef RESULT_H
#define RESULT_H

#include <QString>

namespace gams {
namespace studio {

class Result
{
    friend class SearchResultList;
public:
    int lineNr() const;
    int colNr() const;
    QString filepath() const;
    QString context() const;

private:
    int mLineNr;
    int mColNr;
    QString mFilepath;
    QString mContext;
    explicit Result(int lineNr, int colNr, QString filepath, QString context = "");
};

}
}

#endif // RESULT_H
