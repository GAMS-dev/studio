#include "searchresultlist.h"

namespace gams {
namespace studio {

SearchResultList::SearchResultList(const QString &searchTerm) :
    mSearchTerm(searchTerm)
{
    useRegex(false); // set default
}

QList<Result> SearchResultList::resultList()
{
    return mResultList;
}

void SearchResultList::addResult(int locLineNr, QString locFile, QString context)
{
    mResultList.append(Result(locLineNr, locFile, context));
}

void SearchResultList::addResultList(QList<Result> resList)
{
    mResultList.append(resList);
}

QString SearchResultList::searchTerm() const
{
    return mSearchTerm;
}

void SearchResultList::useRegex(bool regex)
{
    mIsRegex = regex;
}

int SearchResultList::size()
{
    return mResultList.size();
}

bool SearchResultList::isRegex() const
{
    return mIsRegex;
}

}
}




