#include "searchresultlist.h"

namespace gams {
namespace studio {

SearchResultList::SearchResultList(SearchResultList &searchResultList) :
    QAbstractTableModel(searchResultList.parent()), mResultList(searchResultList.resultList())
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
    if (parent.isValid())
        return 0;
    return mResultList.size();
}

int SearchResultList::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 3;
}

QVariant SearchResultList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        int row = index.row();

        Result item = mResultList.at(row);

        switch(index.column())
        {
        case 0: return item.locFile(); break;
        case 1: return item.locLineNr(); break;
        case 2: return item.context(); break;
        }
    }
    return QVariant();
}

QVariant SearchResultList::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Filename");
            case 1:
                return QString("LineNr");
            case 2:
                return QString("Context");
            }
        }
    }
    return QVariant();
}

bool SearchResultList::isRegex() const
{
    return mIsRegex;
}

}
}




