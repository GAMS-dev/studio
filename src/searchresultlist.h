#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include <QtWidgets>

namespace gams {
namespace studio {

class Result
{
    friend class SearchResultList;
public:
    int locLineNr() const;
    QString locFile() const;
    QString context() const;

private:
    int mLocLineNr;
    QString mLocFile;
    QString mContext;
    explicit Result(int locLineNr, QString locFile, QString context = "");
};

class SearchResultList
{
public:
    SearchResultList(const QString &searchTerm);
    QList<Result> resultList();
    void addResult(int locLineNr, QString locFile, QString context = "");
    void addResultList(QList<Result> resList);
    QString searchTerm() const;
    bool isRegex() const;
    void useRegex(bool regex);
    int size();

private:
    QString mSearchTerm;
    bool mIsRegex;
    QList<Result> mResultList;
};

}
}
#endif // SEARCHRESULTLIST_H
