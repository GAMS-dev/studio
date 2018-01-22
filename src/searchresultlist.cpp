#include "searchresultlist.h"

namespace gams {
namespace studio {

SearchResultList::SearchResultList(QObject *parent) : QAbstractTableModel(parent)
{

}

SearchResultList::SearchResultList(const SearchResultList &searchResultList)/* : QAbstractTableModel(searchResultList.parent())*/
{

}

SearchResultList::SearchResultList(const QString &searchTerm, QObject *parent) :
     QAbstractTableModel(parent), mSearchTerm(searchTerm)
{
    useRegex(false); // set default
}

SearchResultList::~SearchResultList()
{
    // todo
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

int SearchResultList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mResultList.size();
}

int SearchResultList::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant SearchResultList::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        int row = index.row();
        int col = index.column();

        Result item = mResultList.at(row);

        return QString("Filename%1, LineNr%2, Context%3")
                .arg(item.locFile())
                .arg(item.locLineNr())
                .arg(item.context());
    } else {
        return QVariant();
    }
}

bool SearchResultList::isRegex() const
{
    return mIsRegex;
}

}
}




