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
    int length() const;

private:
    int mLineNr;
    int mColNr;
    int mLength;
    QString mFilepath;
    QString mContext;
    explicit Result(int lineNr, int colNr, int length, QString fileLoc, QString context = "");
};

}
}

#endif // RESULT_H
